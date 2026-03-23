#pragma once
#include "Prerequisites.h"

class Entity;
class DeviceContext;

/**
 * @class   SceneGraph
 * @brief   Estructura de datos que administra la jerarquía espacial y lógica de las entidades.
 *
 * El SceneGraph (Grafo de Escena) es el sistema central encargado de gestionar
 * todas las entidades vivas del motor. Organiza los objetos en una estructura de árbol
 * evaluando las relaciones de parentesco (padre-hijo). Esto permite la propagación
 * eficiente de transformaciones espaciales (matrices de mundo) y garantiza que las
 * actualizaciones y los ciclos de renderizado se ejecuten en el orden correcto.
 */
class
	SceneGraph {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	SceneGraph() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~SceneGraph() = default;

	/**
	 * @brief Inicializa el grafo de escena.
	 *
	 * Prepara las estructuras internas para comenzar a recibir y procesar entidades.
	 */
	void
		init();

	/**
	 * @brief Registra una nueva entidad en el grafo de escena.
	 *
	 * Incorpora la entidad a la lista global administrada por el SceneGraph para que
	 * sea tomada en cuenta durante los ciclos de actualización y renderizado.
	 *
	 * @param e Puntero a la entidad a registrar.
	 */
	void
		addEntity(Entity* e);  // registra en el grafo

	/**
	 * @brief Elimina una entidad del grafo de escena.
	 *
	 * Desvincula la entidad del procesamiento global. Si la entidad tiene hijos,
	 * el usuario o el sistema deberá gestionar la reasignación o eliminación en cascada.
	 *
	 * @param e Puntero a la entidad a remover.
	 */
	void
		removeEntity(Entity* e);

	/**
	 * @brief Verifica si una entidad es un ancestro lógico de otra.
	 *
	 *
	 * @param possibleAncestor La entidad que podría ser el ancestro (padre, abuelo, etc.).
	 * @param node             La entidad de referencia desde la cual se inicia la búsqueda.
	 * @return                 @c true si se confirma la relación de ascendencia; @c false en caso contrario.
	 */
	bool
		isAncestor(Entity* possibleAncestor, Entity* node) const;

	/**
	 * @brief Establece una relación padre-hijo entre dos entidades.
	 *
	 * Vincula un nodo hijo a un nodo padre dentro de la jerarquía espacial,
	 * asegurando que las transformaciones del padre afecten al hijo.
	 *
	 * @param child  Puntero a la entidad subordinada.
	 * @param parent Puntero a la entidad de jerarquía superior.
	 * @return       @c true si la vinculación fue exitosa; @c false si ocurrió un error (ej. creación de ciclos).
	 */
	bool
		attach(Entity* child, Entity* parent);

	/**
	 * @brief Rompe la relación jerárquica de una entidad subordinada.
	 *
	 * Convierte a la entidad en un nodo raíz independiente dentro del grafo,
	 * eliminando su dependencia de transformaciones heredadas.
	 *
	 * @param child Puntero a la entidad que se desvinculará de su padre actual.
	 * @return      @c true si la desvinculación fue exitosa.
	 */
	bool
		detach(Entity* child);

	/**
	 * @brief Actualiza todas las entidades administradas por el grafo.
	 *
	 * Recorre los nodos raíz y propaga la actualización y las transformaciones espaciales
	 * en cascada a todos sus descendientes.
	 *
	 * @param deltaTime     Tiempo transcurrido en segundos desde el último frame.
	 * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
	 */
	void
		update(float deltaTime, DeviceContext& deviceContext);

	/**
	 * @brief Envía las entidades visibles del grafo al pipeline gráfico.
	 *
	 * Invoca los métodos de renderizado de las entidades respetando los estados y
	 * prioridades establecidas en la escena.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para emitir comandos de dibujo.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera los recursos del grafo de escena.
	 *
	 * Limpia las listas internas y destruye de manera segura las referencias
	 * administradas antes del cierre del sistema.
	 */
	void
		destroy();
private:
	/**
	 * @brief Propaga las transformaciones del mundo a través de la jerarquía.
	 *
	 * Función recursiva interna que calcula la matriz de mundo absoluta de un nodo
	 * multiplicando su matriz local por la matriz de mundo de su padre.
	 *
	 * @param node        La entidad actual a procesar.
	 * @param parentWorld Matriz de transformación absoluta heredada de la entidad padre.
	 */
	void
		updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

	/**
	 * @brief Verifica si la entidad proporcionada carece de un nodo padre.
	 *
	 * @param e Puntero a la entidad a evaluar.
	 * @return  @c true si la entidad es un nodo raíz independiente.
	 */
	bool
		isRoot(Entity* e) const;

	/**
	 * @brief Comprueba si una entidad ya se encuentra administrada por el grafo.
	 *
	 * @param e Puntero a la entidad.
	 * @return  @c true si la entidad existe dentro de la lista interna del grafo.
	 */
	bool
		isRegistered(Entity* e) const;

private:
	// ============================================================================
	// Estructuras de Almacenamiento Interno
	// ============================================================================
	//std::vector<EU::TSharedPointer<Entity>> m_entities;
public:
	   std::vector<Entity*> m_entities;  ///< Colección plana de todas las entidades registradas en el motor.
};