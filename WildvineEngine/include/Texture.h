#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class   Texture
 * @brief   Encapsula una textura 2D en Direct3D 11 y su Vista de Recurso de Shader (SRV).
 *
 * Esta clase administra el ciclo de vida de los datos de píxeles en la
 * memoria de video (VRAM). Es altamente versátil y soporta texturas provenientes de:
 * - Archivos de imagen estándar en disco (PNG, JPG, DDS).
 * - Creación programática en memoria (usado para Render Targets y Depth Stencil Buffers).
 * - Copias estructuradas a partir de otras texturas.
 * - Arreglos de texturas para mapas de entorno (Cubemaps).
 * * Proporciona automáticamente el @c ID3D11ShaderResourceView, necesario para que
 * los shaders puedan muestrear (leer) la textura durante la rasterización.
 */
class
    Texture {

public:
    /**
     * @brief Constructor por defecto.
     * @details Instancia el contenedor vacío. Se requiere llamar a una de las variantes de @c init().
     */
    Texture() = default;

    /**
     * @brief Destructor por defecto.
     * @warning Al igual que otras clases que encapsulan objetos COM, no libera
     * automáticamente la memoria de video. Se debe llamar a @c destroy() explícitamente.
     */
    ~Texture() = default;

    // ============================================================================
    // Inicialización y Creación
    // ============================================================================

    /**
     * @brief Carga e inicializa una textura desde un archivo en disco.
     *
     * Utiliza utilidades de DirectX (como WICTextureLoader o DDSTextureLoader) para
     * decodificar la imagen, subirla a la GPU y generar su SRV automáticamente.
     *
     * @param device        Dispositivo gráfico responsable de la creación.
     * @param textureName   Ruta o nombre del archivo de textura (ej. "albedo.png").
     * @param extensionType Formato de archivo indicado por el enumerador (PNG, JPG, DDS).
     * @return              Código @c HRESULT nativo indicando el éxito (@c S_OK) o error.
     *
     * @post Si retorna @c S_OK, los punteros @c m_texture y @c m_textureFromImg serán válidos.
     */
    HRESULT
            init(Device& device,
                 const std::string& textureName,
                 ExtensionType extensionType);

    /**
     * @brief Crea una textura programática vacía en la memoria de video.
     *
     * Genera un recurso de textura 2D con dimensiones y formato explícitos.
     * Fundamental para crear texturas que actuarán como destinos de renderizado (RTV)
     * o buffers de profundidad (DSV) en pasadas off-screen.
     *
     * @param device        Dispositivo gráfico creador.
     * @param width         Ancho deseado de la textura en píxeles.
     * @param height        Alto deseado de la textura en píxeles.
     * @param Format        Formato de píxel DXGI (ej. @c DXGI_FORMAT_R8G8B8A8_UNORM).
     * @param BindFlags     Banderas de uso (ej. @c D3D11_BIND_RENDER_TARGET | @c D3D11_BIND_SHADER_RESOURCE).
     * @param sampleCount   Cantidad de muestras por píxel para MSAA (1 = desactivado).
     * @param qualityLevels Niveles de calidad del muestreo MSAA.
     * @return              Código @c HRESULT nativo de la operación.
     */
    HRESULT
        init(Device& device,
            unsigned int width,
            unsigned int height,
            DXGI_FORMAT Format,
            unsigned int BindFlags,
            unsigned int sampleCount = 1,
            unsigned int qualityLevels = 0);

    /**
     * @brief Crea una nueva textura heredando las dimensiones de otra existente.
     *
     * Extrae la descripción de @p textureRef y crea un recurso idéntico en tamaño,
     * pero aplicando el nuevo @p format especificado.
     *
     * @param device     Dispositivo gráfico creador.
     * @param textureRef Textura de origen que servirá como plantilla (template).
     * @param format     Nuevo formato DXGI a aplicar.
     * @return           Código @c HRESULT nativo de la operación.
     */
    HRESULT
            init(Device& device, 
                 Texture& textureRef, 
                 DXGI_FORMAT format);

    /**
     * @brief Lógica de actualización de la textura.
     *
     * Método de marcador arquitectónico. Útil para futuras implementaciones de
     * texturas dinámicas (donde la CPU sube datos a la GPU cada frame).
     *
     * @note Actualmente carece de implementación activa.
     */
    void
        update();

    /**
     * @brief Vincula la textura (SRV) al pipeline gráfico para ser leída por los shaders.
     *
     * Llama internamente a @c PSSetShaderResources, permitiendo que el Pixel Shader
     * muestree los colores de esta textura.
     *
     * @param deviceContext Contexto del dispositivo para emitir el comando.
     * @param StartSlot     Ranura (Slot) del registro del shader donde se enlazará la textura.
     * @param NumViews      Cantidad de vistas a enlazar simultáneamente (generalmente 1).
     *
     * @pre @c m_textureFromImg debe haber sido instanciado correctamente con @c init().
     */
    void
        render(DeviceContext& deviceContext, 
                unsigned int StartSlot, 
                unsigned int NumViews);

    /**
     * @brief Libera la memoria de video de la textura y su vista.
     *
     * Destruye de forma segura los objetos COM de @c ID3D11Texture2D y
     * @c ID3D11ShaderResourceView. Es una operación idempotente.
     *
     * @post @c m_texture y @c m_textureFromImg se restablecerán a @c nullptr.
     */
    void
        destroy();

    // ============================================================================
    // Herramientas de Cubemap
    // ============================================================================

    /**
     * @brief Genera un mapa de entorno (Cubemap) a partir de 6 imágenes separadas.
     *
     * Carga 6 texturas bidimensionales y las ensambla en un único @c ID3D11Texture2D
     * configurado con la bandera @c D3D11_RESOURCE_MISC_TEXTURECUBE.
     *
     * @param device        Dispositivo gráfico creador.
     * @param deviceContext Contexto del dispositivo para la copia en memoria y mips.
     * @param facePaths     Arreglo fijo de 6 rutas (strings) a las imágenes de las caras (+X, -X, +Y, -Y, +Z, -Z).
     * @param generateMips  Si es @c true, calculará automáticamente los niveles de mipmap.
     * @return              Código @c HRESULT nativo indicando el éxito o fallo.
     */
    HRESULT
            CreateCubemap(Device& device,
                          DeviceContext& deviceContext,
                          const std::array<std::string, 6>& facePaths,
                          bool generateMips /*= false*/);

    /**
     * @brief Extrae una vista (SRV) de una única cara específica de un Cubemap.
     *
     * Interfaz en línea que facilita la extracción de una cara individual tratándola
     * como un @c Texture2DArray de un solo elemento.
     *
     * @param device     Puntero nativo al dispositivo creador.
     * @param cubemapTex Puntero nativo al Cubemap origen.
     * @param format     Formato de la textura.
     * @param faceIndex  Índice de la cara a extraer (0 a 5).
     * @param mipLevels  Cantidad de niveles de mipmap a incluir en la vista.
     * @return           Puntero nativo al nuevo @c ID3D11ShaderResourceView, o @c nullptr si falla.
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(
        ID3D11Device* device,
        ID3D11Texture2D* cubemapTex,
        DXGI_FORMAT format,
        UINT faceIndex,
        UINT mipLevels = 1
    )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC d{};
        d.Format = format;
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        d.Texture2DArray.MostDetailedMip = 0;
        d.Texture2DArray.MipLevels = mipLevels;       // usa 1 para vista simple
        d.Texture2DArray.FirstArraySlice = faceIndex; // cara
        d.Texture2DArray.ArraySize = 1;               // solo esa cara

        ID3D11ShaderResourceView* srv = nullptr;
        if (FAILED(device->CreateShaderResourceView(cubemapTex, &d, &srv)))
            return nullptr;

        return srv;
    }

public:
        // ============================================================================
        // Interfaz Nativa
        // ============================================================================

        /** * @brief Puntero al recurso de datos crudos (píxeles) alojado en la GPU.
         */
        ID3D11Texture2D* m_texture = nullptr;

        /** * @brief Puntero a la vista del recurso configurada para lectura desde los shaders.
         */
        ID3D11ShaderResourceView* m_textureFromImg = nullptr;

        /** * @brief Almacena la ruta o nombre del archivo de origen para propósitos de depuración o recarga.
         */
        std::string m_textureName;
};