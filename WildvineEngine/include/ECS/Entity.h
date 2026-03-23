#pragma once
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

/**
 * @class   Entity
 * @brief   Clase base abstracta para todos los objetos vivos dentro de la escena.
 *
 * La clase Entity actúa como un contenedor fundamental en la arquitectura
 * del motor. Define el ciclo de vida básico (inicialización, actualización,
 * renderizado y destrucción) y permite adherir múltiples componentes (@c Component)
 * que definen el comportamiento y las características específicas del objeto.
 */
class
    Entity {
public:
    /**
     * @brief Constructor por defecto.
     */
    Entity() = default;

    /**
     * @brief Destructor virtual por defecto.
     *
     * Garantiza la correcta destrucción y limpieza de los objetos derivados.
     */
    virtual
        ~Entity() = default;

    /**
     * @brief Inicializa la entidad justo antes de su primer ciclo de actualización.
     *
     * Método virtual puro. Se utiliza para configurar estados iniciales o dependencias
     * una vez que la entidad ha sido instanciada en la escena.
     */
    virtual void
        awake() = 0;

    /**
     * @brief Inicializa los recursos internos de la entidad.
     *
     * Método virtual puro. Debe invocarse para preparar la memoria, componentes o
     * configuraciones necesarias antes de que la entidad comience a operar.
     */
    virtual void
        init() = 0;

    /**
     * @brief Actualiza la lógica de comportamiento de la entidad.
     *
     * Método virtual puro invocado una vez por frame. Se encarga de procesar la lógica
     * temporal de la entidad y de propagar la actualización a sus componentes.
     *
     * @param deltaTime     Tiempo transcurrido en segundos desde el último frame.
     * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
     */
    virtual void
        update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Envía la representación visual de la entidad al pipeline gráfico.
     *
     * Método virtual puro. Propaga el llamado de renderizado a aquellos componentes
     * que posean una representación visual.
     *
     * @param deviceContext Contexto del dispositivo utilizado para emitir comandos de dibujo.
     */
    virtual void
        render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Libera y limpia de manera segura los recursos de la entidad.
     *
     * Método virtual puro. Destruye los componentes internos y libera la memoria
     * asociada antes de que la entidad sea eliminada por completo.
     */
    virtual void
        destroy() = 0;

    /**
     * @brief Agrega un nuevo componente a la entidad.
     *
     * @tparam T            El tipo del componente a añadir. Debe ser una clase derivada de @c Component.
     * @param component     Puntero inteligente compartido que encapsula el componente a agregar.
     */
    template <typename T> void
        addComponent(EU::TSharedPointer<T> component) {
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Recupera un componente específico adjunto a la entidad según su tipo.
     *
     * @tparam T            El tipo del componente que se desea recuperar.
     * @return              Un puntero inteligente compartido al componente si se encuentra;
     * de lo contrario, devuelve un puntero nulo.
     */
    template<typename T>
    EU::TSharedPointer<T>
        getComponent() {
        for (auto& component : m_components) {
            EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
            if (specificComponent) {
                return specificComponent;
            }
        }
        return EU::TSharedPointer<T>();
    }
private:
protected:
    // ============================================================================
    // Propiedades Generales e Identificación
    // ============================================================================
    bool m_isActive; ///< Bandera que indica si la entidad está activa y procesándose en la escena.
    int  m_id;       ///< Identificador numérico único de la entidad en el motor.

    // ============================================================================
    // Arquitectura de Componentes
    // ============================================================================
    std::vector<EU::TSharedPointer<Component>> m_components; ///< Colección de componentes adjuntos a esta entidad.
};