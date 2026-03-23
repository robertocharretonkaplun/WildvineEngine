#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"

class Device;
class DeviceContext;

/**
 * @class   Skybox
 * @brief   Renderiza un entorno tridimensional infinito alrededor de la cámara.
 *
 * La clase Skybox gestiona la creación y el dibujo de un cubo proyectado
 * en el fondo de la escena, utilizando una textura de tipo Cubemap. Internamente
 * maneja sus propios estados gráficos (Rasterizer para desactivar el culling de
 * caras internas, Depth Stencil para forzar el dibujo en el fondo) y se apoya en
 * la arquitectura de entidades mediante un @c Actor dedicado.
 */
class
	Skybox {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Skybox() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Skybox() = default;

	/**
	 * @brief Inicializa los recursos, mallas y estados gráficos del Skybox.
	 *
	 * Configura el modelo cúbico base, carga los shaders especializados para
	 * proyecciones de fondo, establece la textura del cubemap y configura
	 * los estados de profundidad y rasterización específicos (ej. dibujar
	 * caras internas y usar pruebas de profundidad LEQUAL).
	 *
	 * @param device        Dispositivo gráfico utilizado para crear los buffers y estados.
	 * @param deviceContext Contexto del dispositivo para operaciones inmediatas.
	 * @param cubemap       Textura de tipo Cubemap que recubrirá el entorno.
	 * @return              Código @c HRESULT que indica el éxito o fallo en la inicialización.
	 */
	HRESULT
		init(Device& device, DeviceContext* deviceContext, Texture& cubemap);

	/**
	 * @brief Actualiza las transformaciones del Skybox en base a la cámara.
	 *
	 * Calcula la matriz de Vista-Proyección (View-Projection) asegurándose de
	 * remover la traslación de la cámara, logrando así que el Skybox siga
	 * al jugador sin que este pueda acercarse a los bordes del cubo.
	 *
	 * @param deviceContext Contexto del dispositivo para actualizar el Constant Buffer.
	 * @param camera        Cámara principal activa en la escena.
	 */
	void
		update(DeviceContext& deviceContext, Camera& camera);

	/**
	 * @brief Dibuja el Skybox en la escena.
	 *
	 * Vincula los shaders, la textura del cubemap, los estados gráficos
	 * modificados y ejecuta la llamada de renderizado del actor cúbico interno.
	 *
	 * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera los recursos asociados al Skybox.
	 *
	 * (Actualmente delegando la limpieza a los destructores automáticos o al
	 * gestor de recursos).
	 */
	void
		destroy() {}

private:
		// ============================================================================
		// Recursos Gráficos
		// ============================================================================
		ShaderProgram       m_shaderProgram;      ///< Programa de shaders (Vertex y Pixel) específicos para proyectar el cubemap.
		Buffer              m_constantBuffer;     ///< Buffer constante que envía la matriz View-Projection sin traslación a la GPU.
		SamplerState        m_samplerState;       ///< Estado de muestreo para el filtrado lineal de la textura del cubemap.

		// ============================================================================
		// Estados del Pipeline
		// ============================================================================
		RasterizerState     m_rasterizerState;    ///< Estado configurado habitualmente para omitir el Backface Culling o invertirlo.
		DepthStencilState   m_depthStencilState;  ///< Estado configurado para forzar que el Skybox se dibuje siempre en el nivel de profundidad máximo.

		// ============================================================================
		// Geometría y Actores
		// ============================================================================
		Texture             m_skyboxTexture;      ///< Referencia a la textura del Cubemap.
		Model3D* m_cubeModel = nullptr;///< Puntero al modelo geométrico cúbico (o esfera) sobre el que se proyecta.
		EU::TSharedPointer<Actor> m_skybox;       ///< Entidad interna que encapsula la malla y permite renderizar la geometría.

};