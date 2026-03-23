#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

/**
 * @class   Camera
 * @brief   Representa una cámara virtual en el espacio tridimensional.
 *
 * La clase Camera gestiona la posición, orientación y parámetros de
 * proyección necesarios para renderizar una escena 3D. Calcula y mantiene actualizadas
 * las matrices de Vista (View) y Proyección (Projection) utilizando un sistema
 * de coordenadas Left-Handed (LH), fundamental para el pipeline gráfico del motor.
 * Además, provee utilidades para el movimiento relativo (FPS) y la orientación.
 */
class
	Camera {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa la cámara en el origen con los vectores base predeterminados
	 * y una matriz de identidad implícita.
	 */
	Camera();

	/**
	 * @brief Destructor por defecto.
	 */
	~Camera() = default;

	/**
	 * @brief Configura la matriz de proyección en perspectiva (Left-Handed).
	 *
	 * Calcula la matriz de proyección basándose en el campo de visión y los planos
	 * de recorte. Esta matriz transforma las coordenadas de la vista al espacio de recorte (Clip Space).
	 *
	 * @param fovYRadians Ángulo del campo de visión vertical en radianes.
	 * @param aspectRatio Relación de aspecto de la pantalla (ancho / alto).
	 * @param nearPlane   Distancia al plano de recorte cercano (Near Z).
	 * @param farPlane    Distancia al plano de recorte lejano (Far Z).
	 *
	 * @note Debe llamarse al inicializar la ventana y cada vez que esta cambie de resolución.
	 */
	void
		setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);

	/**
	 * @brief Establece la posición absoluta de la cámara en el mundo.
	 *
	 * @param x Coordenada X en el espacio global.
	 * @param y Coordenada Y en el espacio global.
	 * @param z Coordenada Z en el espacio global.
	 */
	void
		setPosition(float x, float y, float z);

	/**
	 * @brief Establece la posición absoluta de la cámara en el mundo.
	 *
	 * @param pos Vector tridimensional con las nuevas coordenadas globales.
	 */
	void
		setPosition(const EU::Vector3& pos);

	/**
	 * @brief Obtiene una copia de la posición absoluta actual.
	 * @return Un vector @c EU::Vector3 con las coordenadas de la cámara.
	 */
	EU::Vector3
		getPosition() const { return m_position; }

	/**
	 * @brief Obtiene una referencia a la posición absoluta actual.
	 * @return Referencia modificable al vector @c EU::Vector3 de posición.
	 */
	EU::Vector3&
		getPosition() { return m_position; }

	/**
	 * @brief Orienta la cámara para que mire directamente hacia un punto objetivo.
	 *
	 * Reconstruye los vectores base (Right, Up, Forward) calculando la dirección
	 * hacia el objetivo y normalizando el resultado. Marca la matriz de vista
	 * como "sucia" para forzar su recálculo.
	 *
	 * @param pos    Posición actual o nueva posición de la cámara en el mundo.
	 * @param target Coordenada tridimensional en el mundo hacia la cual mirar.
	 * @param up     Vector que define la dirección "arriba" global (por defecto Y+).
	 */
	void
		lookAt(const EU::Vector3& pos,
			   const EU::Vector3& target,
			   const EU::Vector3& up = EU::Vector3(0, 1, 0));

	/**
	 * @brief Desplaza la cámara a lo largo de su vector frontal (Forward).
	 *
	 * @param d Distancia a recorrer. Valores positivos mueven hacia adelante; negativos hacia atrás.
	 */
	void
		walk(float d);

	/**
	 * @brief Desplaza la cámara a lo largo de su vector lateral (Right).
	 *
	 * @param d Distancia a recorrer. Valores positivos mueven a la derecha; negativos a la izquierda.
	 */
	void
		strafe(float d);

	/**
	 * @brief Rota la cámara sobre el eje Y global (Yaw o Guiñada).
	 *
	 * Ideal para controles estilo First Person Shooter (FPS) donde la rotación horizontal
	 * no se ve afectada por el balanceo de la cámara.
	 *
	 * @param radians Ángulo de rotación en radianes.
	 */
	void
		yaw(float radians);

	/**
	 * @brief Rota la cámara sobre su propio eje lateral (Pitch o Cabeceo).
	 *
	 * Modifica la elevación de la vista hacia arriba o hacia abajo.
	 *
	 * @param radians Ángulo de rotación en radianes.
	 */
	void
		pitch(float radians);

	/**
	 * @brief Recalcula la matriz de Vista (View Matrix) si se han producido cambios.
	 *
	 * Si la cámara se ha movido o rotado (marcada como sucia), reconstruye el
	 * sistema base ortonormal y genera la nueva matriz utilizando `XMMatrixLookToLH`.
	 * Debe llamarse una vez por frame antes del renderizado.
	 */
	void
		updateViewMatrix();

	/**
	 * @brief Obtiene la matriz de Vista actual.
	 *
	 * Transforma vértices desde el espacio del mundo (World Space) al espacio de la vista (View Space).
	 *
	 * @return La matriz @c XMMATRIX de vista.
	 */
	XMMATRIX
		getView() const { return XMLoadFloat4x4(&m_view); }

	/**
	 * @brief Obtiene la matriz de Proyección actual.
	 *
	 * Transforma vértices desde el espacio de la vista (View Space) al espacio de recorte (Clip Space).
	 *
	 * @return La matriz @c XMMATRIX de proyección.
	 */
	XMMATRIX
		getProj() const { return XMLoadFloat4x4(&m_proj); }

	/**
	 * @brief Obtiene una matriz de Vista omitiendo los datos de traslación.
	 *
	 * Útil para renderizar elementos proyectados al infinito que solo deben
	 * responder a la rotación de la cámara, como un Skybox.
	 *
	 * @return La matriz @c XMMATRIX de vista modificada (fila de traslación en 0).
	 */
	XMMATRIX
		GetViewNoTranslation() const {
		XMMATRIX v = getView();
		// Quitar traslación (fila 4)
		v.r[3] = XMVectorSet(0, 0, 0, 1);
		return v;
	}

	// ============================================================================
	// Métodos de Acceso a Parámetros y Vectores Base
	// ============================================================================

	/** @brief Devuelve el ángulo del campo de visión vertical (FOV) en radianes. */
	float getFovY()   const { return m_fovY; }
	/** @brief Devuelve la relación de aspecto actual (Ancho / Alto). */
	float getAspect() const { return m_aspectRatio; }
	/** @brief Devuelve la distancia al plano de recorte cercano. */
	float getNearZ()  const { return m_nearPlane; }
	/** @brief Devuelve la distancia al plano de recorte lejano. */
	float getFarZ()   const { return m_farPlane; }

	/** @brief Obtiene el vector lateral normalizado de la cámara en el espacio global. */
	EU::Vector3 GetRight()   const { return m_right; }
	/** @brief Obtiene el vector superior normalizado de la cámara en el espacio global. */
	EU::Vector3 GetUp()      const { return m_up; }
	/** @brief Obtiene el vector frontal normalizado de la cámara en el espacio global. */
	EU::Vector3 GetForward() const { return m_forward; }

	/**
	 * @brief Función auxiliar para convertir un vector SIMD a un vector del motor.
	 *
	 * @param v Vector de DirectX Math (@c FXMVECTOR).
	 * @return El vector equivalente en formato @c EU::Vector3.
	 */
	inline EU::Vector3
		FromXM(FXMVECTOR v) {
		XMFLOAT3 t;
		XMStoreFloat3(&t, v);
		return EU::Vector3(t.x, t.y, t.z);
	}


