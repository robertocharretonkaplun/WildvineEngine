#pragma once

#include <cstdint>
#include <limits>

// ============================================================
//  ECS :: Types.h
//  Tipos fundamentales del Entity Component System.
//
//  EntityID  = uint64_t que empaqueta:
//              bits [0..31]  → EntityIndex  (posición en el array)
//              bits [32..63] → EntityVersion (generación; invalida IDs viejos)
//
//  Al destruir una entidad su version sube 1, así cualquier
//  EntityID antiguo guardado en otro sitio queda inválido.
// ============================================================

namespace ECS {

    // ── Tipos primitivos ──────────────────────────────────────
    using EntityIndex    = uint32_t;
    using EntityVersion  = uint32_t;
    using EntityID       = uint64_t;
    using ComponentTypeID = uint32_t;

    // Valor centinela para "ninguna entidad"
    // Parentesis alrededor de max para blindar contra la macro max() de Windows.h
    inline constexpr EntityID NULL_ENTITY = (std::numeric_limits<EntityID>::max)();

    // ── Empaquetado / desempaquetado de EntityID ─────────────
    [[nodiscard]] inline EntityIndex GetEntityIndex(EntityID id) noexcept
    {
        return static_cast<EntityIndex>(id & 0xFFFF'FFFFull);
    }

    [[nodiscard]] inline EntityVersion GetEntityVersion(EntityID id) noexcept
    {
        return static_cast<EntityVersion>((id >> 32) & 0xFFFF'FFFFull);
    }

    [[nodiscard]] inline EntityID MakeEntityID(EntityIndex index, EntityVersion version) noexcept
    {
        return (static_cast<EntityID>(version) << 32) | static_cast<EntityID>(index);
    }

    // ── Generador de IDs de tipo de componente ────────────────
    // Cada tipo T obtiene un ID único en tiempo de ejecución
    // la primera vez que se llama a GetComponentTypeID<T>().

    [[nodiscard]] inline ComponentTypeID NextComponentTypeID() noexcept
    {
        static ComponentTypeID counter = 0;
        return counter++;
    }

    template<typename T>
    [[nodiscard]] ComponentTypeID GetComponentTypeID() noexcept
    {
        static const ComponentTypeID id = NextComponentTypeID();
        return id;
    }

} // namespace ECS
