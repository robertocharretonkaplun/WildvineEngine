/**
 * @file SceneGraph.h
 * @brief Declara la API de SceneGraph dentro del subsistema SceneGraph.
 * @ingroup scenegraph
 */
#pragma once
#include "Prerequisites.h"
#include "FixedECS/Types.h"
#include <vector>

namespace ECS { class Registry; }
class Camera;
class RenderScene;

/**
 * @class SceneGraph
 * @brief Administra la jerarquia de entidades y su actualizacion espacial.
 *
 * El `SceneGraph` registra entidades del `ECS::Registry`, resuelve relaciones
 * padre-hijo (via `HierarchyComponent`) y actualiza las matrices de mundo
 * antes de generar la informacion necesaria para render.
 */
class
SceneGraph {
public:
	SceneGraph()  = default;
	~SceneGraph() = default;

	void
	init(ECS::Registry& registry);

	/**
	 * @brief Registra una entidad dentro del grafo.
	 *
	 * Garantiza que la entidad tenga `Transform` y `HierarchyComponent`.
	 * @param e Entidad a registrar.
	 */
	void
	addEntity(ECS::EntityID e);

	/**
	 * @brief Elimina una entidad del grafo si esta registrada.
	 * @param e Entidad a retirar.
	 */
	void
	removeEntity(ECS::EntityID e);

	bool
	isAncestor(ECS::EntityID possibleAncestor, ECS::EntityID node) const;

	bool
	attach(ECS::EntityID child, ECS::EntityID parent);

	bool
	detach(ECS::EntityID child);

	void
	update(float deltaTime);

	void
	gatherRenderScene(RenderScene& outScene, const Camera& camera);

	void
	destroy();
private:
	void
	updateWorldRecursive(ECS::EntityID node, const XMMATRIX& parentWorld);

	bool
	isRoot(ECS::EntityID e) const;

	bool
	isRegistered(ECS::EntityID e) const;

private:
	ECS::Registry* m_registry = nullptr;
public:
	std::vector<ECS::EntityID> m_entities; ///< Entidades registradas en el grafo.
};
