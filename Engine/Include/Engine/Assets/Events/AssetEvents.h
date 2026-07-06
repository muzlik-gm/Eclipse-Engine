// ============================================================================
// File: Engine/Include/Engine/Assets/Events/AssetEvents.h
// Runtime events for the asset system.
// ============================================================================
#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Core/UUID.h"
#include "Engine/Assets/Core/AssetTypes.h"

#include <string>
#include <string_view>

namespace engine::asset_events {

    using engine::core::UUID;
    using engine::events::Event;
    using engine::events::EventType;
    using engine::events::EventCategory;
    using engine::assets::AssetUUID;
    using engine::assets::AssetType;

    // ========================================================================
    // Asset lifecycle events
    // ========================================================================

    class AssetCreatedEvent final : public Event
    {
    public:
        explicit AssetCreatedEvent(AssetUUID uuid, AssetType type)
            : m_UUID(uuid), m_Type(type) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetCreated"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::EntityCreated; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] AssetType GetType() const noexcept { return m_Type; }

    private:
        AssetUUID m_UUID;
        AssetType m_Type;
    };

    class AssetDeletedEvent final : public Event
    {
    public:
        explicit AssetDeletedEvent(AssetUUID uuid) : m_UUID(uuid) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetDeleted"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::EntityDestroyed; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }

    private:
        AssetUUID m_UUID;
    };

    class AssetLoadedEvent final : public Event
    {
    public:
        explicit AssetLoadedEvent(AssetUUID uuid) : m_UUID(uuid) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetLoaded"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::ComponentAdded; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }

    private:
        AssetUUID m_UUID;
    };

    class AssetUnloadedEvent final : public Event
    {
    public:
        explicit AssetUnloadedEvent(AssetUUID uuid) : m_UUID(uuid) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetUnloaded"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::ComponentRemoved; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }

    private:
        AssetUUID m_UUID;
    };

    class AssetReloadedEvent final : public Event
    {
    public:
        explicit AssetReloadedEvent(AssetUUID uuid) : m_UUID(uuid) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetReloaded"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::ComponentModified; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }

    private:
        AssetUUID m_UUID;
    };

    class AssetMovedEvent final : public Event
    {
    public:
        AssetMovedEvent(AssetUUID uuid, std::string oldPath, std::string newPath)
            : m_UUID(uuid), m_OldPath(std::move(oldPath)), m_NewPath(std::move(newPath)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetMoved"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::HierarchyChanged; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] const std::string& GetOldPath() const noexcept { return m_OldPath; }
        [[nodiscard]] const std::string& GetNewPath() const noexcept { return m_NewPath; }

    private:
        AssetUUID   m_UUID;
        std::string m_OldPath;
        std::string m_NewPath;
    };

    class AssetRenamedEvent final : public Event
    {
    public:
        AssetRenamedEvent(AssetUUID uuid, std::string oldName, std::string newName)
            : m_UUID(uuid), m_OldName(std::move(oldName)), m_NewName(std::move(newName)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "AssetRenamed"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::TransformChanged; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] const std::string& GetOldName() const noexcept { return m_OldName; }
        [[nodiscard]] const std::string& GetNewName() const noexcept { return m_NewName; }

    private:
        AssetUUID   m_UUID;
        std::string m_OldName;
        std::string m_NewName;
    };

    // ========================================================================
    // Import events
    // ========================================================================

    class ImportStartedEvent final : public Event
    {
    public:
        explicit ImportStartedEvent(AssetUUID uuid, std::string filePath)
            : m_UUID(uuid), m_FilePath(std::move(filePath)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ImportStarted"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::SceneLoaded; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] const std::string& GetFilePath() const noexcept { return m_FilePath; }

    private:
        AssetUUID   m_UUID;
        std::string m_FilePath;
    };

    class ImportFinishedEvent final : public Event
    {
    public:
        explicit ImportFinishedEvent(AssetUUID uuid, bool success)
            : m_UUID(uuid), m_Success(success) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ImportFinished"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::SceneActivated; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] bool IsSuccess() const noexcept { return m_Success; }

    private:
        AssetUUID m_UUID;
        bool      m_Success;
    };

    class ImportFailedEvent final : public Event
    {
    public:
        ImportFailedEvent(AssetUUID uuid, std::string reason)
            : m_UUID(uuid), m_Reason(std::move(reason)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "ImportFailed"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::SceneUnloaded; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] const std::string& GetReason() const noexcept { return m_Reason; }

    private:
        AssetUUID   m_UUID;
        std::string m_Reason;
    };

    // ========================================================================
    // Serialization / metadata events
    // ========================================================================

    class SerializationCompletedEvent final : public Event
    {
    public:
        explicit SerializationCompletedEvent(AssetUUID uuid, std::string filePath)
            : m_UUID(uuid), m_FilePath(std::move(filePath)) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "SerializationCompleted"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::VisibilityChanged; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }
        [[nodiscard]] const std::string& GetFilePath() const noexcept { return m_FilePath; }

    private:
        AssetUUID   m_UUID;
        std::string m_FilePath;
    };

    class MetadataChangedEvent final : public Event
    {
    public:
        explicit MetadataChangedEvent(AssetUUID uuid) : m_UUID(uuid) {}

        [[nodiscard]] std::string_view GetName() const noexcept override { return "MetadataChanged"; }
        [[nodiscard]] EventType GetEventType() const noexcept override { return EventType::ComponentModified; }
        [[nodiscard]] EventCategory GetCategoryFlags() const noexcept override { return EventCategory::Application; }

        [[nodiscard]] AssetUUID GetUUID() const noexcept { return m_UUID; }

    private:
        AssetUUID m_UUID;
    };

} // namespace engine::asset_events
