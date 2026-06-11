#pragma once

#include <tuple>
#include <cstddef>
#include "ComponentPool.h"

// ============================================================
//  ECS :: View.h
//  Iteración eficiente sobre entidades que poseen un conjunto
//  de componentes.
//
//  Uso:
//      auto view = registry.GetView<Transform, Velocity>();
//      view.Each([](EntityID e, Transform& t, Velocity& v) {
//          t.position += v.direction * v.speed;
//      });
//
//  Estrategia:
//    - Detecta el pool más pequeño en tiempo de construcción
//    - Itera solo ese pool y filtra las entidades que no
//      tienen TODOS los demás componentes.
//    - Recorre en orden inverso para permitir eliminar
//      componentes/entidades durante la iteración.
// ============================================================

namespace ECS {

    template<typename... Components>
    class View
    {
    private:
        std::tuple<ComponentPool<Components>*...> m_pools;
        const SparseSet* m_smallest = nullptr;

        // Encuentra el pool con menos elementos (mejor filtro)
        template<std::size_t I = 0>
        void FindSmallest() noexcept
        {
            if constexpr (I < sizeof...(Components)) {
                auto* pool = std::get<I>(m_pools);
                if (pool && (!m_smallest || pool->Size() < m_smallest->Size()))
                    m_smallest = pool;
                FindSmallest<I + 1>();
            }
        }

        // Devuelve true si la entidad está en TODOS los pools
        [[nodiscard]] bool AllHave(EntityID entity) const noexcept
        {
            return std::apply(
                [entity](auto*... pools) noexcept {
                    return (... && (pools && pools->Contains(entity)));
                },
                m_pools);
        }

    public:
        explicit View(ComponentPool<Components>*... pools) noexcept
            : m_pools(pools...)
        {
            FindSmallest();
        }

        // ── Iteración principal ───────────────────────────────
        // Callback: void(EntityID, Components&...)
        template<typename Func>
        void Each(Func&& func)
        {
            if (!m_smallest) return;

            const auto& entities = m_smallest->GetEntities();

            // Recorrido inverso → seguro al eliminar durante la iteración
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))
                {
                    std::apply(
                        [&](auto*... pools) {
                            func(entity, pools->Get(entity)...);
                        },
                        m_pools);
                }
            }
        }

        // ── Iteración solo de entidades ───────────────────────
        // Útil cuando solo necesitas el EntityID y accedes a
        // componentes manualmente.
        template<typename Func>
        void EachEntity(Func&& func)
        {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityID entity = entities[i - 1];
                if (AllHave(entity))
                    func(entity);
            }
        }

        [[nodiscard]] bool  Empty() const noexcept { return !m_smallest || m_smallest->Empty(); }
        [[nodiscard]] std::size_t Size() const noexcept { return m_smallest ? m_smallest->Size() : 0; }
    };

} // namespace ECS
