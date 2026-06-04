/**
 * @file RenderPipeline.h
 * @brief Declara el orquestador de renderers de escena del motor.
 * @ingroup rendering
 */
#pragma once
#include "Rendering/DeferredRenderer.h"

/**
 * @class RenderPipeline
 * @brief Selecciona y ejecuta el renderer activo para el frame actual.
 */
class
RenderPipeline {
public:
	HRESULT init(Device& device);
	HRESULT setRendererType(RendererType rendererType, Device& device);
	void resize(Device& device, unsigned int width, unsigned int height);
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);
	void destroy();

	RendererType getRendererType() const { return RendererType::Deferred; }
	const char* getActiveRendererName() const;
	ID3D11ShaderResourceView* getShadowMapSRV() const;
	ID3D11ShaderResourceView* getPreShadowSRV() const;
	ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const;
	ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const;
	ID3D11ShaderResourceView* getGBufferWorldAoSRV() const;
	ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const;
	void setShadowFactorDebugEnabled(bool enabled);
	void setDeferredDebugViewMode(int mode);
	void setEditorGizmosVisible(bool visible);

private:
	HRESULT ensureDeferredInitialized(Device& device);
	DeferredRenderer m_deferredRenderer;
	bool m_deferredInitialized = false;
};
