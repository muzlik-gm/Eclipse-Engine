// ============================================================================
// File: Engine/Include/Engine/ECS/IComponentReflection.h
// Abstract interface for component reflection — runtime introspection of
// component types, their fields, and serialization hooks.
// ============================================================================
#pragma once

#include "Engine/Core/Types.h"
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Registry.h"

#include <any>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::ecs {

    // ========================================================================
    // FieldInfo — describes a single field of a component.
    // ========================================================================

    /// @brief Describes a single field of a reflected component.
    struct FieldInfo
    {
        std::string Name;
        std::string TypeName;
        core::usize Offset{0};
        core::usize Size{0};

        /// True if the field is an array / container.
        bool IsArray{false};

        /// Number of elements if IsArray is true (0 = dynamic).
        core::usize ArrayCount{0};

        /// Optional human-readable description for the editor.
        std::string Description;

        /// Optional category for grouping in the editor.
        std::string Category{"Default"};

        /// True if the field should be hidden from the editor.
        bool Hidden{false};

        /// True if the field is read-only.
        bool ReadOnly{false};
    };

    // ========================================================================
    // IComponentReflection — reflection interface for a component type.
    // ========================================================================

    /// @brief Provides runtime reflection for a single component type.
    ///
    /// Each reflected component type registers an implementation of this
    /// interface with ComponentReflectionRegistry.  The editor and
    /// serialization system use these interfaces to inspect and manipulate
    /// component data without knowing the concrete C++ type.
    class IComponentReflection
    {
    public:
        virtual ~IComponentReflection() = default;

        /// @brief Returns the component type name (e.g. "TransformComponent").
        [[nodiscard]] virtual const std::string& GetTypeName() const noexcept = 0;

        /// @brief Returns the component version (for schema migration).
        [[nodiscard]] virtual core::u32 GetVersion() const noexcept = 0;

        /// @brief Returns the size of the component in bytes.
        [[nodiscard]] virtual core::usize GetSize() const noexcept = 0;

        /// @brief Returns all reflected fields of the component.
        [[nodiscard]] virtual const std::vector<FieldInfo>& GetFields() const noexcept = 0;

        /// @brief Returns the field info for @p fieldName, or nullptr.
        [[nodiscard]] virtual const FieldInfo* FindField(
            const std::string& fieldName) const = 0;

        /// @brief Returns the value of @p fieldName on @p entity as std::any.
        [[nodiscard]] virtual std::any GetFieldValue(
            Registry& registry, Entity entity,
            const std::string& fieldName) const = 0;

        /// @brief Sets the value of @p fieldName on @p entity from std::any.
        virtual void SetFieldValue(
            Registry& registry, Entity entity,
            const std::string& fieldName,
            const std::any& value) const = 0;

        /// @brief Returns true if @p entity has this component.
        [[nodiscard]] virtual bool HasComponent(
            Registry& registry, Entity entity) const = 0;

        /// @brief Removes this component from @p entity.
        virtual void RemoveComponent(
            Registry& registry, Entity entity) const = 0;

        /// @brief Serializes this component on @p entity to a string.
        ///        The format is implementation-defined (JSON, YAML, etc.).
        [[nodiscard]] virtual std::string Serialize(
            Registry& registry, Entity entity) const = 0;

        /// @brief Deserializes @p data into this component on @p entity.
        virtual void Deserialize(
            Registry& registry, Entity entity,
            const std::string& data) const = 0;
    };

    // ========================================================================
    // ComponentReflectionRegistry — registry of reflected component types.
    // ========================================================================

    /// @brief Registry of all reflected component types.  Components
    ///        register their reflection implementations at startup; the
    ///        editor and serialization system query this registry to
    ///        enumerate and manipulate component types at runtime.
    class ComponentReflectionRegistry
    {
    public:
        /// @brief Returns the global registry instance.
        [[nodiscard]] static ComponentReflectionRegistry& Get();

        /// @brief Registers a reflection implementation for a component type.
        void Register(std::unique_ptr<IComponentReflection> reflection);

        /// @brief Unregisters a component type by name.
        void Unregister(const std::string& typeName);

        /// @brief Returns the reflection for @p typeName, or nullptr.
        [[nodiscard]] const IComponentReflection* Find(
            const std::string& typeName) const;

        /// @brief Returns all registered component type names.
        [[nodiscard]] std::vector<std::string> GetTypeNames() const;

        /// @brief Returns the number of registered component types.
        [[nodiscard]] core::usize GetCount() const noexcept;

        /// @brief Clears all registrations.
        void Clear();

    private:
        ComponentReflectionRegistry() = default;
        std::unordered_map<std::string, std::unique_ptr<IComponentReflection>> m_Reflections;
    };

    // ========================================================================
    // ComponentRegistrar — RAII helper for component reflection registration.
    // ========================================================================

    /// @brief RAII helper that registers a component reflection on
    ///        construction and unregisters it on destruction.
    class ComponentRegistrar
    {
    public:
        explicit ComponentRegistrar(std::unique_ptr<IComponentReflection> reflection)
            : m_TypeName(reflection ? reflection->GetTypeName() : "")
        {
            if (reflection)
                ComponentReflectionRegistry::Get().Register(std::move(reflection));
        }

        ~ComponentRegistrar()
        {
            if (!m_TypeName.empty())
                ComponentReflectionRegistry::Get().Unregister(m_TypeName);
        }

        ComponentRegistrar(const ComponentRegistrar&)            = delete;
        ComponentRegistrar& operator=(const ComponentRegistrar&) = delete;
        ComponentRegistrar(ComponentRegistrar&&)                 = delete;
        ComponentRegistrar& operator=(ComponentRegistrar&&)      = delete;

    private:
        std::string m_TypeName;
    };

} // namespace engine::ecs
