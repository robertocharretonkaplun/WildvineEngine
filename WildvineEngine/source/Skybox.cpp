/**
 * @file Skybox.cpp
 * @brief Implementa la logica de Skybox dentro del subsistema Utilities.
 * @ingroup utilities
 */
#include "EngineUtilities/Utilities/Skybox.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"


HRESULT
Skybox::init(Device& device, DeviceContext* deviceContext, Texture& cubemap) {
	destroy();
	// Cargar el cubemap
	m_skyboxTexture = &cubemap;

	// 1) Geometr�a (cubo)
	 // Cubo unitario centrado en origen. (tama�o no importa si quitas traslaci�n)
	const SkyboxVertex vertices[] = {
			{-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1}, {+1,-1,-1}, // back
			{-1,-1,+1}, {-1,+1,+1}, {+1,+1,+1}, {+1,-1,+1}, // front
	};

	const unsigned int indices[] =
	{
		// back (-Z)
		0,1,2, 0,2,3,
		// front (+Z)
		4,6,5, 4,7,6,
		// left (-X)
		4,5,1, 4,1,0,
		// right (+X)
		3,2,6, 3,6,7,
		// top (+Y)
		1,5,6, 1,6,2,
		// bottom (-Y)
		4,0,3, 4,3,7
	};

	// Crear vertex buffer y index buffer para el skybox
	m_cubeModel = new Model3D("Skybox", vertices, indices);
	const std::vector<MeshComponent>& skyboxMeshes = m_cubeModel->GetMeshes();
	if (skyboxMeshes.empty()) {
		ERROR("Skybox", "Init", "Failed to create Skybox cube mesh.");
		return E_FAIL;
	}

	const MeshComponent& cubeMesh = skyboxMeshes.front();
	HRESULT bufferHr = m_vertexBuffer.init(device, cubeMesh, D3D11_BIND_VERTEX_BUFFER);
	if (FAILED(bufferHr)) {
		ERROR("Skybox", "Init", "Failed to create Skybox vertex buffer.");
		return bufferHr;
	}
	bufferHr = m_indexBuffer.init(device, cubeMesh, D3D11_BIND_INDEX_BUFFER);
	if (FAILED(bufferHr)) {
		ERROR("Skybox", "Init", "Failed to create Skybox index buffer.");
		return bufferHr;
	}
	m_indexCount = cubeMesh.m_numIndex;


	// Define the input layout
	LayoutBuilder builder;

	builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);

	HRESULT hr = S_OK;
	// Create the Shader Program
	hr = m_shaderProgram.init(device, "Skybox.hlsl", builder);
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create the constant buffers
	hr = m_constantBuffer.init(device, sizeof(CBSkybox));  // View
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Init SamplerState
	hr = m_samplerState.init(device);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new SamplerState");
	}

	// Init Rasterizer
	hr = m_rasterizerState.init(device, D3D11_FILL_SOLID, D3D11_CULL_FRONT, false, true);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new RasterizerState");
	}

	// Init DepthStencilState
	hr = m_depthStencilState.init(device, true,
															  D3D11_DEPTH_WRITE_MASK_ZERO,
															  D3D11_COMPARISON_LESS_EQUAL);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new DepthStencilState");
	}

	return S_OK;
}

void Skybox::update(DeviceContext& deviceContext, Camera& camera) {
	// 2) View sin traslaci�n + VP (SOLO una transpuesta al final)
	XMMATRIX viewNoT = camera.GetViewNoTranslation();
	XMMATRIX vp = viewNoT * camera.getProj();
	CBSkybox cb{};
	cb.mviewProj = XMMatrixTranspose(vp);
	m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

void
Skybox::render(DeviceContext& deviceContext) {
	// Guard: si no se inicializ� bien, no intentes renderizar
	if (!m_cubeModel || !m_skyboxTexture || !m_skyboxTexture->m_textureFromImg) return;

	// 1) States del skybox
	m_rasterizerState.render(deviceContext);
	m_depthStencilState.render(deviceContext, 0, false);

	m_constantBuffer.render(deviceContext, 0, 1);

	// 3) Shader + sampler (slot 10)
	m_shaderProgram.render(deviceContext);
	m_samplerState.render(deviceContext, 10, 1);

	// 4) IMPORTANT�SIMO: bindea cubemap ANTES del draw (slot 10)
	m_skyboxTexture->render(deviceContext, 10, 1);

	// 5) Asegura IA (topology + VB/IB) antes del DrawIndexed
	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_vertexBuffer.render(deviceContext, 0, 1);
	m_indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
	deviceContext.DrawIndexed(m_indexCount, 0, 0);

	// 5) Unbind t10
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);

	// 6) Limpia t0 para evitar mismatch por shaders 2D que usen t0
	deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);
}

void
Skybox::destroy() {
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
	m_indexCount = 0;

	delete m_cubeModel;
	m_cubeModel = nullptr;

	m_depthStencilState.destroy();
	m_rasterizerState.destroy();
	m_samplerState.destroy();
	m_constantBuffer.destroy();
	m_shaderProgram.destroy();

	m_skyboxTexture = nullptr;
}
