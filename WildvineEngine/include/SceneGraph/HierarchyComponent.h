#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;
class Entity;

/**
 * @class   HierarchyComponent
 * @brief   Componente que administra las relaciones jerárquicas entre entidades.
 *
 * @details Este componente es la columna vertebral del grafo de escena (Scene Graph)
 * dentro de MonacoEngine3. Permite que una entidad actúe como nodo, estableciendo
 * relaciones de parentesco (padre-hijo) con otras entidades. Esto es fundamental para
 * heredar transformaciones espaciales y propagar estados o comportamientos a
 * través de un árbol de objetos en el mundo del juego.
 */
class
	HierarchyComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa el componente asignándole automáticamente el identificador
	 * de tipo @c ComponentType::HIERARCHY.
	 */
	HierarchyComponent() : Component(ComponentType::HIERARCHY) {}

	/**
	 * @brief Destructor por defecto.
	 */
	~HierarchyComponent() = default;

	/**
	 * @brief Inicializa los recursos internos del componente jerárquico.
	 *
	 * Método heredado de @c Component. Actualmente vacío ya que las relaciones
	 * se establecen dinámicamente mediante el API del SceneGraph.
	 */
	void
		init() override {}

	/**
	 * @brief Actualiza la lógica del componente.
	 *
	 * Método heredado de @c Component.
	 *
	 * @param float Tiempo transcurrido (deltaTime). No utilizado en este componente.
	 */
	void
		update(float) override {}

	/**
	 * @brief Renderiza el componente.
	 *
	 * Método heredado de @c Component. Carece de implementación activa ya que la jerarquía
	 * representa datos estructurales sin una representación visual directa propia.
	 *
	 * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
	 */
	void
		render(DeviceContext& deviceContext) override {}

	/**
	 * @brief Libera los recursos asociados a la jerarquía.
	 *
	 * Limpia de manera segura la lista de entidades hijas y elimina la referencia
	 * a la entidad padre, desvinculando el nodo del grafo de escena.
	 */
	void
		destroy() override {
		m_children.clear();
		m_parent = nullptr;
	}

	// ============================================================================
	// API SceneGraph
	// ============================================================================

	/**
	 * @brief Establece la entidad padre de este nodo.
	 *
	 * @param parent Puntero a la entidad que actuará como nodo superior.
	 */
	void
		setParent(Entity* parent) {
		m_parent = parent;
	}

	/**
	 * @brief Verifica si esta entidad es la raíz de su propia jerarquía.
	 *
	 * @return @c true si la entidad no tiene un padre asignado; @c false en caso contrario.
	 */
	bool
		isRoot() const {
		return m_parent == nullptr;
	}

	/**
	 * @brief Verifica si esta entidad tiene entidades subordinadas.
	 *
	 * @return @c true si la lista de hijos contiene al menos un elemento.
	 */
	bool
		hasChildren() const {
		return !m_children.empty();
	}

	/**
	 * @brief Añade una nueva entidad como hija de este nodo.
	 *
	 * Incorpora un nodo subordinado a la jerarquía, asegurándose previamente de
	 * que el puntero sea válido y de que la entidad no esté ya registrada para
	 * evitar duplicados.
	 *
	 * @param child Puntero a la entidad que se subordinará a este nodo.
	 */
	void
		addChild(Entity* child) {
		if (!child) {
			return;
		}

		if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
			return;
		}
		m_children.push_back(child);
	}

	/**
	 * @brief Elimina una entidad específica de la lista de hijos.
	 *
	 * Busca la entidad subordinada proporcionada y, si existe, la remueve
	 * de la colección de este nodo.
	 *
	 * @param child Puntero a la entidad que se desea remover de la jerarquía.
	 */
	void
		removeChild(Entity* child) {
		if (!child) return;

		m_children.erase(
			std::remove(m_children.begin(), m_children.end(), child),
			m_children.end()
		);
	}

public:
	// ============================================================================
	// Propiedades Estructurales
	// ============================================================================
		Entity* m_parent = nullptr;             ///< Referencia a la entidad padre en el nivel superior del árbol.
		std::vector<Entity*> m_children;        ///< Colección de entidades subordinadas contenidas en este nodo.
};