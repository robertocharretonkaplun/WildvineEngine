#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;

/**
 * @class   DepthStencilView
 * @brief   Encapsula la creación y gestión de un @c ID3D11DepthStencilView.
 *
 * Esta clase administra una vista de profundidad y esténcil (DSV) en DirectX 11.
 * El DSV es indispensable en la etapa de Output-Merger del pipeline gráfico para
 * determinar qué píxeles son visibles (Z-buffering) y aplicar máscaras lógicas
 * (Stencil testing). Actúa como un puente entre una textura subyacente que almacena
 * los datos de profundidad y el contexto del dispositivo que los evalúa.
 */
class
    DepthStencilView {
public:
    /**
     * @brief Constructor por defecto.
     * @details No inicializa recursos en la GPU automáticamente. Se debe invocar @c init().
     */
    DepthStencilView() = default;

    /**
     * @brief Destructor por defecto.
     * @warning Al igual que con otros wrappers COM, no libera automáticamente la
     * memoria de video. Se debe llamar a @c destroy() explícitamente.
     */
    ~DepthStencilView() = default;

    /**
     * @brief Crea la vista de profundidad/esténcil a partir de una textura.
     *
     * Asocia un @c DepthStencilView básico a una textura que fue previamente creada
     * con la bandera de enlace @c D3D11_BIND_DEPTH_STENCIL.
     *
     * @param device       Dispositivo gráfico responsable de la creación del recurso.
     * @param depthStencil Textura 2D que actuará como el buffer de profundidad/esténcil.
     * @param format       Formato de datos DXGI compatible (ej. @c DXGI_FORMAT_D24_UNORM_S8_UINT).
     * @return             Código @c HRESULT indicando el éxito o el motivo del fallo.
     *
     * @post Si retorna @c S_OK, el puntero @c m_depthStencilView será válido para su uso.
     */
    HRESULT
           init(Device& device, 
                Texture& depthStencil, 
                DXGI_FORMAT format);

    /**
     * @brief Crea la vista de profundidad/esténcil especificando su dimensionalidad.
     *
     * Versión sobrecargada que permite definir explícitamente cómo la GPU debe
     * interpretar la textura (por ejemplo, como una textura 2D multisampleada,
     * un arreglo de texturas o una cara de un cubemap).
     *
     * @param device        Dispositivo gráfico responsable de la creación.
     * @param depthStencil  Textura base de profundidad.
     * @param format        Formato de datos DXGI de la vista.
     * @param viewDimension Tipología dimensional nativa de DirectX (ej. @c D3D11_DSV_DIMENSION_TEXTURE2D).
     * @return              Código @c HRESULT indicando el estado de la operación.
     */
    HRESULT
           init(Device& device,
                Texture& depthStencil,
                DXGI_FORMAT format,
                D3D11_DSV_DIMENSION viewDimension);

    /**
     * @brief Lógica de actualización del recurso.
     *
     * Método reservado para mantener la coherencia con la arquitectura del motor.
     * Actualmente carece de implementación activa ya que un DSV rara vez requiere
     * actualizaciones de estado por frame más allá de ser limpiado (Clear).
     */
    void
        update() {};

    /**
     * @brief Vincula la vista de profundidad/esténcil al pipeline de renderizado.
     *
     * Notifica al @c DeviceContext que utilice este DSV en la etapa de Output-Merger
     * (generalmente a través de llamadas combinadas en @c OMSetRenderTargets).
     *
     * @param deviceContext Contexto del dispositivo para emitir el comando.
     *
     * @pre El recurso @c m_depthStencilView debe haberse inicializado correctamente.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera la vista y la memoria de video asociada al objeto COM.
     *
     * Resetea el puntero interno de manera segura. Es una operación idempotente,
     * por lo que puede llamarse múltiples veces sin causar errores.
     *
     * @post @c m_depthStencilView apuntará a @c nullptr.
     */
    void
        destroy();

public:
        // ============================================================================
        // Interfaz Nativa
        // ============================================================================
        /** * @brief Puntero directo al recurso COM administrado por la clase.
         * @details Permite el acceso de bajo nivel a las funciones de @c ID3D11DepthStencilView.
         */
        ID3D11DepthStencilView* m_depthStencilView = nullptr;
};