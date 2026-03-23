#pragma once
#include "Prerequisites.h"

class Window;
class DeviceContext;

/**
 * @class   Viewport
 * @brief   Encapsula una estructura @c D3D11_VIEWPORT para definir el área de dibujo.
 *
 * @details El Viewport es el componente encargado de mapear las coordenadas
 * normalizadas del espacio de recorte (Clip Space) a las coordenadas de píxeles
 * de la ventana o del render target (Screen Space). Esta clase administra las
 * dimensiones, el origen y los rangos de profundidad (MinDepth/MaxDepth) que
 * utiliza el rasterizador para proyectar la escena final.
 */
class
	Viewport {

public:
	/**
	 * @brief Constructor por defecto.
	 * @details Inicializa la clase. Se requiere llamar a @c init() para configurar
	 * las dimensiones reales antes del renderizado.
	 */
	Viewport() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Viewport() = default;

	/**
	 * @brief Inicializa el viewport basándose en las dimensiones de una ventana.
	 *
	 * Extrae el ancho y el alto del área cliente de la @c Window proporcionada
	 * para ajustar automáticamente la región de renderizado.
	 *
	 * @param window Referencia a la ventana de la aplicación que define el lienzo.
	 * @return       Código @c S_OK indicando que la configuración fue exitosa.
	 *
	 * @post El miembro @c m_viewport contendrá los datos de resolución de la ventana.
	 */
	HRESULT
		   init(const Window& window);

	/**
	 * @brief Inicializa el viewport con dimensiones personalizadas.
	 *
	 * Permite definir un área de dibujo específica, útil para técnicas como
	 * pantalla dividida (Split-screen) o renderizado en texturas de tamaño fijo.
	 * Establece por defecto el rango de profundidad de 0.0f a 1.0f.
	 *
	 * @param width  Ancho de la región en píxeles.
	 * @param height Alto de la región en píxeles.
	 * @return       Código @c S_OK indicando que la configuración fue exitosa.
	 */
	HRESULT
		   init(unsigned int width, 
			    unsigned int height);

	/**
	 * @brief Lógica de actualización del viewport.
	 *
	 * Método de marcador arquitectónico para soportar cambios dinámicos o
	 * interpolaciones en la región de visualización.
	 *
	 * @note Actualmente carece de implementación activa.
	 */
	void
		update();

	/**
	 * @brief Vincula el viewport al pipeline gráfico.
	 *
	 * Envía la estructura @c m_viewport al @c DeviceContext mediante la llamada
	 * nativa @c RSSetViewports, estableciendo el área activa para la etapa de rasterización.
	 *
	 * @param deviceContext Contexto del dispositivo responsable de emitir el comando.
	 *
	 * @pre El viewport debe haber sido configurado previamente mediante @c init().
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Limpia los metadatos del viewport.
	 *
	 * Dado que el viewport es una estructura de datos plana y no un recurso COM
	 * de DirectX, este método no realiza liberaciones de memoria de video.
	 */
	void
		destroy() {}

public:
		// ============================================================================
		// Interfaz Nativa
		// ============================================================================
		/**
		 * @brief Estructura nativa de Direct3D 11 que almacena la configuración regional.
		 * @details Contiene TopLeftX, TopLeftY, Width, Height, MinDepth y MaxDepth.
		 */
		D3D11_VIEWPORT m_viewport;
};