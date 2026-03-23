#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

class Device;
class DeviceContext;

/**
 * @class   EditorViewportPass
 * @brief   Administra un pase de renderizado fuera de pantalla (off-screen) para el editor.
 *
 * @details Esta clase encapsula la creación y gestión de un Render Target independiente
 * con su propio buffer de color y de profundidad (Depth/Stencil). Es fundamental para
 * renderizar la escena del juego dentro de una ventana del editor (por ejemplo,
 * integrada con ImGui), separando el renderizado del mundo 3D del backbuffer
 * principal de la aplicación.
 */
class
	EditorViewportPass {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	EditorViewportPass() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~EditorViewportPass() = default;

	/**
	 * @brief Inicializa los recursos del pase de renderizado.
	 *
	 * Llama internamente a la creación de texturas, Render Target Views (RTV)
	 * y Depth Stencil Views (DSV) con las dimensiones especificadas.
	 *
	 * @param device Dispositivo gráfico utilizado para la creación de recursos.
	 * @param width  Ancho inicial del viewport en píxeles.
	 * @param height Alto inicial del viewport en píxeles.
	 * @return       Código @c HRESULT indicando el éxito o la razón del fallo en la creación.
	 */
	HRESULT init(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Redimensiona los buffers del viewport.
	 *
	 * Destruye los recursos actuales y los vuelve a crear si las dimensiones
	 * de la ventana del editor han cambiado, asegurando que la resolución coincida.
	 *
	 * @param device Dispositivo gráfico utilizado para la recreación de recursos.
	 * @param width  Nuevo ancho del viewport en píxeles.
	 * @param height Nuevo alto del viewport en píxeles.
	 * @return       Código @c HRESULT indicando el éxito de la operación.
	 */
	HRESULT resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Prepara el pase para comenzar a dibujar.
	 *
	 * Vincula el Render Target y el Depth Stencil al pipeline gráfico y
	 * limpia los buffers con el color de fondo especificado.
	 *
	 * @param deviceContext Contexto del dispositivo para emitir comandos.
	 * @param clearColor    Arreglo de 4 flotantes (RGBA) que define el color de limpieza.
	 */
	void begin(DeviceContext& deviceContext, const float clearColor[4]);

	/**
	 * @brief Intercambia los recursos internos con otro pase de renderizado.
	 *
	 * Útil para técnicas de ping-pong rendering o post-procesamiento donde
	 * se necesita alternar entre buffers de lectura y escritura.
	 *
	 * @param other Referencia al otro @c EditorViewportPass con el que se intercambiarán datos.
	 */
	void swap(EditorViewportPass& other);

	/**
	 * @brief Limpia exclusivamente el buffer de profundidad y esténcil.
	 *
	 * @param deviceContext Contexto del dispositivo para emitir comandos de limpieza.
	 */
	void clearDepth(DeviceContext& deviceContext);

	/**
	 * @brief Configura el área de dibujo en el pipeline (D3D11 Viewport).
	 *
	 * Mapea las coordenadas normalizadas del dispositivo a las dimensiones
	 * en píxeles de este pase de renderizado.
	 *
	 * @param deviceContext Contexto del dispositivo para aplicar el viewport.
	 */
	void setViewport(DeviceContext& deviceContext);

	/**
	 * @brief Libera todos los recursos gráficos asociados (Texturas, RTV, DSV, SRV).
	 */
	void destroy();

	/**
	 * @brief Obtiene el recurso de vista de shader (SRV) del buffer de color.
	 *
	 * **Aplicación práctica**
	 * - Se envía a ImGui (o al sistema de UI) para dibujar este pase como una textura en pantalla.
	 *
	 * @return Puntero nativo al @c ID3D11ShaderResourceView.
	 */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el ancho actual del pase de renderizado.
	 * @return Ancho en píxeles.
	 */
	unsigned int getWidth() const { return m_width; }

	/**
	 * @brief Obtiene el alto actual del pase de renderizado.
	 * @return Alto en píxeles.
	 */
	unsigned int getHeight() const { return m_height; }

	/**
	 * @brief Verifica la integridad de los recursos creados.
	 * @return @c true si los buffers de color, profundidad y las vistas existen; @c false en caso contrario.
	 */
	bool isValid() const
	{
		return m_colorTexture.m_texture != nullptr &&
			   m_colorSRV.m_textureFromImg != nullptr &&
			   m_depthTexture.m_texture != nullptr;
	}

private:
		/**
		 * @brief Función interna para la generación en memoria de video de las texturas y vistas.
		 *
		 * @param device Dispositivo gráfico.
		 * @param width  Ancho solicitado.
		 * @param height Alto solicitado.
		 * @return       @c HRESULT con el resultado de las operaciones en DirectX.
		 */
		HRESULT createResources(Device& device, unsigned int width, unsigned int height);

private:
		// ============================================================================
		// Recursos de Color
		// ============================================================================
		Texture           m_colorTexture; ///< Textura 2D que actúa como destino de los píxeles (Backbuffer local).
		Texture           m_colorSRV;     ///< Vista de Recurso de Shader (SRV) para leer el resultado como textura.
		RenderTargetView  m_rtv;          ///< Vista de Render Target (RTV) para escribir en el buffer de color.

		// ============================================================================
		// Recursos de Profundidad
		// ============================================================================
		Texture           m_depthTexture; ///< Textura 2D para almacenar los valores de profundidad de la escena.
		DepthStencilView  m_dsv;          ///< Vista de Depth/Stencil para las pruebas de profundidad.

		// ============================================================================
		// Dimensiones
		// ============================================================================
		unsigned int      m_width = 1;    ///< Ancho en píxeles del pase de renderizado.
		unsigned int      m_height = 1;   ///< Alto en píxeles del pase de renderizado.
};