private:
		// ============================================================================
		// Estado Espacial y Orientación
		// ============================================================================
		EU::Vector3 m_position; /**< Coordenadas actuales de la cámara en el mundo. */

		EU::Vector3 m_right{ 1.0f, 0.0f, 0.0f }; /**< Eje X local (Derecha) de la base ortonormal. */
		EU::Vector3 m_up{ 0.0f, 1.0f, 0.0f }; /**< Eje Y local (Arriba) de la base ortonormal. */
		EU::Vector3 m_forward{ 0.0f, 0.0f, 1.0f }; /**< Eje Z local (Adelante) de la base ortonormal. */

		// ============================================================================
		// Almacenamiento de Matrices
		// ============================================================================
		XMFLOAT4X4 m_view{}; /**< Matriz de Vista precalculada. */
		XMFLOAT4X4 m_proj{}; /**< Matriz de Proyección en perspectiva. */

		// ============================================================================
		// Parámetros de la Lente / Proyección
		// ============================================================================
		float m_fovY{ XM_PIDIV4 }; /**< Ángulo vertical del campo de visión en radianes. */
		float m_aspectRatio = 1.0f; /**< Proporción entre el ancho y el alto de la pantalla. */
		float m_nearPlane = 0.01f;  /**< Distancia mínima de renderizado. */
		float m_farPlane = 1000.0f; /**< Distancia máxima de renderizado. */

		bool m_viewDirty = true;    /**< Bandera que indica si la matriz de vista requiere ser recalculada. */
};