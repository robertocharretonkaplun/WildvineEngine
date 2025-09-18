#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class Texture
 * @brief Encapsula una textura 2D en Direct3D 11, incluyendo su recurso y vista como Shader Resource.
 *
 * Esta clase administra texturas que pueden provenir de:
 * - Archivos de imagen (png, jpg, etc.).
 * - Texturas creadas en memoria (RTV, DSV, UAV).
 * - Copias a partir de otra textura.
 *
 * Proporciona m�todos para inicializaci�n, actualizaci�n, uso en shaders y destrucci�n.
 */
class 
Texture {
public:
  /**
   * @brief Constructor por defecto.
   */
  Texture()  = default;

  /**
   * @brief Destructor por defecto.
   * @details No libera autom�ticamente los recursos COM; llamar a destroy().
   */
  ~Texture() = default;

  /**
   * @brief Inicializa una textura cargada desde archivo.
   *
   * Crea un recurso de textura a partir de un archivo de imagen y genera su
   * @c ShaderResourceView correspondiente para ser usado en shaders.
   *
   * @param device        Dispositivo con el que se crear� la textura.
   * @param textureName   Nombre o ruta del archivo de textura.
   * @param extensionType Tipo de extensi�n de archivo (ej. PNG, JPG, DDS).
   * @return @c S_OK si fue exitoso; c�digo @c HRESULT en caso contrario.
   *
   * @post Si retorna @c S_OK, @c m_texture y @c m_textureFromImg != nullptr.
   */
  HRESULT 
  init(Device & device,
       const std::string & textureName,
       ExtensionType extensionType);

  /**
   * @brief Inicializa una textura creada desde memoria.
   *
   * Crea un recurso de textura 2D vac�o con un tama�o y formato especificados.
   * �til para render targets, depth buffers o texturas din�micas.
   *
   * @param device        Dispositivo con el que se crear� la textura.
   * @param width         Ancho de la textura en p�xeles.
   * @param height        Alto de la textura en p�xeles.
   * @param Format        Formato DXGI de la textura (ej. DXGI_FORMAT_R8G8B8A8_UNORM).
   * @param BindFlags     Banderas de enlace (ej. @c D3D11_BIND_SHADER_RESOURCE, @c D3D11_BIND_RENDER_TARGET).
   * @param sampleCount   N�mero de muestras para MSAA (por defecto 1 = sin MSAA).
   * @param qualityLevels Niveles de calidad soportados para MSAA.
   * @return @c S_OK si fue exitoso; c�digo @c HRESULT en caso contrario.
   */
  HRESULT 
  init(Device & device,
       unsigned int width,
       unsigned int height,
       DXGI_FORMAT Format,
       unsigned int BindFlags,
       unsigned int sampleCount = 1,
       unsigned int qualityLevels = 0);

  /**
   * @brief Inicializa una textura a partir de otra existente.
   *
   * Crea una nueva textura basada en la descripci�n de @p textureRef,
   * con un formato diferente.
   *
   * @param device     Dispositivo con el que se crear� la textura.
   * @param textureRef Referencia a otra textura existente.
   * @param format     Nuevo formato DXGI de la textura.
   * @return @c S_OK si fue exitoso; c�digo @c HRESULT en caso contrario.
   */
  HRESULT 
  init(Device& device, Texture& textureRef, DXGI_FORMAT format);

  /**
   * @brief Actualiza el contenido de la textura.
   *
   * M�todo de marcador, �til para soportar carga din�mica de datos o streaming
   * de texturas desde CPU hacia GPU.
   *
   * @note Actualmente no realiza ninguna operaci�n.
   */
  void 
  update();

  /**
   * @brief Asigna la textura al pipeline de render.
   *
   * Llama a @c PSSetShaderResources para establecer la textura como
   * recurso de shader en la etapa de Pixel Shader.
   *
   * @param deviceContext Contexto donde se aplicar� la textura.
   * @param StartSlot     Slot inicial en el que se vincular� la textura.
   * @param NumViews      N�mero de vistas de recurso a asignar (normalmente 1).
   *
   * @pre @c m_textureFromImg debe haberse creado con init().
   */
  void 
  render(DeviceContext & deviceContext, unsigned int StartSlot, unsigned int NumViews);

  /**
   * @brief Libera los recursos de la textura.
   *
   * Libera tanto el recurso de textura (@c ID3D11Texture2D) como su
   * @c ShaderResourceView asociado.
   *
   * @post @c m_texture == nullptr y @c m_textureFromImg == nullptr.
   */
  void 
  destroy();

public:
  /**
   * @brief Recurso base de la textura en GPU.
   */
  ID3D11Texture2D* m_texture = nullptr;

  /**
   * @brief Vista de la textura como recurso de shader.
   *
   * Permite acceder a la textura desde Pixel Shader u otros shaders.
   */
  ID3D11ShaderResourceView* m_textureFromImg = nullptr;

  /**
   * @brief Nombre o ruta de la textura (si proviene de archivo).
   */
  std::string m_textureName;
};
