#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @class   GUI
 * @brief   Sistema central de interfaz de usuario para el editor y depuración.
 *
 * @details La clase @c GUI encapsula la integración de la biblioteca Dear ImGui
 * y sus extensiones (como ImGuizmo) dentro del motor MonacoEngine3. Se encarga
 * de gestionar el ciclo de vida de los contextos de UI, el enrutamiento de
 * eventos de entrada (Win32) hacia la interfaz, y la construcción de los
 * diferentes paneles del editor (Inspector, Outliner, Viewport y barras de herramientas).
 */
class
    GUI {

public:
        /**
         * @brief Constructor por defecto.
         */
        GUI() = default;

        /**
         * @brief Destructor por defecto.
         */
        ~GUI() = default;

        // ============================================================================
        // Ciclo de Vida
        // ============================================================================

        /**
         * @brief Preparación previa a la inicialización gráfica.
         *
         * Configura estados internos o banderas necesarias antes de levantar
         * el contexto principal de ImGui.
         */
        void awake();

        /**
         * @brief Inicializa los contextos de ImGui y sus implementaciones (backends).
         *
         * Crea el contexto global de ImGui, configura las opciones de acoplamiento
         * (Docking) y las ventanas múltiples (Multi-Viewport), e inicializa los
         * backends de Win32 y DirectX 11.
         *
         * @param window        Ventana del sistema operativo para capturar eventos (Win32).
         * @param device        Dispositivo gráfico para la creación de fuentes y buffers de UI.
         * @param deviceContext Contexto para la emisión de comandos de dibujo de UI.
         */

    void init(Window& window, 
              Device& device, 
              DeviceContext& deviceContext);

    /**
     * @brief Inicia un nuevo frame de la interfaz de usuario.
     *
     * Llama a las funciones de actualización de los backends y de ImGui para
     * preparar el contexto para recibir nuevos comandos de dibujo (widgets).
     *
     * @param viewport Referencia al viewport principal de la aplicación.
     * @param window   Referencia a la ventana del sistema.
     */
    void update(Viewport& viewport, 
                Window& window);

    /**
     * @brief Finaliza el frame de UI y envía los vértices a la GPU.
     *
     * Compila todas las llamadas a paneles y widgets en buffers de vértices
     * y los dibuja sobre la pantalla usando el contexto de DirectX 11.
     */
    void render();

    /**
     * @brief Apaga y libera los recursos de ImGui.
     *
     * Desconecta los backends de Win32 y DirectX 11, y destruye el contexto global.
     */
    void destroy();

    // ============================================================================
    // Acciones y Popups
    // ============================================================================

    /**
     * @brief Renderiza la barra de herramientas principal del editor.
     */
    void ToolBar();

    /**
     * @brief Maneja la lógica y ventana modal para el cierre seguro de la aplicación.
     */
    void closeApp();

    /**
     * @brief Administra y muestra los tooltips (ventanas emergentes de ayuda)
     * al pasar el ratón sobre ciertos elementos.
     */
    void toolTipData();

    // ============================================================================
    // Estilos Visuales
    // ============================================================================

    /**
     * @brief Aplica un tema visual personalizado a la interfaz (Estilo macOS / Glass).
     *
     * Modifica los colores globales, bordes, redondeos y transparencias del `ImGuiStyle`.
     *
     * @param opacity Nivel de opacidad base para los paneles.
     * @param accent  Color de acento principal (ej. para botones presionados o selecciones).
     */
    void appleLiquidStyle(float opacity,
                          ImVec4 accent);

    // ============================================================================
    // Controles de Usuario (Widgets Personalizados)
    // ============================================================================

    /**
     * @brief Crea un control estilizado para la edición de vectores tridimensionales (XYZ).
     *
     * Utilizado comúnmente en el Inspector para modificar transformaciones (Posición, Rotación, Escala).
     *
     * @param label       Etiqueta descriptiva del control.
     * @param values      Puntero a un arreglo de 3 flotantes que se modificarán.
     * @param resetValues Valor por defecto al que regresan los ejes si se hace clic en su botón de reset.
     * @param columnWidth Ancho reservado para la etiqueta antes de mostrar las cajas de texto.
     */
    void
        vec3Control(const std::string& label,
                    float* values,
                    float resetValues = 0.0f,
                    float columnWidth = 100.0f);

    // ============================================================================
    // Paneles del Editor
    // ============================================================================

    /**
     * @brief Dibuja la sección principal de propiedades para un Actor seleccionado.
     *
     * Muestra el nombre, botones rápidos y metadatos del objeto en foco.
     *
     * @param actor Puntero al actor inspeccionado.
     */
    void
        inspectorGeneral(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja los componentes y propiedades específicas de un Actor seleccionado.
     *
     * Recorre los componentes (Transform, Mesh, Material) y renderiza sus respectivos
     * controles editables.
     *
     * @param actor Puntero al actor inspeccionado.
     */
    void
        inspectorContainer(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Renderiza el árbol de jerarquía (Outliner) de la escena actual.
     *
     * Lista todos los actores de la escena y permite seleccionarlos haciendo clic
     * sobre sus nombres.
     *
     * @param actors Colección de actores activos a mostrar en la lista.
     */
    void
        outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    /**
     * @brief Renderiza la cinta de opciones principal superior (Ribbon/Menú).
     */
    void
        drawStudioTopRibbon();

    /**
     * @brief Dibuja la ventana central que contiene el renderizado del juego (Viewport).
     *
     * @param viewportSRV Vista del recurso del shader (@c ID3D11ShaderResourceView)
     * que contiene la imagen renderizada por el pase off-screen.
     */
    void
        drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

    /**
     * @brief Configura la estructura base de acoplamiento (Dockspace).
     *
     * Prepara la ventana raíz invisible que permite que el resto de los paneles
     * (Outliner, Inspector, Viewport) se acoplen, redimensionen y organicen.
     */
    void
        drawEditorDockspace();

    // ============================================================================
    // Transformación y Gizmos
    // ============================================================================

    /**
     * @brief Controla y dibuja los manipuladores 3D (Gizmos) sobre el viewport.
     *
     * Utiliza la biblioteca @c ImGuizmo para mostrar flechas y aros interactivos
     * que permiten mover, rotar y escalar actores directamente en la vista de escena.
     *
     * @param cam    Cámara desde la cual se está viendo la escena.
     * @param window Ventana principal (para capturar atajos de teclado y eventos).
     * @param actor  Actor actualmente seleccionado que será transformado.
     */
    void
        editTransform(Camera& cam, 
                      Window& window,
                      EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja la barra de herramientas para seleccionar el modo de Gizmo.
     *
     * Permite alternar entre los modos Traslación, Rotación y Escala, o
     * coordenadas Locales y Globales.
     */
    void
        drawGizmoToolbar();

    /**
     * @brief Convierte una matriz nativa DirectX (@c XMMATRIX) a un arreglo lineal de 16 flotantes.
     *
     * Función auxiliar necesaria para alimentar las matrices matemáticas de DirectX
     * hacia el formato esperado por la librería @c ImGuizmo.
     *
     * @param mat  Matriz DirectX a convertir.
     * @param dest Puntero a un arreglo de destino de tamaño igual o mayor a 16 flotantes.
     */
    void
        ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

private:
        // ============================================================================
        // Datos Internos y Estados
        // ============================================================================
        bool checkboxValue = true;                          ///< Bandera auxiliar de UI.
        bool checkboxValue2 = false;                        ///< Bandera auxiliar de UI.
        std::vector<const char*> m_objectsNames;            ///< Lista temporal de nombres para combos o listas.
        std::vector<const char*> m_tooltips;                ///< Almacenamiento de textos de ayuda flotante.

        bool show_exit_popup = false;                       ///< Controla la visibilidad del modal de salida.
        ImDrawList* m_viewportDrawList = nullptr;           ///< Puntero a la lista de dibujo específica de la ventana Viewport.
        bool m_viewportActive = false;                      ///< Indica si la ventana del Viewport está en uso.

public:
        // ============================================================================
        // Interacción Global
        // ============================================================================
        bool m_isUsingGizmo = false;                        ///< @c true si el usuario está arrastrando activamente un Gizmo 3D.
        int selectedActorIndex = -1;                        ///< Índice en el SceneGraph del actor seleccionado (o -1 si no hay selección).
        ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);          ///< Coordenadas absolutas en pantalla de la esquina superior izquierda del Viewport.
        ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);         ///< Dimensiones actuales del área de dibujo dentro de la ventana del Viewport.
        bool m_viewportHovered = false;                     ///< @c true si el cursor del ratón está sobre la ventana del Viewport.
        bool m_viewportFocused = false;                     ///< @c true si la ventana del Viewport tiene el foco del teclado y ratón.
};