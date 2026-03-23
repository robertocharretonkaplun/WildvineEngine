#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h"
#include "SamplerState.h"
#include "RasterizerState.h"
//#include "BlendState.h"
#include "ShaderProgram.h"
#include "DepthStencilState.h"

class Device;
class DeviceContext;
class MeshComponent;

/**
 * @class   Actor
 * @brief   Representa una entidad gráfica dentro de la escena.
 *
 * @details Un Actor agrupa y administra todos los recursos necesarios para dibujar
 * un objeto tridimensional en el motor. Esto incluye la gestión de componentes
 * de malla (mallas, buffers de vértices e índices), texturas aplicadas, y
 * estados de renderizado (rasterización, blending, shaders). Adicionalmente,
 * proporciona soporte integrado para la generación y renderizado de sombras.
 *
 * @see     Entity
 */
class
	Actor : public Entity {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Actor() = default;

	/**
	 * @brief Constructor inicializador.
	 * * @param device Referencia al dispositivo gráfico utilizado para inicializar
	 * los recursos internos del actor.
	 */
	Actor(Device& device);

	/**
	 * @brief Destructor virtual por defecto.
	 */
	virtual
		~Actor() = default;

	/**
	 * @brief Inicializa el actor justo antes del primer ciclo de actualización.
	 * * Método heredado de @c Entity.
	 */
	void
		awake() override {}

	/**
	 * @brief Inicializa los recursos principales del actor.
	 *
	 * Método heredado de @c Entity. Diseñado para ser sobrescrito en clases
	 * derivadas que requieran configuración adicional.
	 */
	void
		init() override {}

	/**
	 * @brief Lógica de actualización por frame del actor.
	 *
	 * Se encarga de actualizar animaciones, transformaciones espaciales o lógicas
	 * que dependan del tiempo transcurrido.
	 *
	 * @param deltaTime     Tiempo transcurrido en segundos desde el último frame.
	 * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
	 */
	void
		update(float deltaTime, DeviceContext& deviceContext) override;

	/**
	 * @brief Envía el actor al pipeline gráfico para ser dibujado.
	 *
	 * Configura los estados de renderizado, buffers y shaders correspondientes
	 * antes de emitir la llamada de dibujo (draw call) para las mallas asociadas.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para emitir comandos de dibujo.
	 */
	void
		render(DeviceContext& deviceContext) override;

	/**
	 * @brief Renderiza el actor con configuraciones específicas para el Skybox.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para emitir comandos de dibujo.
	 */
	void
		renderForSkybox(DeviceContext& deviceContext);

	/**
	 * @brief Libera y destruye todos los recursos gráficos asociados al actor.
	 *
	 * Esto incluye la limpieza segura de buffers, estados de renderizado,
	 * shaders y texturas cargadas en memoria.
	 */
	void
		destroy();

	/**
	 * @brief Asigna y configura las mallas geométricas del actor.
	 *
	 * Almacena las mallas proporcionadas y se encarga de inicializar sus respectivos
	 * buffers de vértices e índices en la memoria de video.
	 *
	 * @param device Dispositivo gráfico utilizado para la creación de los buffers.
	 * @param meshes Colección de componentes de malla que formarán la geometría del actor.
	 */
	void
		setMesh(Device& device, std::vector<MeshComponent> meshes);

	/**
	 * @brief Obtiene el identificador de texto del actor.
	 * * @return Un @c std::string que representa el nombre actual del actor.
	 */
	std::string
		getName() { return m_name; }

	/**
	 * @brief Modifica el identificador de texto del actor.
	 * * @param name El nuevo nombre que se le asignará a la entidad.
	 */
	void
		setName(const std::string& name) { m_name = name; }

	/**
	 * @brief Asigna el conjunto de texturas que recubrirán al actor.
	 * * @param textures Colección de texturas a aplicar sobre las mallas del actor.
	 */
	void
		setTextures(std::vector<Texture> textures) { m_textures = textures; }

	/**
	 * @brief Activa o desactiva la capacidad del actor para proyectar sombras.
	 * * @param v @c true para habilitar la proyección de sombras; @c false para deshabilitarla.
	 */
	void
		setCastShadow(bool v) { castShadow = v; }

	/**
	 * @brief Verifica si la proyección de sombras está activa para este actor.
	 * * @return @c true si el actor está configurado para proyectar sombras.
	 */
	bool
		canCastShadow() const { return castShadow; }

	/**
	 * @brief Dibuja el actor en el mapa de sombras (Shadow Map).
	 *
	 * Utiliza un conjunto simplificado de shaders y estados específicamente
	 * diseñados para calcular la profundidad y proyectar la sombra del actor.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para emitir comandos de dibujo.
	 */
	void
		renderShadow(DeviceContext& deviceContext);

private:
	// ============================================================================
	// Recursos de Geometría y Apariencia
	// ============================================================================
	std::vector<MeshComponent> m_meshes;         ///< Colección de componentes que definen la geometría del actor.
	std::vector<Texture>       m_textures;       ///< Colección de texturas aplicadas a la superficie geométrica.
	std::vector<Buffer>        m_vertexBuffers;  ///< Buffers de video que almacenan los vértices de las mallas.
	std::vector<Buffer>        m_indexBuffers;   ///< Buffers de video que almacenan los índices para la topología.

	// ============================================================================
	// Estados de Renderizado y Buffers Constantes
	// ============================================================================
	//BlendState                 m_blendstate;     ///< Estado que define cómo se combinan los colores de los píxeles.
	//RasterizerState            m_rasterizer;     ///< Estado que define cómo se rasterizan los polígonos.
	SamplerState               m_sampler;        ///< Estado que define el filtrado y direccionamiento de texturas.
	CBChangesEveryFrame        m_model;          ///< Estructura de datos para las transformaciones matriciales del frame.
	Buffer                     m_modelBuffer;    ///< Buffer constante en GPU asociado a @c m_model.

	// ============================================================================
	// Recursos para Mapeo de Sombras
	// ============================================================================
	ShaderProgram              m_shaderShadow;            ///< Programa de shaders utilizado exclusivamente para la pasada de sombras.
	Buffer                     m_shaderBuffer;            ///< Buffer auxiliar para el envío de datos en el paso de sombras.
	//BlendState                 m_shadowBlendState;        ///< Estado de mezcla específico para evitar artefactos en sombras.
	DepthStencilState          m_shadowDepthStencilState; ///< Estado que regula las pruebas de profundidad al renderizar sombras.
	CBChangesEveryFrame        m_cbShadow;                ///< Buffer constante con matrices y datos específicos de la luz proyectante.

	// ============================================================================
	// Propiedades Generales
	// ============================================================================
	XMFLOAT4                   m_LightPos;       ///< Coordenadas posicionales de la luz que afecta la sombra del actor.
	std::string                m_name = "Actor"; ///< Cadena de texto para identificar al actor en la jerarquía o debug.
	bool                       castShadow = true;///< Bandera que determina si el actor debe incluirse en el pase de sombras.
};