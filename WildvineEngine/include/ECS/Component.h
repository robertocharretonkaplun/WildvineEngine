#pragma once
#include "Prerequisites.h"
class DeviceContext;

/**
 * @class   Component
 * @brief   Clase base abstracta para la arquitectura basada en componentes.
 * 
 * La clase Component define el contrato estándar y el ciclo de vida
 * que todos los componentes del motor deben implementar. Los componentes encapsulan
 * datos y comportamientos específicos (como físicas, renderizado o lógica)
 * que pueden ser adjuntados a una entidad en la escena. Esta clase garantiza
 * que el motor pueda inicializar, actualizar, renderizar y destruir de
 * manera uniforme cualquier tipo de componente.
 */
class
	Component {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Component() = default;

	/**
	 * @brief Constructor inicializador.
	 *
	 * Asigna un identificador de tipo al componente en el momento de su creación.
	 *
	 * @param type Enumeración o valor que identifica de forma única la categoría
	 * o tipo de este componente.
	 */
	Component(const ComponentType type) : m_type(type) {}

	/**
	 * @brief Destructor virtual por defecto.
	 *
	 * Garantiza la correcta destrucción de los objetos de las clases derivadas.
	 */
	virtual
		~Component() = default;

	/**
	 * @brief Inicializa los recursos internos del componente.
	 *
	 * Método virtual puro. Se debe llamar una única vez tras la creación del
	 * componente y antes de que comience su ciclo de actualización, permitiendo
	 * reservar memoria o preparar dependencias.
	 */
	virtual void
		init() = 0;

	/**
	 * @brief Actualiza la lógica de comportamiento del componente.
	 *
	 * Método virtual puro invocado una vez por frame. Aquí reside la lógica
	 * principal dependiente del tiempo.
	 *
	 * @param deltaTime Tiempo transcurrido en segundos desde el último frame.
	 */
	virtual void
		update(float deltaTime) = 0;

	/**
	 * @brief Envía la representación visual del componente al pipeline gráfico.
	 *
	 * Método virtual puro encargado de ejecutar llamadas de dibujo o
	 * transferir datos a la GPU si el componente tiene una representación visual.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
	 */
	virtual void
		render(DeviceContext& deviceContext) = 0;

	/**
	 * @brief Libera y limpia de manera segura los recursos del componente.
	 *
	 * Método virtual puro que debe implementarse para garantizar que la memoria
	 * y los recursos gráficos se liberen correctamente antes de destruir el objeto.
	 */
	virtual void
		destroy() = 0;

	/**
	 * @brief Obtiene el identificador de tipo del componente.
	 *
	 * Útil para realizar conversiones de tipo seguras (casting) o buscar
	 * componentes específicos dentro de una entidad.
	 *
	 * @return El tipo de componente definido por @c ComponentType.
	 */
	ComponentType
		getType() const { return m_type; }

protected:
	// ============================================================================
	// Propiedades Generales
	// ============================================================================
	ComponentType m_type; ///< Identificador de tipo asociado a la instancia del componente.
};