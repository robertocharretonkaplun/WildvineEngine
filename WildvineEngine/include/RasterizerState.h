#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class   RasterizerState
 * @brief   Encapsula la configuración de la etapa de rasterización en el pipeline gráfico.
 *
 * La etapa de rasterización en Direct3D 11 es el puente entre el procesamiento
 * geométrico y el procesamiento de píxeles. Define cómo se transforman las primitivas matemáticas
 * (triángulos, líneas) en fragmentos (píxeles) 2D. Esta clase administra el ciclo de vida
 * de un objeto @c ID3D11RasterizerState, permitiendo alterar dinámicamente el modo de
 * relleno (Wireframe o Sólido), el descarte de caras (Culling) y el recorte de profundidad.
 */
class
    RasterizerState {

public:
    /**
     * @brief Constructor por defecto.
     * @details No inicializa recursos en la GPU automáticamente. Se debe invocar a @c init().
     */
    RasterizerState() = default;

    /**
     * @brief Destructor por defecto.
     * @warning Al envolver un objeto COM de DirectX, la memoria de video no se libera
     * automáticamente. Es obligatorio llamar a @c destroy() antes de que el objeto sea destruido.
     */
    ~RasterizerState() = default;

    /**
     * @brief Inicializa el estado de rasterización con valores predeterminados.
     *
     * Crea un @c ID3D11RasterizerState configurado típicamente para renderizado estándar
     * (por ejemplo, relleno sólido @c D3D11_FILL_SOLID y descarte de caras traseras @c D3D11_CULL_BACK).
     *
     * @param device Dispositivo gráfico responsable de la creación del recurso.
     * @return       Código @c HRESULT nativo de DirectX indicando el resultado de la operación.
     *
     * @post Si retorna @c S_OK, @c m_rasterizerState apuntará a un bloque de estado válido.
     */
    HRESULT
           init(Device device);

    /**
     * @brief Inicializa el estado de rasterización con parámetros específicos.
     *
     * Permite crear un estado a medida, ideal para depuración (Wireframe), renderizado
     * de dos caras (Cull None), o para alterar el orden de los vértices frontales.
     *
     * @param device    Referencia al dispositivo gráfico creador.
     * @param fill      Modo de relleno para las primitivas (ej. @c D3D11_FILL_SOLID o @c D3D11_FILL_WIREFRAME).
     * @param cull      Modo de descarte geométrico (ej. @c D3D11_CULL_NONE, @c D3D11_CULL_FRONT, @c D3D11_CULL_BACK).
     * @param frontCCW  Define si los triángulos con vértices en sentido antihorario (Counter-Clockwise) se consideran frontales.
     * @param depthClip Habilita o deshabilita el recorte de píxeles basándose en la distancia.
     * @return          Código @c HRESULT nativo de la operación en D3D11.
     */
    HRESULT
           init(Device& device,
                D3D11_FILL_MODE fill,
                D3D11_CULL_MODE cull,
                bool frontCCW,
                bool depthClip);

    /**
     * @brief Lógica de actualización del estado de rasterización.
     *
     * Método de marcador arquitectónico. Los estados en DirectX 11 son inmutables una vez
     * creados; para cambiar el estado, normalmente se crea uno nuevo o se alterna entre
     * estados precreados durante el @c render. Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Vincula este estado de rasterización al pipeline gráfico.
     *
     * Llama internamente a @c RSSetState sobre el contexto del dispositivo, forzando
     * a la GPU a utilizar esta configuración de rasterización para las siguientes llamadas de dibujo.
     *
     * @param deviceContext Contexto del dispositivo para emitir el comando.
     *
     * @pre El recurso @c m_rasterizerState debe haber sido creado exitosamente con @c init().
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera la memoria de video y resetea la interfaz COM.
     *
     * Libera de manera segura el puntero a @c ID3D11RasterizerState.
     * Es una operación idempotente (puede llamarse múltiples veces sin riesgo de cuelgue).
     */
    void
        destroy();

private:
        // ============================================================================
        // Interfaz Nativa
        // ============================================================================
        /**
         * @brief Puntero al bloque de estado inmutable en la memoria de DirectX 11.
         * @details Válido después de una llamada exitosa a @c init(); @c nullptr tras @c destroy().
         */
        ID3D11RasterizerState* m_rasterizerState = nullptr;
};