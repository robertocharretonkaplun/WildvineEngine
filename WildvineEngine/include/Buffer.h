#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class   Buffer
 * @brief   Encapsula la creación y gestión de un recurso @c ID3D11Buffer de DirectX 11.
 *
 * Esta clase actúa como un "wrapper" multipropósito que administra el ciclo
 * de vida y las operaciones de un único buffer en la GPU. Soporta su uso como Buffer
 * de Vértices (Vertex Buffer), Buffer de Índices (Index Buffer) o Buffer Constante
 * (Constant Buffer), deduciendo su comportamiento en la etapa de renderizado a partir
 * de la bandera de enlace (@c m_bindFlag) con la que fue inicializado.
 *
 * @warning La clase no implementa semántica de copia profunda para el recurso COM.
 * Si se requieren copias, se deben manejar explícitamente las referencias del @c ID3D11Buffer.
 */
class
    Buffer {
public:
    /**
     * @brief Constructor por defecto.
     * @details No inicializa recursos en la GPU automáticamente. Se debe llamar a @c init().
     */
    Buffer() = default;

    /**
     * @brief Destructor por defecto.
     * @warning No libera automáticamente la memoria de video. Se debe invocar @c destroy()
     * antes de destruir el objeto para evitar fugas de memoria (Memory Leaks).
     */
    ~Buffer() = default;

    /**
     * @brief Inicializa el recurso como un Vertex Buffer o Index Buffer.
     *
     * Extrae la información geométrica directamente de un @c MeshComponent y solicita
     * la creación del buffer correspondiente en la GPU.
     *
     * @param device   Referencia al dispositivo gráfico encargado de la creación.
     * @param mesh     Componente que contiene los arreglos de vértices o índices.
     * @param bindFlag Bandera que define el uso (@c D3D11_BIND_VERTEX_BUFFER o @c D3D11_BIND_INDEX_BUFFER).
     * @return         Código @c HRESULT que indica el éxito (@c S_OK) o el fallo de la operación.
     *
     * @post Si es exitoso, @c m_buffer será válido y @c m_bindFlag guardará el propósito del buffer.
     */
    HRESULT
           init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa el recurso como un Constant Buffer (Buffer Constante).
     *
     * Crea un buffer en la GPU diseñado para actualizar variables de los shaders (como
     * matrices de mundo, vista y proyección).
     *
     * @param device    Referencia al dispositivo gráfico.
     * @param ByteWidth Tamaño total del buffer en bytes.
     * @note            Por restricciones estrictas de DirectX 11, este tamaño **debe** * ser múltiplo de 16 bytes.
     * @return          Código @c HRESULT indicando el resultado de la creación.
     */
    HRESULT
           init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Sobrescribe los datos internos del buffer en la GPU.
     *
     * Utiliza internamente la función @c UpdateSubresource del contexto del dispositivo.
     * Es el método principal para enviar las transformaciones frame a frame a los Constant Buffers.
     *
     * @param deviceContext  Contexto del dispositivo utilizado para la transferencia.
     * @param pDstResource   Puntero al recurso destino (generalmente el propio @c m_buffer).
     * @param DstSubresource Índice del subrecurso (0 por defecto para buffers simples).
     * @param pDstBox        Caja delimitadora para actualizar una región específica (nulo para todo el buffer).
     * @param pSrcData       Puntero en RAM a los nuevos datos a transferir.
     * @param SrcRowPitch    Distancia entre filas (ignorado por D3D11 para este tipo de buffers).
     * @param SrcDepthPitch  Distancia entre rebanadas de profundidad (ignorado por D3D11).
     */
    void
        update(DeviceContext& deviceContext,
               ID3D11Resource* pDstResource,
               unsigned int    DstSubresource,
               const D3D11_BOX* pDstBox,
               const void* pSrcData,
               unsigned int    SrcRowPitch,
               unsigned int    SrcDepthPitch);

    /**
     * @brief Enlaza el buffer a la etapa correspondiente del pipeline gráfico.
     *
     * Dependiendo del @c m_bindFlag establecido durante la inicialización, este método
     * decidirá automáticamente si debe llamar a @c IASetVertexBuffers, @c IASetIndexBuffer,
     * o a los métodos de enlace de los shaders (@c VSSetConstantBuffers / @c PSSetConstantBuffers).
     *
     * @param deviceContext  Contexto del dispositivo para emitir el comando.
     * @param StartSlot      Ranura (Slot) de registro inicial donde se conectará el buffer.
     * @param NumBuffers     Cantidad de buffers a conectar simultáneamente (típicamente 1).
     * @param setPixelShader Si es @c true y es un Constant Buffer, lo enlazará también al Pixel Shader.
     * @param format         Formato de los datos (ej. @c DXGI_FORMAT_R32_UINT) aplicable solo a Index Buffers.
     */
    void
        render(DeviceContext& deviceContext,
               unsigned int   StartSlot,
               unsigned int   NumBuffers,
               bool           setPixelShader = false,
               DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera la memoria de video y resetea los metadatos.
     *
     * Implementa una liberación segura del objeto COM (@c ID3D11Buffer) y devuelve
     * los valores de offset y stride a cero. Es una operación idempotente (segura
     * de llamar múltiples veces).
     */
    void
        destroy();

    /**
     * @brief Función de bajo nivel para instanciar el buffer en la API de DirectX.
     *
     * Llamada internamente por las funciones @c init públicas para encapsular
     * el proceso de creación genérica mediante @c CreateBuffer.
     *
     * @param device   Dispositivo gráfico.
     * @param desc     Descriptor que especifica las propiedades estructurales del recurso.
     * @param initData Datos iniciales a cargar en la memoria de video (puede ser @c nullptr).
     * @return         Código @c HRESULT nativo de DirectX.
     */
    HRESULT
            createBuffer(Device& device,
                D3D11_BUFFER_DESC& desc,
                D3D11_SUBRESOURCE_DATA* initData);

public:
        // ============================================================================
        // Interfaz Nativa
        // ============================================================================
        /** @brief Puntero directo al recurso COM administrado por esta clase. */
        ID3D11Buffer* m_buffer = nullptr;

private:
        // ============================================================================
        // Metadatos de Enlace (Binding)
        // ============================================================================
        /**
         * @brief Tamaño en bytes de un único elemento del buffer.
         * @details Utilizado exclusivamente cuando actúa como Vertex Buffer para @c IASetVertexBuffers.
         */
        unsigned int m_stride = 0;

        /**
         * @brief Desplazamiento inicial en bytes desde el principio del buffer.
         * @details Utilizado al enlazar Vertex Buffers. Por lo general, su valor es 0.
         */
        unsigned int m_offset = 0;

        /**
         * @brief Identificador del rol o propósito funcional del buffer en el pipeline.
         * @details Almacena banderas nativas como @c D3D11_BIND_VERTEX_BUFFER.
         */
        unsigned int m_bindFlag = 0;
};