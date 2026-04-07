#include "Rendering/MaterialInstance.h"
#include "DeviceContext.h"
#include "Texture.h"

void
MaterialInstance::bindTextures(DeviceContext& deviceContext) const {
	if (m_albedo) {
		m_albedo->render(deviceContext, 0, 1);
	}
	if (m_normal) {
		m_normal->render(deviceContext, 1, 1);
	}
	if (m_metallic) {
		m_metallic->render(deviceContext, 2, 1);
	}
	if (m_roughness) {
		m_roughness->render(deviceContext, 3, 1);
	}
	if (m_ao) {
		m_ao->render(deviceContext, 4, 1);
	}
}
