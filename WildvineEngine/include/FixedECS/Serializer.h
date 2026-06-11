#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <fstream>
#include <stdexcept>
#include "Registry.h"

// ============================================================
//  ECS :: Serializer.h
//  Serialización / deserialización de una escena completa.
//
//  Dependencia: nlohmann/json (header-only)
//    https://github.com/nlohmann/json
//    Incluye <nlohmann/json.hpp> antes de este header,
//    o añade la ruta en tu CMakeLists / proyecto.
//
//  ── Cómo registrar un componente ─────────────────────────
//
//  1. Añade Serialize() / Deserialize() a tu struct:
//
//     struct Transform {
//         float x = 0, y = 0, z = 0;
//
//         nlohmann::json Serialize() const {
//             return { {"x", x}, {"y", y}, {"z", z} };
//         }
//         static Transform Deserialize(const nlohmann::json& j) {
//             return { j.at("x"), j.at("y"), j.at("z") };
//         }
//     };
//
//  2. Registra el componente en el Serializer:
//
//     serializer.Register<Transform>("Transform");
//
//  3. Serializa / deserializa:
//
//     json scene = serializer.Serialize(registry);
//     serializer.SaveToFile(registry, "scene.json");
//     serializer.LoadFromFile(registry, "scene.json");
//
// ============================================================

#include <nlohmann/json.hpp>

namespace ECS {

    using json = nlohmann::json;

    class Serializer
    {
    public:
        // ── Registro de componentes ───────────────────────────

        // T debe implementar:
        //   json        Serialize()            const;
        //   static T    Deserialize(const json&);
        template<typename T>
        void Register(const std::string& name)
        {
            const ComponentTypeID typeID = GetComponentTypeID<T>();

            ComponentHandler handler;
            handler.name = name;

            // Serializar: void* → json
            handler.serialize = [](void* raw) -> json {
                return static_cast<T*>(raw)->Serialize();
            };

            // Deserializar: json → añadir/reemplazar componente en entidad
            handler.deserialize = [](Registry& registry, EntityID entity, const json& j) {
                T component = T::Deserialize(j);
                if (registry.HasComponent<T>(entity))
                    registry.GetComponent<T>(entity) = std::move(component);
                else
                    registry.AddComponent<T>(entity, std::move(component));
            };

            m_byTypeID[typeID]  = handler;
            m_byName[name]      = typeID;
        }

        // ── Serializar escena ─────────────────────────────────

        [[nodiscard]] json Serialize(Registry& registry) const
        {
            json root;
            json entitiesArr = json::array();

            for (EntityID entity : registry.GetAllEntities())
            {
                if (entity == NULL_ENTITY || !registry.IsAlive(entity))
                    continue;

                json entityJson;
                entityJson["id"]      = entity;
                entityJson["index"]   = GetEntityIndex(entity);
                entityJson["version"] = GetEntityVersion(entity);

                json componentsObj = json::object();
                const auto& pools = registry.GetPools();

                for (auto& [typeID, handler] : m_byTypeID)
                {
                    auto it = pools.find(typeID);
                    if (it == pools.end()) continue;

                    void* raw = it->second->GetRaw(entity);
                    if (raw)
                        componentsObj[handler.name] = handler.serialize(raw);
                }

                entityJson["components"] = componentsObj;
                entitiesArr.push_back(entityJson);
            }

            root["ecs_version"] = 1;
            root["entities"]    = entitiesArr;
            return root;
        }

        // ── Deserializar escena ───────────────────────────────
        // NOTA: recrea las entidades en el mismo orden y asigna
        // IDs consecutivos. Si los sistemas guardan EntityIDs
        // cruzados, usa el campo "id" del JSON para remapearlos.

        void Deserialize(Registry& registry, const json& root)
        {
            registry.Clear();

            for (const auto& entityJson : root.at("entities"))
            {
                EntityID newEntity = registry.CreateEntity();

                for (const auto& [name, data] : entityJson.at("components").items())
                {
                    auto it = m_byName.find(name);
                    if (it == m_byName.end()) continue;

                    auto handlerIt = m_byTypeID.find(it->second);
                    if (handlerIt == m_byTypeID.end()) continue;

                    handlerIt->second.deserialize(registry, newEntity, data);
                }
            }
        }

        // ── I/O de archivo ────────────────────────────────────

        void SaveToFile(Registry& registry, const std::string& path, int indent = 2) const
        {
            std::ofstream file(path);
            if (!file.is_open())
                throw std::runtime_error("Serializer: no se pudo abrir '" + path + "' para escribir");
            file << Serialize(registry).dump(indent);
        }

        void LoadFromFile(Registry& registry, const std::string& path)
        {
            std::ifstream file(path);
            if (!file.is_open())
                throw std::runtime_error("Serializer: no se pudo abrir '" + path + "' para leer");
            json root;
            file >> root;
            Deserialize(registry, root);
        }

    private:
        struct ComponentHandler {
            std::string                                                    name;
            std::function<json(void*)>                                     serialize;
            std::function<void(Registry&, EntityID, const json&)>         deserialize;
        };

        std::unordered_map<ComponentTypeID, ComponentHandler> m_byTypeID;
        std::unordered_map<std::string, ComponentTypeID>      m_byName;
    };

} // namespace ECS
