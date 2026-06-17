/**
 * @file RenderTypes.h
 * @brief Declara la API de RenderTypes dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"
#include <cmath>

class Mesh;
class MaterialInstance;

enum class
MaterialDomain {
	Opaque = 0,
	Masked,
	Transparent
};

enum class
BlendMode {
	Opaque = 0,
	Alpha,
	Additive,
	PremultipliedAlpha
};

enum class
RenderPassType {
	Shadow = 0,
	Opaque,
	Skybox,
	Transparent,
	Editor
};

enum class
LightType {
	Directional = 0,
	Point,
	Spot,
	Rect
};

constexpr int kMaxSceneLights = 8;
constexpr float kDefaultPointLightRange = 10.0f;
constexpr float kPointLightVisibilityThreshold = 0.01f;

struct
LightData {
	LightType type = LightType::Directional;
	EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;

	EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f);
	float range = 0.0f;

	EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);
	float spotAngle = 0.0f;

	EU::Vector2 rectSize = EU::Vector2(2.0f, 2.0f);
};

inline float
CalculatePointLightEnergy(const LightData& light) {
	const float red = light.color.x > 0.0f ? light.color.x : 0.0f;
	const float green = light.color.y > 0.0f ? light.color.y : 0.0f;
	const float blue = light.color.z > 0.0f ? light.color.z : 0.0f;
	const float maxChannel = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
	const float intensity = light.intensity > 0.0f ? light.intensity : 0.0f;
	return maxChannel * intensity;
}

inline float
CalculateLocalLightInfluenceRadius(const LightData& light) {
	if (light.range > 0.0f) {
		return light.range;
	}

	const float energy = CalculatePointLightEnergy(light);
	if (energy <= 0.0f) {
		return 0.0f;
	}

	return static_cast<float>(std::sqrt(energy / kPointLightVisibilityThreshold));
}

inline float
CalculatePointLightInfluenceRadius(const LightData& light) {
	return CalculateLocalLightInfluenceRadius(light);
}

inline float
ResolveLightRange(const LightData& light) {
	if (light.type == LightType::Point ||
		light.type == LightType::Spot ||
		light.type == LightType::Rect) {
		return CalculateLocalLightInfluenceRadius(light);
	}

	return light.range > 0.0f ? light.range : kDefaultPointLightRange;
}

inline float
ResolveSpotAngleDegrees(const LightData& light) {
	return light.spotAngle > 0.0f ? light.spotAngle : 45.0f;
}

inline EU::Vector2
ResolveRectLightSize(const LightData& light) {
	const float width = light.rectSize.x > 0.0f ? light.rectSize.x : 2.0f;
	const float height = light.rectSize.y > 0.0f ? light.rectSize.y : 2.0f;
	return EU::Vector2(width, height);
}

struct
MaterialParams {
	XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	float metallic = 1.0f;
	float roughness = 1.0f;
	float ao = 1.0f;
	float normalScale = 1.0f;
	float emissiveStrength = 1.0f;
	float alphaCutoff = 0.5f;
};

struct alignas(16)
CBPerFrame {
	XMFLOAT4X4 View{};
	XMFLOAT4X4 Projection{};
	XMFLOAT4X4 LightViewProjection{};
	EU::Vector3 CameraPos{};
	float pad0 = 0.0f;
	EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	float pad1 = 0.0f;
	EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	float LightRange = 10.0f;
	EU::Vector3 LightPosition = EU::Vector3(0.0f, 3.0f, 0.0f);
	int LightType = 0;
	XMFLOAT4 LightPositionsRanges[kMaxSceneLights]{};
	XMFLOAT4 LightColorsTypes[kMaxSceneLights]{};
	XMFLOAT4 LightDirectionsIntensities[kMaxSceneLights]{};
	XMFLOAT4 LightSpotRectParams[kMaxSceneLights]{};
	int LightCount = 0;
	XMFLOAT3 pad2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

struct
CBPerObject {
	XMFLOAT4X4 World{};
};

struct
CBPerMaterial {
	XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	float Metallic = 1.0f;
	float Roughness = 1.0f;
	float AO = 1.0f;
	float NormalScale = 1.0f;
	float EmissiveStrength = 1.0f;
	float AlphaCutoff = 0.0f;
	float pad0 = 0.0f;
	float pad1 = 0.0f;
	float pad2 = 0.0f;
	float pad3 = 0.0f;
	float pad4 = 0.0f;
	float pad5 = 0.0f;
};

struct
RenderObject {
	Mesh* mesh = nullptr;
	MaterialInstance* materialInstance = nullptr;
	std::vector<MaterialInstance*> materialInstances;
	XMMATRIX world = XMMatrixIdentity();
	bool castShadow = true;
	bool transparent = false;
	float distanceToCamera = 0.0f;
};


