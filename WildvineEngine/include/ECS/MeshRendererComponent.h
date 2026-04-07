#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class Mesh;
class MaterialInstance;
class DeviceContext;

class
MeshRendererComponent : public Component {
public:
	MeshRendererComponent()
		: Component(ComponentType::MESH) {}

	void init() override {}
	void update(float deltaTime) override {}
	void render(DeviceContext& deviceContext) override {}
	void destroy() override {}

	void setMesh(Mesh* mesh) { m_mesh = mesh; }
	Mesh* getMesh() const { return m_mesh; }

	void setMaterialInstance(MaterialInstance* materialInstance) {
		m_materialInstance = materialInstance;
	}
	MaterialInstance* getMaterialInstance() const { return m_materialInstance; }

	bool isVisible() const { return m_visible; }
	void setVisible(bool visible) { m_visible = visible; }

	bool canCastShadow() const { return m_castShadow; }
	void setCastShadow(bool value) { m_castShadow = value; }

private:
	Mesh* m_mesh = nullptr;
	MaterialInstance* m_materialInstance = nullptr;
	bool m_visible = true;
	bool m_castShadow = true;
};
