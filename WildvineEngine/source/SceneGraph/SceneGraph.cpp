/**
 * @file SceneGraph.cpp
 * @brief Implementa la logica de SceneGraph dentro del subsistema SceneGraph.
 * @ingroup scenegraph
 */
#include "SceneGraph\SceneGraph.h"
#include "SceneGraph\HierarchyComponent.h"
#include "FixedECS\ECS.h"
#include "Components\Transform.h"
#include "Components\LightComponent.h"
#include "Components\MeshRendererComponent.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/RenderScene.h"

void SceneGraph::init(ECS::Registry& registry) {
	m_registry = &registry;
	m_entities.clear();
}

void SceneGraph::destroy() {
	if (m_registry) {
		for (ECS::EntityID e : m_entities)
		{
			if (!m_registry->IsAlive(e)) continue;
			auto* h = m_registry->TryGetComponent<HierarchyComponent>(e);
			if (h)
			{
				h->m_parent = ECS::NULL_ENTITY;
				h->m_children.clear();
			}
		}
	}

	m_entities.clear();
}

void
SceneGraph::addEntity(ECS::EntityID e) {
	if (!m_registry || !m_registry->IsAlive(e)) {
		return;
	}
	if (isRegistered(e)) {
		return;
	}

	// Validar que existen los componentes minimos
	if (!m_registry->HasComponent<Transform>(e)) {
		m_registry->AddComponent<Transform>(e);
	}
	if (!m_registry->HasComponent<HierarchyComponent>(e)) {
		m_registry->AddComponent<HierarchyComponent>(e);
	}

	m_entities.push_back(e);
}

void
SceneGraph::removeEntity(ECS::EntityID e) {
	if (!m_registry) return;
	if (!isRegistered(e)) return;

	// 1) Detach de su padre (si tiene)
	detach(e);

	// 2) Reparent de hijos a null (roots)
	auto* h = m_registry->TryGetComponent<HierarchyComponent>(e);
	if (h)
	{
		// Copia local para no invalidar mientras iteras
		auto childrenCopy = h->m_children;
		for (ECS::EntityID c : childrenCopy)
		{
			if (!m_registry->IsAlive(c)) continue;
			// detach del padre (que es e)
			auto* hc = m_registry->TryGetComponent<HierarchyComponent>(c);
			if (hc && hc->m_parent == e)
				hc->m_parent = ECS::NULL_ENTITY;

			// quitar referencia en e
			h->removeChild(c);
		}

		h->m_children.clear();
	}

	// 3) eliminar del registro
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
}

bool
SceneGraph::isAncestor(ECS::EntityID possibleAncestor, ECS::EntityID node) const {
	// Recorre hacia arriba desde node: si encuentra possibleAncestor, hay ciclo
	if (!m_registry) return false;
	if (possibleAncestor == ECS::NULL_ENTITY || node == ECS::NULL_ENTITY) return false;

	auto* h = m_registry->TryGetComponent<HierarchyComponent>(node);
	while (h && h->m_parent != ECS::NULL_ENTITY)
	{
		if (h->m_parent == possibleAncestor) return true;
		node = h->m_parent;
		h = m_registry->IsAlive(node)
			? m_registry->TryGetComponent<HierarchyComponent>(node)
			: nullptr;
	}
	return false;
}

bool
SceneGraph::isRoot(ECS::EntityID e) const {
	if (!m_registry || !m_registry->IsAlive(e)) return false;
	auto* h = m_registry->TryGetComponent<HierarchyComponent>(e);
	return (!h || h->m_parent == ECS::NULL_ENTITY);
}

bool
SceneGraph::isRegistered(ECS::EntityID e) const {
	return std::find(m_entities.begin(), m_entities.end(), e) != m_entities.end();
}

bool
SceneGraph::attach(ECS::EntityID child, ECS::EntityID parent)
{
	if (!m_registry) return false;
	if (child == ECS::NULL_ENTITY || parent == ECS::NULL_ENTITY) return false;
	if (child == parent) return false;

	// Registro automatico
	addEntity(child);
	addEntity(parent);

	// Evita ciclos: parent no puede estar debajo de child
	if (isAncestor(child, parent)) return false;

	// Si child ya tiene padre, detach
	detach(child);

	auto* hc = m_registry->TryGetComponent<HierarchyComponent>(child);
	auto* hp = m_registry->TryGetComponent<HierarchyComponent>(parent);
	if (!hc || !hp) return false;

	hc->m_parent = parent;
	hp->addChild(child);

	return true;
}

bool
SceneGraph::detach(ECS::EntityID child) {
	if (!m_registry || child == ECS::NULL_ENTITY) return false;

	auto* hc = m_registry->TryGetComponent<HierarchyComponent>(child);
	if (!hc) return false;

	ECS::EntityID parent = hc->m_parent;
	if (parent == ECS::NULL_ENTITY) return true; // ya estaba root

	if (m_registry->IsAlive(parent)) {
		auto* hp = m_registry->TryGetComponent<HierarchyComponent>(parent);
		if (hp) hp->removeChild(child);
	}

	hc->m_parent = ECS::NULL_ENTITY;

	return true;
}

