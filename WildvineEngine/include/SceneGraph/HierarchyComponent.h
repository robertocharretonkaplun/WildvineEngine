/**
 * @file HierarchyComponent.h
 * @brief Declara la API de HierarchyComponent dentro del subsistema SceneGraph.
 * @ingroup scenegraph
 */
#pragma once
#include "FixedECS/Types.h"
#include <algorithm>
#include <vector>

/**
 * @struct HierarchyComponent
 * @brief Relacion padre-hijos de una entidad dentro del SceneGraph.
 *
 * Guarda EntityIDs (no punteros): el versionado del nuevo ECS invalida
 * automaticamente referencias a entidades destruidas.
 */
struct
HierarchyComponent {
	// API SceneGraph
	void
	setParent(ECS::EntityID parent) {
		m_parent = parent;
	}

	bool
	isRoot() const {
		return m_parent == ECS::NULL_ENTITY;
	}

	bool
	hasChildren() const {
		return !m_children.empty();
	}

	void
	addChild(ECS::EntityID child) {
		if (child == ECS::NULL_ENTITY) {
			return;
		}

		if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
			return;
		}
		m_children.push_back(child);
	}

	void
	removeChild(ECS::EntityID child) {
		if (child == ECS::NULL_ENTITY) return;

		m_children.erase(
			std::remove(m_children.begin(), m_children.end(), child),
			m_children.end()
		);
	}

	ECS::EntityID m_parent = ECS::NULL_ENTITY;
	std::vector<ECS::EntityID> m_children;
};
