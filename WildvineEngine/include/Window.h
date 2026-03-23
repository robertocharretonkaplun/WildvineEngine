#pragma once
#include "Prerequisites.h"

class BaseApp;

/**
 * @class   Window
 * @brief   Encapsula la creación y gestión de una ventana nativa de Microsoft Windows (Win32).
 *
 * @details La clase @c Window actúa como la interfaz entre el motor y el sistema operativo.
 * Se encarga de registrar la clase de ventana, crear el manejador de ventana (@c HWND),
 * definir las dimensiones del área cliente y gestionar el ciclo de vida básico de la
 * interfaz visual del motor. Es el contenedor principal donde el @c SwapChain
 * presentará los frames renderizados.
 */
class
	Window {
public:
	/**
	 * @brief Constructor por defecto.
	 * @details Instancia el objeto sin crear la ventana física. Se requiere llamar a @c init().
	 */
	Window() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Window() = default;

	/**
	 * @brief Inicializa y muestra la ventana en el sistema operativo.
	 *
	 * Registra la estructura @c WNDCLASSEX, define el estilo de la ventana y
	 * crea el @c HWND. También ajusta el rectángulo de la ventana para que el
	 * área cliente coincida con la resolución deseada.
	 *
	 * @param hInstance Identificador de la instancia de la aplicación (proporcionado por el SO).
	 * @param nCmdShow  Estado de visualización inicial (minimizado, maximizado, normal).
	 * @param wndproc   Puntero a la función de procedimiento de ventana (Callback) para manejar mensajes.
	 * @param app       Puntero a la aplicación base para vinculación de contexto.
	 * @return          Código @c HRESULT indicando el éxito (@c S_OK) o fallo en la creación.
	 */
	HRESULT
		   init(HINSTANCE hInstance, 
			    int nCmdShow, 
			    WNDPROC wndproc, 
			    BaseApp* app);

	/**
	 * @brief Lógica de actualización de la ventana.
	 *
	 * Método de marcador arquitectónico. Puede utilizarse para procesar eventos
	 * específicos de la ventana o actualizar metadatos de su estado.
	 */
	void
		update();

	/**
	 * @brief Fase de renderizado de la ventana.
	 *
	 * Método de marcador arquitectónico. Generalmente la lógica de renderizado
	 * se delega a la clase @c BaseApp y al @c SwapChain.
	 */
	void
		render();

	/**
	 * @brief Destruye la ventana nativa y libera recursos del sistema.
	 *
	 * Llama a @c DestroyWindow y limpia los manejadores de instancia y de ventana
	 * restableciéndolos a @c nullptr.
	 */
	void
		destroy();

public:
		// ============================================================================
		// Interfaz Nativa y Dimensiones
		// ============================================================================
		/** @brief Manejador nativo de la ventana de Windows (Win32 HWND). */
		HWND m_hWnd = nullptr;

		/** @brief Ancho actual del área cliente de la ventana en píxeles. */
		unsigned int m_width;

		/** @brief Alto actual del área cliente de la ventana en píxeles. */
		unsigned int m_height;

private:
		// ============================================================================
		// Propiedades Internas
		// ============================================================================
		/** @brief Instancia de la aplicación vinculada a esta ventana. */
		HINSTANCE m_hInst = nullptr;

		/** @brief Estructura que define las coordenadas y límites de la ventana. */
		RECT m_rect;

		/** @brief Título o nombre identificador de la ventana en la barra de tareas. */
		std::string m_windowName = "Wildvine Engine";
};