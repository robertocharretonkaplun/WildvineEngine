#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"
class DeviceContext;

/**
 * @class   MeshComponent
 * @brief   Componente que almacena la información geométrica (malla) de una entidad.
 *
 * MeshComponent encapsula los datos crudos que definen la forma
 * tridimensional de un objeto dentro de la arquitectura ECS de MonacoEngine3.
 * Contiene los arreglos de vértices e índices necesarios para construir primitivas
 * geométricas, y se asocia a entidades visuales como los @c Actor.
 */
class
	MeshComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa los contadores de vértices e índices en cero y registra
	 * el componente automáticamente bajo el identificador de tipo @c ComponentType::MESH.
	 */
	MeshComponent() : m_numVertex(0), 
					  m_numIndex(0), 
					  Component(ComponentType::MESH) {}

	/**
	 * @brief Destructor virtual por defecto.
	 */
	virtual
		~MeshComponent() = default;

	/**
	 * @brief Inicializa los recursos del componente de malla.
	 *
	 * Método heredado de @c Component. Puede usarse para reservar memoria inicial
	 * o preparar integraciones con los buffers de DirectX antes del ciclo principal.
	 */
	void
		init() override {};

	/**
	 * @brief Actualiza la lógica o el estado de la geometría.
	 *
	 * Método heredado de @c Component. Pensado para futuras implementaciones de
	 * animaciones a nivel de vértices (morph targets o vertex displacement).
	 *
	 * @param deltaTime Tiempo transcurrido en segundos desde la última actualización.
	 */
	void
		update(float deltaTime) override {};

	/**
	 * @brief Procesa el componente para la fase de renderizado.
	 *
	 * Método heredado de @c Component. Proveedor de interfaz para delegar u organizar
	 * comandos de dibujo hacia el @c DeviceContext.
	 *
	 * @param deviceContext Contexto del dispositivo para operaciones gráficas.
	 */
	void
		render(DeviceContext& deviceContext) override {};

	/**
	 * @brief Libera los recursos asociados al componente geométrico.
	 *
	 * Método heredado de @c Component. Útil para vaciar vectores o notificar
	 * a otros subsistemas que la memoria ram de estos vértices puede ser reclamada.
	 */
	void
		destroy() override {};

public:
		// ============================================================================
		// Propiedades Generales
		// ============================================================================
		std::string m_name;                     ///< Identificador lógico o nombre del archivo origen de la malla.

		// ============================================================================
		// Datos Geométricos Crudos
		// ============================================================================
		std::vector<SimpleVertex> m_vertex;     ///< Colección de vértices estándar (posición, normal, coordenadas UV, etc.).
		std::vector<SkyboxVertex> m_skyVertex;  ///< Colección de vértices especializados para la proyección del Skybox.
		std::vector<unsigned int> m_index;      ///< Colección de índices que definen la topología (conectividad de triángulos).

		// ============================================================================
		// Contadores
		// ============================================================================
		int m_numVertex;                        ///< Cantidad total de vértices registrados en el componente.
		int m_numIndex;                         ///< Cantidad total de índices registrados en el componente.
};