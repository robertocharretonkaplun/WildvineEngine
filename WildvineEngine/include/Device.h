#pragma once
#include "Prerequisites.h"

/**
 * @class   Device
 * @brief   Encapsula la creación y gestión del dispositivo gráfico @c ID3D11Device.
 *
 * En la arquitectura de DirectX 11, el "Dispositivo" actúa como una fábrica
 * (Factory) de recursos. Esta clase abstrae las llamadas a la API de Microsoft para
 * instanciar de manera segura vistas, texturas, shaders, estados y buffers en la
 * memoria de video (VRAM).
 *
 * @note Esta clase se limita exclusivamente a la creación de recursos y comprobación de
 * capacidades del hardware; no emite comandos de dibujo ni administra el pipeline
 * gráfico.
 */
class
	Device {

public:
	/**
	 * @brief Constructor por defecto.
	 */
	Device() = default;

	/**
	 * @brief Destructor por defecto.
	 * @warning Se debe llamar a @c destroy() antes de la destrucción para evitar fugas de memoria COM.
	 */
	~Device() = default;

	/**
	 * @brief Inicializa el dispositivo gráfico.
	 *
	 * Configura los niveles de características (Feature Levels) soportados por
	 * el hardware (típicamente D3D_FEATURE_LEVEL_11_0) y crea el objeto COM interno.
	 */
	void
		init();

	/**
	 * @brief Lógica de actualización general del dispositivo.
	 * @note Raramente utilizado en la fábrica de recursos; incluido para consistencia de arquitectura.
	 */
	void
		update();

	/**
	 * @brief Fase de renderizado.
	 * @note El Device no emite comandos de renderizado directo.
	 */
	void
		render();

	/**
	 * @brief Libera el @c ID3D11Device base y limpia las interfaces COM asociadas.
	 */
	void
		destroy();

	// ============================================================================
	// Fábrica de Vistas (Views) y Texturas
	// ============================================================================

	/**
	 * @brief Crea una vista de destino de renderizado (Render Target View).
	 *
	 * Permite que el pipeline gráfico escriba color directamente en un recurso
	 * (normalmente el backbuffer de la SwapChain o una textura off-screen).
	 *
	 * @param pResource Recurso de origen previamente creado en memoria de video.
	 * @param pDesc     Descriptor opcional que define el formato y dimensión de la vista
	 * (puede ser @c nullptr para heredar los del recurso).
	 * @param ppRTView  Puntero de salida donde se guardará la interfaz instanciada.
	 * @return          Código @c HRESULT de DirectX indicando el éxito (@c S_OK) o error.
	 */
	HRESULT
			CreateRenderTargetView(ID3D11Resource* pResource,
								   const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
								   ID3D11RenderTargetView** ppRTView);

	/**
	 * @brief Reserva memoria en la GPU para una textura bidimensional.
	 *
	 * @param pDesc        Descriptor que define ancho, alto, formato, mipmaps y banderas de uso.
	 * @param pInitialData Arreglo opcional con los píxeles iniciales a transferir (puede ser @c nullptr).
	 * @param ppTexture2D  Puntero de salida donde se guardará la textura generada.
	 * @return             Código @c HRESULT indicando el resultado de la creación.
	 */
	HRESULT
			CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
							const D3D11_SUBRESOURCE_DATA* pInitialData,
							ID3D11Texture2D** ppTexture2D);

	/**
	 * @brief Crea una vista de profundidad y esténcil (Depth Stencil View).
	 *
	 * @param pResource          Textura base (debe haber sido creada con @c D3D11_BIND_DEPTH_STENCIL).
	 * @param pDesc              Descriptor opcional para personalizar el formato de la vista.
	 * @param ppDepthStencilView Puntero de salida para la vista creada.
	 * @return                   Código @c HRESULT nativo de la operación.
	 */
	HRESULT
			CreateDepthStencilView(ID3D11Resource* pResource,
								   const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
								   ID3D11DepthStencilView** ppDepthStencilView);

	// ============================================================================
	// Fábrica de Shaders y Layouts
	// ============================================================================

  /**
   * @brief Instancia un programa de Vertex Shader en la GPU.
   *
   * @param pShaderBytecode El código objeto (CSO) compilado del shader.
   * @param BytecodeLength  Tamaño en bytes del arreglo compilado.
   * @param pClassLinkage   Enlace dinámico de clases de HLSL (usualmente @c nullptr).
   * @param ppVertexShader  Puntero de salida al nuevo Vertex Shader.
   * @return                Código @c HRESULT nativo de la operación.
   */
	HRESULT
			CreateVertexShader(const void* pShaderBytecode,
							   unsigned int BytecodeLength,
							   ID3D11ClassLinkage* pClassLinkage,
							   ID3D11VertexShader** ppVertexShader);

	/**
	 * @brief Define cómo el ensamblador de entrada (Input Assembler) leerá los vértices.
	 *
	 * Valida la estructura de los vértices enviada desde C++ contra la firma
	 * de entrada esperada por el Vertex Shader.
	 *
	 * @param pInputElementDescs                Arreglo de descriptores de elementos (semánticas, formatos, slots).
	 * @param NumElements                       Cantidad total de elementos en el arreglo.
	 * @param pShaderBytecodeWithInputSignature Bytecode del Vertex Shader para la validación cruzada.
	 * @param BytecodeLength                    Tamaño en bytes del bytecode.
	 * @param ppInputLayout                     Puntero de salida para el layout validado.
	 * @return                                  Código @c HRESULT nativo de la operación.
	 */
	HRESULT
		CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
						  unsigned int NumElements,
						  const void* pShaderBytecodeWithInputSignature,
						  unsigned int BytecodeLength,
						  ID3D11InputLayout** ppInputLayout);

	/**
	 * @brief Instancia un programa de Pixel Shader en la GPU.
	 *
	 * @param pShaderBytecode Código compilado (CSO) del pixel shader.
	 * @param BytecodeLength  Tamaño del código en bytes.
	 * @param pClassLinkage   Enlace de clases dinámico (usualmente @c nullptr).
	 * @param ppPixelShader   Puntero de salida al nuevo Pixel Shader.
	 * @return                Código @c HRESULT nativo de la operación.
	 */
	HRESULT
			CreatePixelShader(const void* pShaderBytecode,
						      unsigned int BytecodeLength,
							  ID3D11ClassLinkage* pClassLinkage,
							  ID3D11PixelShader** ppPixelShader);

	// ============================================================================
	// Fábrica de Buffers y Estados
	// ============================================================================

  /**
   * @brief Crea un Buffer genérico en memoria de video.
   *
   * Usado para instanciar Vertex Buffers, Index Buffers o Constant Buffers dependiendo
   * de las banderas especificadas en su descriptor.
   *
   * @param pDesc        Descriptor estructurado que define propósito, uso y tamaño.
   * @param pInitialData Información inicial opcional a inyectar en el buffer al momento de crearlo.
   * @param ppBuffer     Puntero de salida al nuevo @c ID3D11Buffer.
   * @return             Código @c HRESULT nativo de la operación.
   */
	HRESULT
		   CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
						const D3D11_SUBRESOURCE_DATA* pInitialData,
						ID3D11Buffer** ppBuffer);

	/**
	 * @brief Crea un bloque de estado para el muestreo de texturas.
	 *
	 * Define cómo la GPU debe leer los texeles (ej. filtrado bilineal, anisotrópico, clamp o wrap).
	 *
	 * @param pSamplerDesc   Descriptor con las reglas de filtrado y direccionamiento.
	 * @param ppSamplerState Puntero de salida al estado instanciado.
	 * @return               Código @c HRESULT nativo de la operación.
	 */
	HRESULT
		   CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
						      ID3D11SamplerState** ppSamplerState);

public:
	// ============================================================================
	// Interfaz Nativa
	// ============================================================================
	/**
	 * @brief Interfaz directa del dispositivo en DirectX 11.
	 * @details Puntero COM instanciado exitosamente tras llamar a @c init(), y limpiado mediante @c destroy().
	 */
	ID3D11Device* m_device = nullptr;
};