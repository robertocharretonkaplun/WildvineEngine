#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

class
Material {
public:
	void setShader(ShaderProgram* shader) { m_shader = shader; }
	void setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }
	void setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }
	void setSamplerState(SamplerState* state) { m_samplerState = state; }
	void setDomain(MaterialDomain domain) { m_domain = domain; }

	ShaderProgram* getShader() const { return m_shader; }
	RasterizerState* getRasterizerState() const { return m_rasterizerState; }
	DepthStencilState* getDepthStencilState() const { return m_depthStencilState; }
	SamplerState* getSamplerState() const { return m_samplerState; }
	MaterialDomain getDomain() const { return m_domain; }

private:
	ShaderProgram* m_shader = nullptr;
	RasterizerState* m_rasterizerState = nullptr;
	DepthStencilState* m_depthStencilState = nullptr;
	SamplerState* m_samplerState = nullptr;
	MaterialDomain m_domain = MaterialDomain::Opaque;
};
