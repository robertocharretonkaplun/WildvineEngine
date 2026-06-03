/**
 * @file RenderPipeline.cpp
 * @brief Implementa el selector de renderers de escena del motor.
 * @ingroup rendering
 */
#include "Rendering/RenderPipeline.h"

HRESULT
RenderPipeline::init(Device& device) {
	m_deferredInitialized = false;
	return ensureDeferredInitialized(device);
}

HRESULT
RenderPipeline::setRendererType(RendererType rendererType, Device& device) {
	if (rendererType != RendererType::Deferred) {
		return E_INVALIDARG;
	}

	HRESULT hr = ensureDeferredInitialized(device);
	if (FAILED(hr)) {
		return hr;
	}

	MESSAGE("RenderPipeline", "setRendererType", m_deferredRenderer.getDebugName());
	return S_OK;
}

void
RenderPipeline::resize(Device& device, unsigned int width, unsigned int height) {
	if (m_deferredInitialized) {
		m_deferredRenderer.resize(device, width, height);
	}
}

void
RenderPipeline::render(DeviceContext& deviceContext,
	const Camera& camera,
	RenderScene& scene,
	EditorViewportPass& viewportPass) {
	if (m_deferredInitialized) {
		m_deferredRenderer.render(deviceContext, camera, scene, viewportPass);
	}
}

void
RenderPipeline::destroy() {
	if (m_deferredInitialized) {
		m_deferredRenderer.destroy();
		m_deferredInitialized = false;
	}
}

const char*
RenderPipeline::getActiveRendererName() const {
	return m_deferredInitialized ? m_deferredRenderer.getDebugName() : "NoRenderer";
}

ID3D11ShaderResourceView*
RenderPipeline::getShadowMapSRV() const {
	return m_deferredRenderer.getShadowMapSRV();
}

ID3D11ShaderResourceView*
RenderPipeline::getPreShadowSRV() const {
	return m_deferredRenderer.getPreShadowSRV();
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferAlbedoMetallicSRV() const {
	return m_deferredRenderer.getGBufferAlbedoMetallicSRV();
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferNormalRoughnessSRV() const {
	return m_deferredRenderer.getGBufferNormalRoughnessSRV();
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferWorldAoSRV() const {
	return m_deferredRenderer.getGBufferWorldAoSRV();
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferEmissiveAlphaSRV() const {
	return m_deferredRenderer.getGBufferEmissiveAlphaSRV();
}

void
RenderPipeline::setShadowFactorDebugEnabled(bool enabled) {
	m_deferredRenderer.setShadowFactorDebugEnabled(enabled);
}

void
RenderPipeline::setDeferredDebugViewMode(int mode) {
	m_deferredRenderer.setDeferredDebugViewMode(mode);
}

HRESULT
RenderPipeline::ensureDeferredInitialized(Device& device) {
	if (!m_deferredInitialized) {
		HRESULT hr = m_deferredRenderer.init(device);
		if (FAILED(hr)) {
			return hr;
		}
		m_deferredInitialized = true;
	}
	return S_OK;
}

