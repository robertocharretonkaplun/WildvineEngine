/**
 * @file LightComponent.h
 * @brief Componente de luz (dato puro) para el ECS.
 * @ingroup components
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

/**
 * @struct LightComponent
 * @brief Parametros de una luz de escena (direccional, puntual, spot o rect).
 */
struct
LightComponent {
	LightData& getLightData() { return m_light; }
	const LightData& getLightData() const { return m_light; }

	void setCastShadow(bool value) { m_castShadow = value; }
	bool canCastShadow() const { return m_castShadow; }

	LightData m_light;
	bool m_castShadow = false;
};
