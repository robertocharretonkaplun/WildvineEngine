#pragma once
#include "Prerequisites.h"

// Forward Declarations
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @class   RenderTargetView
 * @brief   Encapsula la creación y gestión de un @c ID3D11RenderTargetView.
 *
 * Un Render Target View (RTV) es una interfaz de DirectX 11 que permite
 * al pipeline gráfico (específicamente a la etapa Output-Merger) escribir color
 * en un recurso de memoria bidimensional. Esta clase administra su ciclo de vida
 * y facilita su enlace al contexto de renderizado. Puede asociarse tanto al
 * backbuffer principal de la ventana como a texturas secundarias para técnicas
 * avanzadas (ej. G-Buffers, post-procesamiento o viewports del editor).
 */
class
    RenderTargetView {

public:
    /**
     * @brief Constructor por defecto.
     * @details Instancia la clase sin inicializar recursos en la GPU. Se debe invocar a @c init().
     */
    RenderTargetView() = default;

    /**
     * @brief Destructor por defecto.
     * @warning Al administrar un recurso COM nativo, no libera automáticamente la memoria
     * de video asociada. Se requiere llamar explícitamente a @c destroy().
     */
    ~RenderTargetView() = default;

    // ============================================================================
    // Inicialización y Creación
    // ============================================================================

    /**
     * @brief Inicializa la vista utilizando el Back Buffer principal.
     *
     * @param device     Dispositivo gráfico responsable de la creación del recurso.
     * @param backBuffer Textura extraída directamente de la SwapChain que representa el lienzo final.
     * @param Format     Formato de color del RTV (ej. @c DXGI_FORMAT_R8G8B8A8_UNORM).
     * @return           Código @c HRESULT nativo indicando el éxito (@c S_OK) o fallo.
     *
     * @post Si retorna @c S_OK, el puntero interno @c m_renderTargetView será válido.
     */
    HRESULT
           init(Device& device, 
                Texture& backBuffer, 
                DXGI_FORMAT Format);

    /**
     * @brief Inicializa la vista utilizando una textura genérica (Off-screen).
     *
     * Permite renderizar la escena en una textura independiente en lugar de la pantalla.
     * Es fundamental para la creación de mapas de sombras, reflejos o la integración del editor.
     *
     * @param device        Dispositivo gráfico responsable de la creación.
     * @param inTex         Textura base que recibirá los píxeles (debe haber sido creada con banderas RTV).
     * @param ViewDimension Tipología de la vista en DirectX (ej. @c D3D11_RTV_DIMENSION_TEXTURE2D).
     * @param Format        Formato de color esperado para los datos escritos.
     * @return              Código @c HRESULT nativo indicando el estado de la operación.
     */
    HRESULT
           init(Device& device,
                Texture& inTex,
                D3D11_RTV_DIMENSION ViewDimension,
                DXGI_FORMAT Format);

    /**
     * @brief Lógica de actualización del recurso.
     *
     * Método de marcador arquitectónico para posibles redimensionamientos dinámicos
     * o cambios de estado.
     *
     * @note Actualmente carece de implementación activa.
     */
    void
        update();

    /**
     * @brief Limpia la vista y la vincula al pipeline junto con un Depth Stencil.
     *
     * Combina dos operaciones frecuentes al inicio de un frame: invoca @c ClearRenderTargetView
     * para rellenar el lienzo con un color sólido, y utiliza @c OMSetRenderTargets para
     * establecer este RTV y el DSV proporcionado como los destinos activos.
     *
     * @param deviceContext    Contexto del dispositivo para emitir los comandos gráficos.
     * @param depthStencilView Vista de profundidad y esténcil para las pruebas de visibilidad espacial.
     * @param numViews         Cantidad de Render Targets a vincular (típicamente 1).
     * @param ClearColor       Arreglo de 4 flotantes (RGBA) con el color de limpieza del fondo.
     *
     * @pre El @c m_renderTargetView debe haber sido instanciado correctamente con @c init().
     */
    void
        render(DeviceContext& deviceContext,
               DepthStencilView& depthStencilView,
               unsigned int numViews,
               const float ClearColor[4]);

    /**
     * @brief Vincula la vista al pipeline de forma aislada (Sin limpieza ni profundidad).
     *
     * Llama a @c OMSetRenderTargets exclusivamente con este RTV, desvinculando cualquier
     * buffer de profundidad previo. Útil para pasadas de renderizado puramente 2D (como la UI).
     *
     * @param deviceContext Contexto del dispositivo para emitir el comando.
     * @param numViews      Cantidad de Render Targets a vincular (típicamente 1).
     *
     * @pre El @c m_renderTargetView debe haber sido instanciado correctamente con @c init().
     */
    void
        render(DeviceContext& deviceContext,
               unsigned int numViews);

    // ============================================================================
    // Liberación de Recursos
    // ============================================================================

    /**
     * @brief Destruye la interfaz COM subyacente y libera la memoria de video.
     *
     * Implementa una liberación segura llamando a @c Release() sobre el objeto interno.
     * Es una operación idempotente, segura de ejecutar múltiples veces.
     *
     * @post El puntero @c m_renderTargetView se restablecerá a @c nullptr.
     */
    void
        destroy();

private:
        // ============================================================================
        // Interfaz Nativa
        // ============================================================================
        /**
         * @brief Puntero al recurso COM de Direct3D 11.
         * @details Gestiona la comunicación de bajo nivel para las escrituras de color en GPU.
         */
        ID3D11RenderTargetView* m_renderTargetView = nullptr;
};