#pragma once
#include "Prerequisites.h"

/**
 * @enum    ResourceType
 * @brief   Define las categorías de recursos soportadas por el motor.
 *
 * Clasifica el tipo de contenido que representa una instancia de recurso,
 * facilitando su gestión, filtrado y casteo seguro dentro del administrador de recursos.
 */
enum class
	ResourceType {
	Unknown,    ///< Tipo de recurso no definido o no reconocido.
	Model3D,    ///< Geometría tridimensional y datos de malla.
	Texture,    ///< Imagen 2D o mapa de bits utilizado para texturizado.
	Sound,      ///< Archivo de audio o efecto sonoro.
	Shader,     ///< Programa ejecutable en la GPU (Vertex, Pixel, Compute, etc.).
	Material    ///< Definición de propiedades visuales y referencias a texturas/shaders.
};

/**
 * @enum    ResourceState
 * @brief   Representa el estado actual del ciclo de vida de un recurso en memoria.
 *
 * Utilizado para el seguimiento de la carga asíncrona y la gestión de memoria
 * dentro del sistema de streaming de recursos.
 */
enum class
	ResourceState {
	Unloaded,   ///< El recurso no está en memoria y solo existe su referencia.
	Loading,    ///< El recurso se encuentra actualmente en proceso de carga o inicialización.
	Loaded,     ///< El recurso está completamente cargado en RAM/VRAM y listo para usarse.
	Failed      ///< Ocurrió un error crítico durante la carga o inicialización del recurso.
};

/**
 * @class   IResource
 * @brief   Interfaz base para todos los recursos gestionables del motor.
 *
 * @details Esta clase define el contrato fundamental para cualquier recurso que deba
 * ser administrado por el sistema central de recursos de MonacoEngine3. Proporciona
 * la base para rastrear la ubicación en disco, el estado de carga, el tipo y un
 * identificador único global para cada instancia de recurso cargada en el juego.
 */
class IResource {
public:
	/**
	 * @brief Constructor inicializador.
	 *
	 * Asigna un nombre descriptivo al recurso y genera automáticamente un
	 * identificador numérico único para su seguimiento.
	 *
	 * @param name Nombre lógico o identificador legible por humanos para el recurso.
	 */
	IResource(const std::string& name): 
			 m_name(name),
			 m_filePath(""),
			 m_type(ResourceType::Unknown),
			 m_state(ResourceState::Unloaded),
			 m_id(GenerateID())
	{
	}

	/**
	 * @brief Destructor virtual por defecto.
	 *
	 * Garantiza la correcta destrucción de los recursos derivados y la liberación
	 * de memoria específica de cada tipo.
	 */
	virtual 
			~IResource() = default;

	/**
	 * @brief Inicializa y crea los buffers o recursos directamente en la GPU.
	 *
	 * Método virtual puro. Se llama tras haber cargado los datos crudos en RAM para
	 * transferirlos a la memoria de video o realizar configuraciones del API gráfico.
	 *
	 * @return @c true si la inicialización gráfica fue exitosa; @c false en caso de error.
	 */
	virtual bool 
				init() = 0;

	/**
	 * @brief Carga los datos del recurso desde el almacenamiento en disco.
	 *
	 * Método virtual puro encargado de la lectura de archivos y la
	 * asignación inicial en la memoria del sistema (RAM).
	 *
	 * @param filename Ruta del archivo desde donde se cargará el recurso.
	 * @return @c true si la lectura y parseo en memoria fue exitosa.
	 */
	virtual bool 
				load(const std::string& filename) = 0;

	/**
	 * @brief Libera toda la memoria y recursos (RAM y VRAM) ocupados.
	 *
	 * Método virtual puro. Debe retornar el estado del recurso a @c Unloaded
	 * y garantizar que no haya fugas de memoria gráfica o del sistema.
	 */
	virtual void 
				unload() = 0;

	/**
	 * @brief Obtiene el tamaño en memoria ocupado por este recurso.
	 *
	 * Método virtual puro fundamental para sistemas de profiling, depuración
	 * y gestión de presupuestos de memoria.
	 *
	 * @return El tamaño total del recurso expresado en bytes.
	 */
	virtual 
		   size_t getSizeInBytes() const = 0;

	/**
	 * @brief Define la ruta física o virtual del archivo fuente.
	 * @param path Ruta absoluta o relativa al directorio de assets.
	 */
	void 
		SetPath(const std::string& path) { m_filePath = path; }

	/**
	 * @brief Clasifica la categoría a la que pertenece este recurso.
	 * @param t Valor del enumerador @c ResourceType correspondiente.
	 */
	void 
		SetType(ResourceType t) { m_type = t; }

	/**
	 * @brief Actualiza la fase del ciclo de vida en la que se encuentra el recurso.
	 * @param s Valor del enumerador @c ResourceState correspondiente.
	 */
	void 
		SetState(ResourceState s) { m_state = s; }

	/**
	 * @brief Recupera el nombre descriptivo del recurso.
	 * @return Referencia constante al nombre lógico de tipo @c std::string.
	 */
	const std::string& GetName() const { return m_name; }

	/**
	 * @brief Recupera la ruta del archivo asociado.
	 * @return Referencia constante a la ruta del disco de tipo @c std::string.
	 */
	const std::string& GetPath() const { return m_filePath; }

	/**
	 * @brief Recupera la clasificación funcional del recurso.
	 * @return El tipo actual definido por @c ResourceType.
	 */
	ResourceType GetType() const { return m_type; }

	/**
	 * @brief Recupera la fase de carga o ciclo de vida actual.
	 * @return El estado actual definido por @c ResourceState.
	 */
	ResourceState GetState() const { return m_state; }

	/**
	 * @brief Obtiene el identificador numérico único y global del recurso.
	 * @return Un entero de 64 bits garantizado de ser único durante la ejecución.
	 */
	uint64_t GetID() const { return m_id; }

protected:
	// ============================================================================
	// Propiedades Generales del Recurso
	// ============================================================================
		  std::string m_name;        ///< Nombre lógico o identificador amigable del recurso.
		  std::string m_filePath;    ///< Ruta completa del archivo físico del que proviene.
		  ResourceType m_type;       ///< Categoría funcional asignada al recurso.
		  ResourceState m_state;     ///< Estado actual de carga y disponibilidad en memoria.
		  uint64_t m_id;             ///< Hash o identificador numérico único autogenerado.

private:
	/**
	 * @brief Genera un identificador secuencial único para cada nueva instancia.
	 *
	 * Mantiene un contador estático interno de 64 bits para asegurar que ningún par
	 * de recursos comparta el mismo identificador durante el ciclo de vida de la aplicación.
	 *
	 * @return Un nuevo número de identificación secuencial (ID).
	 */
	static uint64_t GenerateID()
	{
		static uint64_t nextID = 1;
		return nextID++;
	}
};