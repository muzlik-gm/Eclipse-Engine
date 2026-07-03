#pragma once

/**
 * @file Config.h
 * @brief Hierarchical configuration system supporting JSON and YAML formats.
 *
 * Provides a key-value store with typed accessors, in-memory defaults,
 * validation hooks, change-notification callbacks, and dot-notation
 * section queries.  Values are stored as a variant of string, i64, f64,
 * or bool, and can be loaded from / saved to JSON (.json) and YAML
 * (.yaml / .yml) files.
 */

#include "Engine/Core/Types.h"

#include <nlohmann/json_fwd.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace engine::config {

/// Polymorphic value type stored by Config.
using Value = std::variant<std::string, core::i64, core::f64, bool>;

/// Validator function — returns true when a value is acceptable.
using Validator = std::function<bool(const Value&)>;

/// Callback invoked when a configuration value changes.
using ChangeCallback = std::function<void(std::string_view key, const Value& value)>;

/**
 * @brief Hierarchical key-value configuration store.
 *
 * Supports loading and saving in JSON and YAML formats, typed accessors
 * with optional defaults, per-key validators, change-notification
 * callbacks, and dot-notation section extraction.
 */
class Config
{
public:
    /// Creates an empty configuration.
    Config();

    /**
     * @brief Loads a configuration from a file.
     * @param filepath Path to a .json, .yaml, or .yml file.
     *
     * If the file cannot be opened or parsed, the Config remains empty
     * and a warning is logged.
     */
    explicit Config(std::string_view filepath);

    // ========================================================================
    // Loading / Saving
    // ========================================================================

    /**
     * @brief Loads configuration from a file.
     * @param filepath Path to a .json, .yaml, or .yml file.
     * @return True on success.
     */
    bool Load(std::string_view filepath);

    /**
     * @brief Saves the current configuration to a file.
     * @param filepath Destination path (extension selects format).
     * @return True on success.
     */
    bool Save(std::string_view filepath) const;

    /**
     * @brief Parses configuration from an in-memory string.
     * @param content  The configuration text.
     * @param format   "json" or "yaml" (case-insensitive).
     */
    void LoadFromString(std::string_view content, std::string_view format = "json");

    /**
     * @brief Serializes the configuration to a string.
     * @param format "json" or "yaml" (case-insensitive).
     * @return The serialized configuration text.
     */
    [[nodiscard]] std::string SaveToString(std::string_view format = "json") const;

    // ========================================================================
    // Typed Getters
    // ========================================================================

    /// Returns the raw value for @p key, or std::nullopt if absent.
    [[nodiscard]] std::optional<Value> Get(std::string_view key) const;

    /// Returns the string value, or @p defaultValue if the key is absent or not a string.
    [[nodiscard]] std::string GetString(std::string_view key, std::string_view defaultValue = "") const;

    /// Returns the integer value, or @p defaultValue if the key is absent or not an integer.
    [[nodiscard]] core::i64 GetInt(std::string_view key, core::i64 defaultValue = 0) const;

    /// Returns the float value, or @p defaultValue if the key is absent or not a float.
    [[nodiscard]] core::f64 GetFloat(std::string_view key, core::f64 defaultValue = 0.0) const;

    /// Returns the boolean value, or @p defaultValue if the key is absent or not a bool.
    [[nodiscard]] bool GetBool(std::string_view key, bool defaultValue = false) const;

    // ========================================================================
    // Setters
    // ========================================================================

    /// Stores a generic value under @p key and fires change callbacks.
    void Set(std::string_view key, const Value& value);

    /// Convenience: stores a string value.
    void SetString(std::string_view key, std::string_view value);

    /// Convenience: stores an integer value.
    void SetInt(std::string_view key, core::i64 value);

    /// Convenience: stores a float value.
    void SetFloat(std::string_view key, core::f64 value);

    /// Convenience: stores a boolean value.
    void SetBool(std::string_view key, bool value);

