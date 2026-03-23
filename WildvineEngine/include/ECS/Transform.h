#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class   Transform
 * @brief   Componente que define la posición, rotación y escala de una entidad.
 *
 * @details La clase @c Transform encapsula las propiedades espaciales de una
 * entidad dentro del mundo 3D. Se encarga de calcular y mantener actualizada la
 * matriz de transformación mundial componiendo su escala, rotación y traslación
 * en cada ciclo del motor. Es un componente derivado de @c Component.
 */
class
    Transform : public Component {
public:
    /**
     * @brief Constructor por defecto.
     *
     * Inicializa la posición, rotación, escala y la matriz con sus constructores
     * predeterminados y asigna el tipo de componente correspondiente.
     */
    Transform() : position(),
                  rotation(),
                  scale(),
                  matrix(),
                  Component(ComponentType::TRANSFORM) {
    }

    /**
     * @brief Inicializa los valores base del componente Transform.
     *
     * Establece la escala inicial a la unidad (1, 1, 1) y define la matriz de
     * transformación como la matriz identidad.
     */
    void
        init() {
            scale.one();
            matrix = XMMatrixIdentity();
    }

    /**
     * @brief Actualiza la matriz de transformación del componente.
     *
     * Calcula la matriz resultante componiendo las transformaciones individuales
     * en el orden estándar para gráficos 3D: Escala -> Rotación -> Traslación.
     *
     * @param deltaTime Tiempo transcurrido en segundos desde la última actualización.
     */
    void
        update(float deltaTime) override {
        // Aplicar escala
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
        // Aplicar rotacion
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        // Aplicar traslacion
        XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

        // Componer la matriz final en el orden: scale -> rotation -> translation
        matrix = scaleMatrix * rotationMatrix * translationMatrix;
    }

    /**
     * @brief Renderiza el componente Transform.
     *
     * Este método carece de implementación activa ya que un Transform representa
     * datos matemáticos y espaciales sin una representación visual directa propia.
     *
     * @param deviceContext Contexto del dispositivo utilizado para operaciones gráficas.
     */
    void
        render(DeviceContext& deviceContext) override {}

    /**
     * @brief Libera los recursos del componente Transform.
     *
     * Al no manejar punteros dinámicos crudos o recursos gráficos, la
     * implementación está vacía.
     */
    void
        destroy() {}

    /**
     * @brief Obtiene la posición espacial actual.
     * @return Referencia constante al vector @c EU::Vector3 de posición.
     */
    const EU::Vector3&
              getPosition() const { return position; }

    /**
     * @brief Establece una nueva posición espacial.
     * @param newPos Vector 3D con las nuevas coordenadas (x, y, z).
     */
    void
        setPosition(const EU::Vector3& newPos) { position = newPos; }

    /**
     * @brief Obtiene la rotación actual de la entidad.
     * @return Referencia constante al vector @c EU::Vector3 de rotación.
     */
    const EU::Vector3&
              getRotation() const { return rotation; }

    /**
     * @brief Establece una nueva rotación espacial.
     * @param newRot Vector 3D con los nuevos ángulos de rotación (Pitch, Yaw, Roll).
     */
    void
        setRotation(const EU::Vector3& newRot) { rotation = newRot; }

    /**
     * @brief Obtiene la escala tridimensional actual.
     * @return Referencia constante al vector @c EU::Vector3 de escala.
     */
    const EU::Vector3&
              getScale() const { return scale; }

    /**
     * @brief Establece una nueva escala tridimensional.
     * @param newScale Vector 3D con los nuevos factores de multiplicación de tamaño.
     */
    void
        setScale(const EU::Vector3& newScale) { scale = newScale; }

    /**
     * @brief Define simultáneamente la posición, rotación y escala de la entidad.
     *
     * @param newPos Vector 3D con la nueva posición.
     * @param newRot Vector 3D con la nueva rotación.
     * @param newSca Vector 3D con la nueva escala.
     */
    void
        setTransform(const EU::Vector3& newPos,
                const EU::Vector3& newRot,
                const EU::Vector3& newSca) {
                position = newPos;
                rotation = newRot;
                scale = newSca;
    }

    /**
     * @brief Desplaza la posición actual sumando un vector de traslación.
     *
     * @param translation Vector 3D que indica la dirección y magnitud del movimiento.
     */
    void
        translate(const EU::Vector3& translation);

private:
    // ============================================================================
    // Propiedades Espaciales
    // ============================================================================
    EU::Vector3 position;  ///< Coordenadas actuales en el espacio 3D.
    EU::Vector3 rotation;  ///< Valores de rotación aplicados en los ejes correspondientes.
    EU::Vector3 scale;     ///< Modificadores de tamaño a lo largo de los ejes X, Y y Z.

public:
    // ============================================================================
    // Matrices de Cálculo
    // ============================================================================
    XMMATRIX matrix;    ///< Matriz de transformación mundial (World Matrix) precalculada.
};