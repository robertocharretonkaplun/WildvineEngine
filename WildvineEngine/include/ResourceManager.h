#pragma once
#include "Prerequisites.h"
#include "IResource.h"

/**
 * @class   ResourceManager
 * @brief   Administrador centralizado de recursos del motor.
 *
 * Implementa el patrón Singleton para garantizar un punto de acceso
 * global y único. Utiliza principios del patrón Flyweight para asegurar que
 * los recursos pesados (modelos 3D, texturas, shaders, etc.) se carguen en
 * memoria una única vez. Previene la duplicación de datos compartiendo
 * punteros inteligentes entre todas las entidades que solicitan el mismo archivo,
 * optimizando así el uso de RAM/VRAM en MonacoEngine3.
 */
class
	ResourceManager {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	ResourceManager() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~ResourceManager() = default;

	// Singleton
	/**
	 * @brief Obtiene la instancia única global del administrador de recursos.
	 *
	 * Utiliza el patrón Singleton de Meyer, garantizando una inicialización
	 * segura e inmediata en el primer uso (thread-safe en C++11 en adelante).
	 *
	 * @return Referencia estática a la instancia de @c ResourceManager.
	 */
	static ResourceManager& getInstance() {
			static ResourceManager instance;
			return instance;
	}

	/** @brief Elimina el constructor de copia para forzar la unicidad del Singleton. */
	ResourceManager(const ResourceManager&) = delete;
	/** @brief Elimina el operador de asignación para forzar la unicidad del Singleton. */
	ResourceManager& operator=(const ResourceManager&) = delete;

	/**
	 * @brief Obtiene o carga un recurso de tipo T (T debe heredar de IResource).
	 *
	 * Busca en el caché interno usando la clave proporcionada. Si el recurso ya fue
	 * cargado previamente, devuelve su puntero compartido (Flyweight). Si no existe,
	 * crea una nueva instancia de tipo @c T, ejecuta su método @c load, seguido de su
	 * método @c init, y finalmente lo almacena en el caché para futuros accesos.
	 *
	 * @tparam T        Clase derivada de @c IResource que se desea obtener o instanciar.
	 * @tparam Args     Tipos de los argumentos variables para el constructor de @c T.
	 * @param key       Identificador lógico único (ej. nombre del recurso) usado como llave en el diccionario.
	 * @param filename  Ruta del archivo físico a cargar desde el disco.
	 * @param args      Parámetros adicionales enviados directamente al constructor del recurso.
	 * @return          Un puntero compartido (@c std::shared_ptr) al recurso listo para usarse,
	 * o @c nullptr si la carga o inicialización fallaron.
	 */
	 /// Obtener o cargar un recurso de tipo T (T debe heredar de IResource).
	template<typename T, typename... Args>
	std::shared_ptr<T> GetOrLoad(const std::string& key,
		const std::string& filename,
		Args&&... args) {
		static_assert(std::is_base_of<IResource, T>::value,
			"T debe heredar de IResource");
		// 1. ¿Ya existe el recurso en el caché?
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			// Intentar castear al tipo correcto
			auto existing = std::dynamic_pointer_cast<T>(it->second);
			if (existing && existing->GetState() == ResourceState::Loaded) {
				return existing; // Flyweight: reutilizamos la instancia
			}
		}

		// 2. No existe o no está cargado -> crearlo y cargarlo
		std::shared_ptr<T> resource = std::make_shared<T>(key, std::forward<Args>(args)...);

		if (!resource->load(filename)) {
			// Puedes manejar errores más fino aquí
			return nullptr;
		}

		if (!resource->init()) {
			return nullptr;
		}

		// 3. Guardar en el caché y devolver
		m_resources[key] = resource;
		return resource;
	}

	/**
	 * @brief Obtiene un recurso ya cargado de la caché.
	 *
	 * Ideal para la fase de juego (gameplay loop) donde no se desea realizar
	 * accesos lentos a disco. Si el recurso no fue cargado previamente, devuelve un puntero nulo.
	 *
	 * @tparam T   Tipo del recurso esperado.
	 * @param key  Identificador lógico único del recurso a buscar.
	 * @return     Puntero compartido al recurso si fue encontrado; @c nullptr en caso contrario.
	 */
	 /// Obtener un recurso ya cargado, sin cargarlo si no existe.
	template<typename T>
	std::shared_ptr<T> Get(const std::string& key) const
	{
		auto it = m_resources.find(key);
		if (it == m_resources.end()) return nullptr;

		return std::dynamic_pointer_cast<T>(it->second);
	}

	/**
	 * @brief Libera de la memoria un recurso específico.
	 *
	 * Invoca el método @c unload del recurso y lo elimina completamente del
	 * diccionario interno del administrador.
	 *
	 * @param key Identificador lógico único del recurso a liberar.
	 */
	 /// Liberar un recurso específico
	void Unload(const std::string& key)
	{
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			it->second->unload();
			m_resources.erase(it);
		}
	}

	/**
	 * @brief Descarga absolutamente todos los recursos almacenados.
	 *
	 * Útil al cerrar el motor o al transicionar entre niveles completamente
	 * diferentes para garantizar que la memoria se limpie por completo.
	 */
	 /// Liberar todos los recursos
	void UnloadAll()
	{
		for (auto& [key, res] : m_resources) {
			if (res) {
				res->unload();
			}
		}
		m_resources.clear();
	}

private:
		// ============================================================================
		// Estructuras de Almacenamiento Interno
		// ============================================================================
		/**
		 * @brief Caché principal de recursos del motor.
		 *
		 * Mapea identificadores de cadena (strings) a punteros polimórficos de la clase
		 * base @c IResource. Utiliza una tabla hash para lograr tiempos de acceso de O(1).
		 */
		std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;
};