/**
 * @file Config.cpp
 * @brief Implementation of the hierarchical configuration system.
 */

#include "Engine/Configuration/Config.h"
#include "Engine/Core/Log.h"

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

namespace engine::config {

using engine::core::usize;
using engine::core::i64;
using engine::core::f64;

// ============================================================================
// Internal helpers
// ============================================================================

namespace {

/// Detects the file format from the extension (case-insensitive).
std::string DetectFormat(std::string_view filepath)
{
    std::filesystem::path path(filepath);
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (ext == ".yaml" || ext == ".yml")
    {
        return "yaml";
    }
    if (ext == ".ini")
    {
        return "ini";
    }
    return "json"; // default fallback
}

/// Recursively flattens a YAML node into dot-notation key-value pairs.
void FlattenYAML(const YAML::Node& node, std::string_view prefix,
                 std::unordered_map<std::string, Value>& out)
{
    if (!node.IsMap())
    {
        return;
    }

    for (const auto& entry : node)
    {
        std::string key = std::string(prefix) + entry.first.as<std::string>();

        if (entry.second.IsMap())
        {
            // Recurse into nested maps, appending a dot separator.
            FlattenYAML(entry.second, key + ".", out);
        }
        else if (entry.second.IsSequence())
        {
            // Sequences are stored as their YAML string representation.
            YAML::Emitter emitter;
            emitter << entry.second;
            out.emplace(std::move(key), emitter.c_str());
        }
        else if (entry.second.IsScalar())
        {
            const YAML::Node& val = entry.second;

            // Try to determine the scalar type.
            // YAML::Node::as<T> will throw on mismatch, so probe in order
            // of most-specific first.  We use Tag() heuristics as well.
            if (val.Tag() == "!!bool" || val.Tag() == "tag:yaml.org,2002:bool")
            {
                out.emplace(std::move(key), val.as<bool>());
            }
            else if (val.Tag() == "!!int" || val.Tag() == "tag:yaml.org,2002:int")
            {
                out.emplace(std::move(key), static_cast<core::i64>(val.as<core::i64>()));
            }
            else if (val.Tag() == "!!float" || val.Tag() == "tag:yaml.org,2002:float")
            {
                out.emplace(std::move(key), val.as<core::f64>());
            }
            else
            {
                // Default: try bool, then integer, then float, then string.
                // This handles cases where the YAML tag is unclear
                // (e.g. implicit resolution for unquoted values).

                // Bool probe
                try
                {
                    bool b = val.as<bool>();
                    // Distinguish from integers/strings: check the raw representation.
                    // "true"/"false" (case-insensitive) are unambiguous bools.
                    std::string raw = val.Scalar();
                    std::string lower = raw;
                    std::transform(lower.begin(), lower.end(), lower.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (lower == "true" || lower == "false")
                    {
                        out.emplace(std::move(key), b);
                        continue;
                    }
                }
                catch (const YAML::Exception&)
                {
                    // Not a bool — fall through.
                }

                // Integer probe
                try
                {
                    core::i64 i = val.as<core::i64>();
                    out.emplace(std::move(key), i);
                    continue;
                }
                catch (const YAML::Exception&)
                {
                    // Not an integer — fall through.
                }

                // Float probe
                try
                {
                    core::f64 f = val.as<core::f64>();
                    out.emplace(std::move(key), f);
                    continue;
                }
                catch (const YAML::Exception&)
                {
                    // Not a float — fall through to string.
                }

                out.emplace(std::move(key), val.as<std::string>());
            }
        }
    }
}

} // anonymous namespace

// ============================================================================
// Constructors
// ============================================================================

Config::Config() = default;

Config::Config(std::string_view filepath)
{
    Load(filepath);
}

// ============================================================================
// Loading / Saving
// ============================================================================

bool Config::Load(std::string_view filepath)
{
    std::ifstream stream{std::string(filepath)};
    if (!stream.is_open())
    {
        ENGINE_LOG_ERROR("Config::Load — failed to open file: '{}'", filepath);
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
    stream.close();

    std::string format = DetectFormat(filepath);
    LoadFromString(content, format);

    ENGINE_LOG_INFO("Config::Load — loaded {} entries from '{}' ({})", m_values.size(), filepath, format);
    return true;
}

bool Config::Save(std::string_view filepath) const
{
    std::string format = DetectFormat(filepath);
    std::string content = SaveToString(format);

    std::ofstream stream{std::string(filepath)};
    if (!stream.is_open())
    {
        ENGINE_LOG_ERROR("Config::Save — failed to open file for writing: '{}'", filepath);
        return false;
    }

    stream << content;
    stream.close();

    ENGINE_LOG_INFO("Config::Save — saved {} entries to '{}' ({})", m_values.size(), filepath, format);
    return true;
}

void Config::LoadFromString(std::string_view content, std::string_view format)
{
    std::string fmt(format);
    std::transform(fmt.begin(), fmt.end(), fmt.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (fmt == "yaml" || fmt == "yml")
    {
        LoadYAML(content);
    }
    else if (fmt == "ini")
    {
        LoadINI(content);
    }
    else
    {
        LoadJSON(content);
    }
}

std::string Config::SaveToString(std::string_view format) const
{
    std::string fmt(format);
    std::transform(fmt.begin(), fmt.end(), fmt.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (fmt == "yaml" || fmt == "yml")
    {
        return SaveYAML();
    }
    if (fmt == "ini")
    {
        return SaveINI();
    }
    return SaveJSON();
}

// ============================================================================
// Typed Getters
// ============================================================================

std::optional<Value> Config::Get(std::string_view key) const
{
    auto it = m_values.find(std::string(key));
    if (it != m_values.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::string Config::GetString(std::string_view key, std::string_view defaultValue) const
{
    auto it = m_values.find(std::string(key));
    if (it != m_values.end())
    {
        const auto* str = std::get_if<std::string>(&it->second);
        if (str)
        {
            return *str;
        }
    }
    return std::string(defaultValue);
}

core::i64 Config::GetInt(std::string_view key, core::i64 defaultValue) const
{
    auto it = m_values.find(std::string(key));
    if (it != m_values.end())
    {
        const auto* val = std::get_if<core::i64>(&it->second);
        if (val)
        {
            return *val;
        }
    }
    return defaultValue;
}

core::f64 Config::GetFloat(std::string_view key, core::f64 defaultValue) const
{
    auto it = m_values.find(std::string(key));
    if (it != m_values.end())
    {
        const auto* val = std::get_if<core::f64>(&it->second);
        if (val)
        {
            return *val;
        }
    }
    return defaultValue;
}

bool Config::GetBool(std::string_view key, bool defaultValue) const
{
    auto it = m_values.find(std::string(key));
    if (it != m_values.end())
    {
        const auto* val = std::get_if<bool>(&it->second);
        if (val)
        {
            return *val;
        }
    }
    return defaultValue;
}

// ============================================================================
// Setters
// ============================================================================

void Config::Set(std::string_view key, const Value& value)
{
    std::string k(key);
    auto [it, inserted] = m_values.emplace(k, value);
    if (!inserted)
    {
        it->second = value;
    }
    NotifyChange(k);
}

void Config::SetString(std::string_view key, std::string_view value)
{
    Set(key, std::string(value));
}

void Config::SetInt(std::string_view key, core::i64 value)
{
    Set(key, value);
}

void Config::SetFloat(std::string_view key, core::f64 value)
{
    Set(key, value);
}

void Config::SetBool(std::string_view key, bool value)
{
    Set(key, value);
}

// ============================================================================
// Queries
// ============================================================================

bool Config::Has(std::string_view key) const
{
    return m_values.contains(std::string(key));
}

bool Config::Remove(std::string_view key)
{
    std::string k(key);
    auto it = m_values.find(k);
    if (it != m_values.end())
    {
        m_values.erase(it);
        m_validators.erase(k);
        m_callbacks.erase(k);
        return true;
    }
    return false;
}

void Config::Clear()
{
    m_values.clear();
    m_validators.clear();
    m_callbacks.clear();
}

core::usize Config::Size() const
{
    return m_values.size();
}

std::vector<std::string> Config::Keys() const
{
    std::vector<std::string> keys;
    keys.reserve(m_values.size());
    for (const auto& [k, v] : m_values)
    {
        keys.push_back(k);
    }
    return keys;
}

bool Config::IsString(std::string_view key) const
{
    auto it = m_values.find(std::string(key));
    return it != m_values.end() && std::holds_alternative<std::string>(it->second);
}

bool Config::IsInt(std::string_view key) const
{
    auto it = m_values.find(std::string(key));
    return it != m_values.end() && std::holds_alternative<core::i64>(it->second);
}

bool Config::IsFloat(std::string_view key) const
{
    auto it = m_values.find(std::string(key));
    return it != m_values.end() && std::holds_alternative<core::f64>(it->second);
}

bool Config::IsBool(std::string_view key) const
{
    auto it = m_values.find(std::string(key));
    return it != m_values.end() && std::holds_alternative<bool>(it->second);
}

// ============================================================================
// Merging
// ============================================================================

void Config::Merge(const Config& other)
{
    for (const auto& [k, v] : other.m_values)
    {
        m_values[k] = v;
    }
}

void Config::MergeDefaults(const Config& defaults)
{
    for (const auto& [k, v] : defaults.m_values)
    {
        m_values.try_emplace(k, v);
    }
}

// ============================================================================
// Validation
// ============================================================================

void Config::AddValidator(std::string_view key, Validator validator)
{
    m_validators[std::string(key)] = std::move(validator);
}

bool Config::Validate(std::string& outError) const
{
    for (const auto& [key, validator] : m_validators)
    {
        auto it = m_values.find(key);
        if (it == m_values.end())
        {
            // Key not present — skip validation (validators only fire for existing keys).
            continue;
        }

        if (!validator(it->second))
        {
            outError = "Validation failed for key '" + key + "'";
            return false;
        }
    }
    return true;
}

// ============================================================================
// Callbacks
// ============================================================================

void Config::OnChange(std::string_view key, ChangeCallback callback)
{
    m_callbacks[std::string(key)].push_back(std::move(callback));
}

void Config::NotifyChange(std::string_view key)
{
    auto it = m_callbacks.find(std::string(key));
    if (it == m_callbacks.end())
    {
        return;
    }

    auto valIt = m_values.find(std::string(key));
    const Value* value = (valIt != m_values.end()) ? &valIt->second : nullptr;

    // If the key was removed, we still fire with a null-ish value by using
    // a default-constructed Value (empty string).
    Value fallbackValue = std::string{""};
    if (!value)
    {
        value = &fallbackValue;
    }

    for (const auto& cb : it->second)
    {
        cb(key, *value);
    }
}

// ============================================================================
// Nested Sections (dot-notation)
// ============================================================================

Config Config::GetSection(std::string_view prefix) const
{
    Config section;
    std::string pfx(prefix);

    for (const auto& [k, v] : m_values)
    {
        if (k.rfind(pfx, 0) == 0) // starts_with
        {
            std::string strippedKey = k.substr(pfx.size());
            if (!strippedKey.empty())
            {
                section.m_values.emplace(std::move(strippedKey), v);
            }
        }
    }

    return section;
}

// ============================================================================
// Private — JSON helpers
// ============================================================================

bool Config::LoadJSON(std::string_view content)
{
    try
    {
        nlohmann::json j = nlohmann::json::parse(content);
        if (!j.is_object())
        {
            ENGINE_LOG_WARN("Config::LoadJSON — top-level JSON value is not an object");
            return false;
        }

        for (const auto& [key, value] : j.items())
        {
            // Recursively flatten nested JSON objects into dot-notation keys.
            if (value.is_object())
            {
                // Flatten the nested object.
                std::function<void(const nlohmann::json&, const std::string&)> flatten =
                    [&](const nlohmann::json& obj, const std::string& pfx)
                {
                    for (const auto& [k, v] : obj.items())
                    {
                        std::string fullKey = pfx.empty() ? k : pfx + "." + k;
                        if (v.is_object())
                        {
                            flatten(v, fullKey);
                        }
                        else
                        {
                            m_values.emplace(fullKey, JSONValueToEngineValue(v));
                        }
                    }
                };
                flatten(value, std::string(key));
            }
            else
            {
                m_values.emplace(std::string(key), JSONValueToEngineValue(value));
            }
        }

        return true;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        ENGINE_LOG_ERROR("Config::LoadJSON — parse error: {}", e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        ENGINE_LOG_ERROR("Config::LoadJSON — error: {}", e.what());
        return false;
    }
}

std::string Config::SaveJSON() const
{
    nlohmann::json j = nlohmann::json::object();

    for (const auto& [key, value] : m_values)
    {
        // Reconstruct nested objects from dot-notation keys.
        nlohmann::json* current = &j;
        std::string::size_type start = 0;
        std::string::size_type dotPos = key.find('.');

        while (dotPos != std::string::npos)
        {
            std::string segment = key.substr(start, dotPos - start);
            current = &(current->operator[](std::move(segment)));
            start = dotPos + 1;
            dotPos = key.find('.', start);
        }

        // Final segment — set the value.
        std::string lastSegment = key.substr(start);
        (*current)[std::move(lastSegment)] = EngineValueToJSON(value);
    }

    return j.dump(4);
}

Value Config::JSONValueToEngineValue(const nlohmann::json& j) const
{
    if (j.is_string())
    {
        return j.get<std::string>();
    }
    if (j.is_number_integer())
    {
        return j.get<core::i64>();
    }
    if (j.is_number_float())
    {
        return j.get<core::f64>();
    }
    if (j.is_boolean())
    {
        return j.get<bool>();
    }
    // Fallback: stringify the value.
    return j.dump();
}

nlohmann::json Config::EngineValueToJSON(const Value& v) const
{
    return std::visit(
        [](const auto& arg) -> nlohmann::json { return arg; },
        v);
}

// ============================================================================
// Private — YAML helpers
// ============================================================================

bool Config::LoadYAML(std::string_view content)
{
    try
    {
        YAML::Node root = YAML::Load(std::string(content));
        if (!root.IsMap())
        {
            ENGINE_LOG_WARN("Config::LoadYAML — top-level YAML node is not a map");
            return false;
        }

        FlattenYAML(root, "", m_values);
        return true;
    }
    catch (const YAML::ParserException& e)
    {
        ENGINE_LOG_ERROR("Config::LoadYAML — parse error: {}", e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        ENGINE_LOG_ERROR("Config::LoadYAML — error: {}", e.what());
        return false;
    }
}

std::string Config::SaveYAML() const
{
    // Build a nested YAML structure from dot-notation keys.
    YAML::Emitter emitter;
    emitter << YAML::BeginMap;

    // Group keys by their top-level prefix.
    std::unordered_map<std::string, std::vector<std::pair<std::string, const Value*>>> groups;

    for (const auto& [k, v] : m_values)
    {
        auto dotPos = k.find('.');
        if (dotPos == std::string::npos)
        {
            groups[""].emplace_back(k, &v);
        }
        else
        {
            groups[k.substr(0, dotPos)].emplace_back(k, &v);
        }
    }

    // Helper: recursively emit a nested YAML map from dot-notation keys.
    std::function<void(YAML::Emitter&, const std::vector<std::pair<std::string, const Value*>>&, std::string_view)> emitGroup =
        [&](YAML::Emitter& em, const std::vector<std::pair<std::string, const Value*>>& entries, std::string_view prefix)
    {
        // Group entries by their next dot-segment.
        std::map<std::string, std::vector<std::pair<std::string, const Value*>>> subGroups;
        std::vector<std::pair<std::string, const Value*>> leafEntries;

        for (const auto& [fullKey, valPtr] : entries)
        {
            std::string relative = fullKey.substr(prefix.size());
            auto dotPos = relative.find('.');
            if (dotPos == std::string::npos)
            {
                leafEntries.emplace_back(relative, valPtr);
            }
            else
            {
                std::string segment = relative.substr(0, dotPos);
                subGroups[segment].emplace_back(fullKey, valPtr);
            }
        }

        // Emit leaf entries first.
        for (const auto& [leafKey, valPtr] : leafEntries)
        {
            em << YAML::Key << leafKey;
            std::visit(
                [&em](const auto& arg) { em << YAML::Value << arg; },
                *valPtr);
        }

        // Emit nested groups.
        for (const auto& [segment, subEntries] : subGroups)
        {
            em << YAML::Key << segment;
            em << YAML::Value;
            em << YAML::BeginMap;
            emitGroup(em, subEntries, prefix.empty() ? segment : std::string(prefix) + "." + segment);
            em << YAML::EndMap;
        }
    };

    // Emit the root-level groups (those with empty prefix).
    if (auto it = groups.find(""); it != groups.end())
    {
        for (const auto& [k, v] : it->second)
        {
            emitter << YAML::Key << k;
            std::visit(
                [&emitter](const auto& arg) { emitter << YAML::Value << arg; },
                *v);
        }
    }

    // Emit each top-level group.
    for (auto& [groupName, entries] : groups)
    {
        if (groupName.empty())
        {
            continue; // already handled above
        }

        emitter << YAML::Key << groupName;
        emitter << YAML::Value;
        emitter << YAML::BeginMap;
        emitGroup(emitter, entries, groupName);
        emitter << YAML::EndMap;
    }

    emitter << YAML::EndMap;

    if (!emitter.good())
    {
        ENGINE_LOG_ERROR("Config::SaveYAML — emitter error: {}", emitter.GetLastError());
    }

    return std::string(emitter.c_str());
}

// ============================================================================
// INI format
// ============================================================================

bool Config::LoadINI(std::string_view content)
{
    // Simple INI parser.  Format:
    //   ; comment
    //   [section]
    //   key = value
    //   key = "quoted value"
    //
    // Sections become dot-notation prefixes: section.key = value.

    std::string currentSection;
    usize lineNum = 0;

    std::istringstream stream{std::string(content)};
    std::string line;

    while (std::getline(stream, line))
    {
        ++lineNum;

        // Trim leading/trailing whitespace.
        usize start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            continue; // blank line
        }
        usize end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);

        // Skip comments (lines starting with ; or #).
        if (trimmed[0] == ';' || trimmed[0] == '#')
        {
            continue;
        }

        // Section header: [section_name]
        if (trimmed[0] == '[' && trimmed.back() == ']')
        {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            // Trim whitespace from section name.
            usize s = currentSection.find_first_not_of(" \t");
            usize e = currentSection.find_last_not_of(" \t");
            if (s != std::string::npos && e != std::string::npos)
            {
                currentSection = currentSection.substr(s, e - s + 1);
            }
            continue;
        }

        // Key-value pair: key = value
        usize eqPos = trimmed.find('=');
        if (eqPos == std::string::npos)
        {
            continue; // malformed line, skip
        }

        std::string key = trimmed.substr(0, eqPos);
        std::string value = trimmed.substr(eqPos + 1);

        // Trim key.
        usize ks = key.find_first_not_of(" \t");
        usize ke = key.find_last_not_of(" \t");
        if (ks != std::string::npos && ke != std::string::npos)
        {
            key = key.substr(ks, ke - ks + 1);
        }
        else
        {
            continue;
        }

        // Trim value.
        usize vs = value.find_first_not_of(" \t");
        usize ve = value.find_last_not_of(" \t");
        if (vs != std::string::npos && ve != std::string::npos)
        {
            value = value.substr(vs, ve - vs + 1);
        }
        else
        {
            value.clear();
        }

        // Remove surrounding quotes from value.
        if (value.size() >= 2 &&
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\'')))
        {
            value = value.substr(1, value.size() - 2);
        }

        // Build the full key with section prefix.
        std::string fullKey = currentSection.empty()
            ? key
            : currentSection + "." + key;

        // Try to parse as integer, then float, then bool, otherwise string.
        // Use the same logic as JSONValueToEngineValue would.
        Value configValue;

        // Try bool.
        std::string lowerVal = value;
        std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lowerVal == "true" || lowerVal == "1")
        {
            configValue = true;
        }
        else if (lowerVal == "false" || lowerVal == "0")
        {
            configValue = false;
        }
        else
        {
            // Try integer.
            bool isInt = !value.empty() &&
                         (value[0] == '-' || value[0] == '+' ||
                          std::isdigit(static_cast<unsigned char>(value[0])));
            if (isInt)
            {
                try
                {
                    i64 intVal = 0;
                    usize pos = 0;
                    intVal = std::stoll(value, &pos);
                    if (pos == value.size())
                    {
                        configValue = intVal;
                    }
                    else
                    {
                        isInt = false;
                    }
                }
                catch (...)
                {
                    isInt = false;
                }
            }

            if (!isInt)
            {
                // Try float.
                bool isFloat = !value.empty() &&
                               (value[0] == '-' || value[0] == '+' ||
                                std::isdigit(static_cast<unsigned char>(value[0])) ||
                                value[0] == '.');
                if (isFloat)
                {
                    try
                    {
                        f64 floatVal = 0.0;
                        usize pos = 0;
                        floatVal = std::stod(value, &pos);
                        if (pos == value.size())
                        {
                            configValue = floatVal;
                        }
                        else
                        {
                            configValue = value;
                        }
                    }
                    catch (...)
                    {
                        configValue = value;
                    }
                }
                else
                {
                    configValue = value;
                }
            }
        }

        m_values[fullKey] = configValue;
    }

    return true;
}

std::string Config::SaveINI() const
{
    // Group keys by their section prefix.
    // Keys without a dot go under a [General] section.
    // Keys like "render.width" go under [render].

    std::map<std::string, std::vector<std::pair<std::string, Value>>> sections;

    for (const auto& [key, value] : m_values)
    {
        usize dotPos = key.find('.');
        if (dotPos != std::string::npos)
        {
            std::string section = key.substr(0, dotPos);
            std::string localKey = key.substr(dotPos + 1);
            sections[section].emplace_back(localKey, value);
        }
        else
        {
            sections["General"].emplace_back(key, value);
        }
    }

    std::ostringstream oss;

    for (const auto& [section, entries] : sections)
    {
        oss << "[" << section << "]\n";
        for (const auto& [key, value] : entries)
        {
            std::visit([&](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>)
                {
                    oss << key << " = " << (v ? "true" : "false") << "\n";
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    // Quote strings that contain spaces or special characters.
                    if (v.find(' ') != std::string::npos || v.find(';') != std::string::npos ||
                        v.find('#') != std::string::npos)
                    {
                        oss << key << " = \"" << v << "\"\n";
                    }
                    else
                    {
                        oss << key << " = " << v << "\n";
                    }
                }
                else
                {
                    oss << key << " = " << v << "\n";
                }
            }, value);
        }
        oss << "\n";
    }

    return oss.str();
}

} // namespace engine::config