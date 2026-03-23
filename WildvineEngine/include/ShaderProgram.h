#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

class Device;
class DeviceContext;
class LayoutBuilder;

/**
 * @class   ShaderProgram
 * @brief   Encapsula la creación, compilación y vinculación de shaders en Direct3D 11.
 *
 * La clase @c ShaderProgram administra el ciclo de vida completo de un
 * conjunto de shaders (habitualmente Vertex Shader y Pixel Shader) requeridos para
 * la etapa programable del pipeline gráfico. Su responsabilidad abarca desde la
 * lectura y compilación de archivos de código fuente HLSL, hasta la extracción del
 * bytecode y la creación de la firma de entrada (Input Layout) que le indica a la
 * GPU cómo interpretar los datos crudos de los vértices.
 */
class
    ShaderProgram {

public:
    /**
     * @brief Constructor por defecto.
     * @details Instancia la clase vacía. Requiere una llamada posterior a @c init()
     * para compilar y generar los recursos en la GPU.
     */
    ShaderProgram() = default;

    /**
     * @brief Destructor por defecto.
     * @warning Esta clase administra interfaces COM e @c ID3DBlob subyacentes.
     * No libera automáticamente la memoria; es indispensable llamar a @c destroy().
     */
    ~ShaderProgram() = default;

    /**
     * @brief Inicializa el programa de shaders desde un archivo fuente HLSL.
     *
     * Realiza el proceso completo de creación: lee el archivo, compila los puntos
     * de entrada estándar (usualmente "VS" para Vertex y "PS" para Pixel), instancia
     * los shaders en la GPU y valida la memoria de los vértices usando el layout proporcionado.
     *
     * @param device        Dispositivo gráfico de DirectX 11 para la creación de recursos.
     * @param fileName      Ruta o nombre del archivo HLSL que contiene el código fuente.
     * @param layoutBuilder Objeto constructor que define la topología de los vértices (semánticas y formatos).
     * @return              Código @c HRESULT nativo indicando el éxito (@c S_OK) o el error de compilación/creación.
     *
     * @post Si es exitoso, los punteros a los shaders (@c m_VertexShader, @c m_PixelShader)
     * y el @c InputLayout estarán listos para ser enlazados.
     */
    HRESULT
        init(Device& device, 
             const std::string& fileName, 
             LayoutBuilder layoutBuilder);

    /**
     * @brief Lógica de actualización del programa de shaders.
     *
     * Método de marcador arquitectónico. Puede ser extendido en el futuro para
     * soportar la recarga en caliente (Hot-Reloading) de shaders modificados en tiempo de ejecución.
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Vincula el programa completo (VS, PS y Layout) al pipeline de renderizado.
     *
     * Llama internamente a las funciones del @c DeviceContext para establecer el
     * Vertex Shader (@c VSSetShader), el Pixel Shader (@c PSSetShader) y la topología
     * de memoria (@c IASetInputLayout) como los activos para las siguientes llamadas de dibujo.
     *
     * @param deviceContext Contexto del dispositivo responsable de emitir los comandos gráficos.
     *
     * @pre El programa debe haber sido compilado y creado exitosamente con @c init().
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Vincula únicamente una etapa específica del shader al pipeline.
     *
     * Permite establecer de forma aislada solo el Vertex Shader o el Pixel Shader.
     * Útil para pasadas de renderizado que solo requieren procesamiento de vértices
     * (como la generación de mapas de sombras).
     *
     * @param deviceContext Contexto del dispositivo para emitir el comando.
     * @param type          Etapa del pipeline a vincular (ej. @c VERTEX_SHADER o @c PIXEL_SHADER).
     */
    void
        render(DeviceContext& deviceContext, 
               ShaderType type);

    /**
     * @brief Libera todos los recursos compilados y la memoria de video asociada.
     *
     * Destruye de forma segura los objetos COM de los shaders, los blobs de memoria
     * y el Input Layout. Es una operación idempotente.
     *
     * @post Los punteros @c m_VertexShader, @c m_PixelShader, @c m_vertexShaderData
     * y @c m_pixelShaderData apuntarán a @c nullptr.
     */
    void
        destroy();

    /**
     * @brief Genera y asocia la firma de entrada (Input Layout) requerida por el Vertex Shader.
     *
     * Valida la estructura definida en C++ contra la firma compilada incrustada en
     * el bytecode del Vertex Shader, garantizando que los datos coincidan.
     *
     * @param device        Dispositivo gráfico creador.
     * @param layoutBuilder Colección de descriptores de elementos de entrada.
     * @return              Código @c HRESULT nativo de la operación en D3D11.
     */
    HRESULT
           CreateInputLayout(Device& device, 
                             LayoutBuilder layoutBuilder);

    /**
     * @brief Crea en memoria de video un shader previamente compilado o preconfigurado.
     *
     * Utiliza el archivo interno @c m_shaderFileName asignado para buscar e instanciar
     * el tipo de shader solicitado.
     *
     * @param device Dispositivo gráfico creador.
     * @param type   El tipo de shader a crear (Vertex o Pixel).
     * @return       Código @c HRESULT indicando el éxito de la creación.
     */
    HRESULT
           CreateShader(Device& device, 
                        ShaderType type);

    /**
     * @brief Compila y crea un shader directamente desde una ruta HLSL específica.
     *
     * @param device   Dispositivo gráfico creador.
     * @param type     El tipo de shader a compilar y crear.
     * @param fileName Ruta del código fuente HLSL.
     * @return         Código @c HRESULT indicando el éxito o el motivo del fallo.
     */
    HRESULT
           CreateShader(Device& device, 
                        ShaderType type, 
                        const std::string& fileName);

    /**
     * @brief Lee un archivo HLSL y compila su código fuente a bytecode de la GPU.
     *
     * Actúa como envoltorio para la función de la API @c D3DCompileFromFile.
     * Convierte el código de alto nivel a las instrucciones de bajo nivel que
     * entiende la tarjeta gráfica según el modelo de shader especificado (ej. Shader Model 5.0).
     *
     * @param szFileName    Ruta del archivo con el código fuente HLSL.
     * @param szEntryPoint  Nombre de la función que actúa como punto de entrada (ej. "VS" o "PS").
     * @param szShaderModel Versión del perfil de compilación requerido (ej. "vs_5_0" o "ps_5_0").
     * @param ppBlobOut     Puntero de salida donde se almacenará el bloque de memoria (Blob) con el bytecode.
     * @return              Código @c HRESULT nativo indicando si la compilación fue exitosa o si hubo errores de sintaxis.
     */
    HRESULT
            CompileShaderFromFile(char* szFileName,
                                  LPCSTR szEntryPoint,
                                  LPCSTR szShaderModel,
                                  ID3DBlob** ppBlobOut);

public:
        // ============================================================================
        // Recursos del Pipeline (Públicos)
        // ============================================================================

        /** @brief Puntero al Vertex Shader alojado en la memoria de la GPU. */
        ID3D11VertexShader* m_VertexShader = nullptr;

        /** @brief Puntero al Pixel Shader alojado en la memoria de la GPU. */
        ID3D11PixelShader* m_PixelShader = nullptr;

        /** @brief Descriptor que define cómo mapear la memoria del Vertex Buffer a la entrada del shader. */
        InputLayout m_inputLayout;

private:
        // ============================================================================
        // Metadatos y Bytecodes (Privados)
        // ============================================================================

        /** @brief Almacena la ruta del archivo HLSL original para recargas o referencias futuras. */
        std::string m_shaderFileName;

        /** @brief Bloque de memoria (Blob) que contiene las instrucciones compiladas del Vertex Shader. */
        ID3DBlob* m_vertexShaderData = nullptr;

        /** @brief Bloque de memoria (Blob) que contiene las instrucciones compiladas del Pixel Shader. */
        ID3DBlob* m_pixelShaderData = nullptr;
};