void
SceneGraph::update(float deltaTime) {
	(void)deltaTime;
	if (!m_registry) return;

	// 1) Recompone matrices locales de todas las entidades con Transform
	m_registry->GetView<Transform>().Each(
		[](ECS::EntityID, Transform& t) {
			t.updateMatrix();
		});

	// 2) Propagacion World: procesa roots
	for (ECS::EntityID e : m_entities)
	{
		if (!m_registry->IsAlive(e)) continue;
		if (isRoot(e))
		{
			updateWorldRecursive(e, XMMatrixIdentity());
		}
	}
}

void
SceneGraph::updateWorldRecursive(ECS::EntityID node, const XMMATRIX& parentWorld) {
	auto* t = m_registry->TryGetComponent<Transform>(node);
	auto* h = m_registry->TryGetComponent<HierarchyComponent>(node);

	if (!t || !h) {
		return;
	}
	// Transform::matrix es LOCAL (S*R*T)
	// World = Local * ParentWorld
	auto worldMatrix = t->matrix * parentWorld;
	t->worldMatrix = worldMatrix;

	for (ECS::EntityID c : h->m_children) {
		if (m_registry->IsAlive(c)) {
			updateWorldRecursive(c, worldMatrix);
		}
	}
}

void
SceneGraph::gatherRenderScene(RenderScene& outScene, const Camera& camera) {
	if (!m_registry) return;

	for (ECS::EntityID entity : m_entities)
	{
		if (!m_registry->IsAlive(entity)) {
			continue;
		}

		auto* lightComponent = m_registry->TryGetComponent<LightComponent>(entity);
		if (lightComponent) {
			LightData lightData = lightComponent->getLightData();
			auto* transform = m_registry->TryGetComponent<Transform>(entity);
			if (transform) {
				XMFLOAT4X4 worldMatrix{};
				XMStoreFloat4x4(&worldMatrix, transform->worldMatrix);
				lightData.position = EU::Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

				XMVECTOR localLightDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
				XMVECTOR worldLightDirection = XMVector3Normalize(XMVector3TransformNormal(localLightDirection, transform->worldMatrix));
				lightData.direction = EU::Vector3(
					XMVectorGetX(worldLightDirection),
					XMVectorGetY(worldLightDirection),
					XMVectorGetZ(worldLightDirection));
			}

			outScene.directionalLights.push_back(lightData);
		}

		auto* meshRenderer = m_registry->TryGetComponent<MeshRendererComponent>(entity);
		auto* transform = m_registry->TryGetComponent<Transform>(entity);
		if (!meshRenderer || !transform || !meshRenderer->isVisible()) {
			continue;
		}

		RenderObject renderObject{};
		renderObject.mesh = meshRenderer->getMesh();
		renderObject.materialInstance = meshRenderer->getMaterialInstance();
		renderObject.materialInstances = meshRenderer->getMaterialInstances();
		renderObject.world = transform->worldMatrix;
		renderObject.castShadow = meshRenderer->canCastShadow();

		EU::Vector3 cameraPos = camera.getPosition();
		XMFLOAT4X4 worldMatrix{};
		XMStoreFloat4x4(&worldMatrix, transform->worldMatrix);
		EU::Vector3 objectPos = EU::Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);
		float dx = objectPos.x - cameraPos.x;
		float dy = objectPos.y - cameraPos.y;
		float dz = objectPos.z - cameraPos.z;
		renderObject.distanceToCamera = dx * dx + dy * dy + dz * dz;

		bool hasOpaqueSubmesh = false;
		bool hasTransparentSubmesh = false;
		if (renderObject.mesh && !renderObject.materialInstances.empty()) {
			const std::vector<Submesh>& submeshes = renderObject.mesh->getSubmeshes();
			for (const Submesh& submesh : submeshes) {
				MaterialInstance* materialInstance = renderObject.materialInstance;
				if (submesh.materialSlot < renderObject.materialInstances.size() &&
					renderObject.materialInstances[submesh.materialSlot]) {
					materialInstance = renderObject.materialInstances[submesh.materialSlot];
				}

				Material* material = materialInstance ? materialInstance->getMaterial() : nullptr;
				const MaterialDomain domain = material ? material->getDomain() : MaterialDomain::Opaque;
				if (domain == MaterialDomain::Transparent) {
					hasTransparentSubmesh = true;
				}
				else {
					hasOpaqueSubmesh = true;
				}
			}
		}
		else {
			Material* material = renderObject.materialInstance ? renderObject.materialInstance->getMaterial() : nullptr;
			const MaterialDomain domain = material ? material->getDomain() : MaterialDomain::Opaque;
			hasTransparentSubmesh = (domain == MaterialDomain::Transparent);
			hasOpaqueSubmesh = !hasTransparentSubmesh;
		}

		if (hasTransparentSubmesh) {
			renderObject.transparent = true;
			outScene.transparentObjects.push_back(renderObject);
		}
		if (hasOpaqueSubmesh) {
			renderObject.transparent = false;
			outScene.opaqueObjects.push_back(renderObject);
		}
	}
}