    // ========================================================================
    // Queries
    // ========================================================================

    /// Returns true if @p key exists in the store.
    [[nodiscard]] bool Has(std::string_view key) const;

    /// Removes @p key from the store.  Returns true if the key existed.
    bool Remove(std::string_view key);

    /// Removes all keys, validators, and callbacks.
    void Clear();

    /// Returns the number of stored key-value pairs.
    [[nodiscard]] core::usize Size() const;

    /// Returns all stored keys in an unspecified order.
    [[nodiscard]] std::vector<std::string> Keys() const;

    /// True when the value at @p key is a std::string (or key is absent — returns false).
    [[nodiscard]] bool IsString(std::string_view key) const;

    /// True when the value at @p key is a core::i64.
    [[nodiscard]] bool IsInt(std::string_view key) const;

    /// True when the value at @p key is a core::f64.
    [[nodiscard]] bool IsFloat(std::string_view key) const;

    /// True when the value at @p key is a bool.
    [[nodiscard]] bool IsBool(std::string_view key) const;

    // ========================================================================
    // Merging
    // ========================================================================

    /**
     * @brief Merges another configuration into this one.
     *
     * Existing values are overwritten by @p other's values.
     */
    void Merge(const Config& other);

    /**
     * @brief Merges defaults — only sets values that do not already exist.
     */
    void MergeDefaults(const Config& defaults);

    // ========================================================================
    // Validation
    // ========================================================================

    /**
     * @brief Registers a validator for a specific key.
     *
     * Replaces any previously registered validator for the same key.
     */
    void AddValidator(std::string_view key, Validator validator);

    /**
     * @brief Runs all registered validators.
     * @param[out] outError Human-readable description of the first failure.
     * @return True when every validator passes.
     */
    bool Validate(std::string& outError) const;

    // ========================================================================
    // Callbacks
    // ========================================================================

    /**
     * @brief Registers a callback invoked whenever @p key's value changes.
     *
     * Multiple callbacks per key are supported and called in registration
     * order.
     */
    void OnChange(std::string_view key, ChangeCallback callback);

    /**
     * @brief Manually triggers change callbacks for @p key.
     */
    void NotifyChange(std::string_view key);

    // ========================================================================
    // Nested Sections (dot-notation)
    // ========================================================================

    /**
     * @brief Extracts a sub-configuration for keys that start with @p prefix.
     *
     * The returned Config contains only matching keys with the prefix
     * stripped.  For example, given keys "render.width" and "render.height",
     * GetSection("render.") returns a Config with keys "width" and "height".
     */
    [[nodiscard]] Config GetSection(std::string_view prefix) const;

private:
    // -- Format helpers ------------------------------------------------------

    /// Parses a JSON string into m_values.  Returns true on success.
    bool LoadJSON(std::string_view content);

    /// Parses a YAML string into m_values.  Returns true on success.
    bool LoadYAML(std::string_view content);

    /// Parses an INI string into m_values.  Returns true on success.
    /// INI sections are prepended as dot-notation prefixes.
    bool LoadINI(std::string_view content);

    /// Serializes m_values to a JSON string (4-space indent).
    [[nodiscard]] std::string SaveJSON() const;

    /// Serializes m_values to a YAML string.
    [[nodiscard]] std::string SaveYAML() const;

    /// Serializes m_values to an INI string with section grouping.
    [[nodiscard]] std::string SaveINI() const;

    /// Converts a nlohmann::json node to a Config Value.
    [[nodiscard]] Value JSONValueToEngineValue(const nlohmann::json& j) const;

    /// Converts a Config Value to a nlohmann::json node.
    [[nodiscard]] nlohmann::json EngineValueToJSON(const Value& v) const;

    // -- Data ----------------------------------------------------------------

    std::unordered_map<std::string, Value> m_values;
    std::unordered_map<std::string, Validator> m_validators;
    std::unordered_map<std::string, std::vector<ChangeCallback>> m_callbacks;
};

} // namespace engine::config