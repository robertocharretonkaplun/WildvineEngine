#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum    ModelType
 * @brief   Define los formatos de archivo de modelos 3D soportados por el motor.
 */
enum
	ModelType {
	OBJ, ///< Formato Wavefront OBJ, ideal para mallas estáticas simples.
	FBX  ///< Formato Autodesk FBX, soporta jerarquías, materiales y datos complejos.
};

/**
 * @class   Model3D
 * @brief   Representa un recurso de modelo tridimensional cargado en memoria.
 *
 * La clase Model3D hereda de IResource y actúa como un contenedor
 * principal para la geometría de un objeto. Se encarga de parsear archivos físicos
 * (como OBJ o FBX) a través de SDKs externos y convertirlos en una colección de
 * instancias de @c MeshComponent. También provee constructores para la generación
 * de mallas programáticas estandarizadas (como el Skybox).
 */
class
	Model3D : public IResource {
public:
	/**
	 * @brief Constructor para cargar un modelo desde un archivo físico.
	 *
	 * Inicializa el recurso base, asigna el tipo de modelo y dispara automáticamente
	 * la rutina de carga y parseo desde el disco.
	 *
	 * @param name      Ruta o nombre identificador del archivo.
	 * @param modelType El formato del archivo especificado en @c ModelType.
	 */
	Model3D(const std::string& name, ModelType modelType): 
		    IResource(name), 
			m_modelType(modelType), 
			lSdkManager(nullptr), 
			lScene(nullptr) 
	{
			SetType(ResourceType::Model3D);
			load(name);
	}

	/**
	 * @brief Constructor programático para modelos de entorno (Skybox).
	 *
	 * Crea un modelo tridimensional directamente a partir de arreglos en memoria
	 * en lugar de leer un archivo físico. Configura internamente un @c MeshComponent
	 * con vértices especializados para la proyección de fondos.
	 *
	 * @param name     Nombre identificador para el recurso.
	 * @param vertices Arreglo fijo de 8 vértices tipo @c SkyboxVertex.
	 * @param indices  Arreglo fijo de 36 índices que definen la topología cúbica.
	 */
	Model3D(const std::string& name,
			const SkyboxVertex vertices[],
			const unsigned int indices[]) : IResource(name) {
			MeshComponent mesh;
			mesh.m_skyVertex.assign(vertices, vertices + 8);
			mesh.m_index.assign(indices, indices + 36);
			mesh.m_numIndex = mesh.m_index.size();
			SetType(ResourceType::Model3D);
			m_meshes.push_back(mesh);
	}

	/**
	 * @brief Destructor por defecto.
	 */
	~Model3D() = default;

	/**
	 * @brief Lee y parsea el archivo físico del modelo tridimensional.
	 *
	 * Método heredado de @c IResource. Dependiendo de @c m_modelType, delega
	 * el procesamiento a la rutina correspondiente (ej. LoadFBXModel).
	 *
	 * @param path Ruta absoluta o relativa al archivo del modelo.
	 * @return     @c true si la carga y el parseo fueron exitosos.
	 */
	bool
		load(const std::string& path) override;

	/**
	 * @brief Inicializa los recursos de hardware asociados al modelo.
	 *
	 * Método heredado de @c IResource. Invoca la inicialización de los buffers
	 * en cada uno de los @c MeshComponent administrados.
	 *
	 * @return @c true si la inicialización en GPU fue exitosa.
	 */
	bool
		init() override;

	/**
	 * @brief Libera la memoria consumida por las mallas del modelo.
	 *
	 * Método heredado de @c IResource. Descarga la geometría y libera estructuras
	 * asociadas al SDK de importación.
	 */
	void
		unload() override;

	/**
	 * @brief Calcula la huella de memoria del modelo.
	 *
	 * Método heredado de @c IResource. Suma el tamaño de todos los vértices,
	 * índices y estructuras auxiliares alojadas en RAM.
	 *
	 * @return El tamaño total del recurso en bytes.
	 */
	size_t
		  getSizeInBytes() const override;

	/**
	 * @brief Obtiene las mallas geométricas que componen este modelo.
	 * @return Referencia constante a la colección de @c MeshComponent.
	 */
	const std::vector<MeshComponent>&
			   GetMeshes() const { return m_meshes; }

	// ============================================================================
	// Integración con el SDK de Autodesk FBX
	// ============================================================================
	/* FBX MODEL LOADER*/

	/**
	 * @brief Inicializa el administrador de memoria y recursos del Autodesk FBX SDK.
	 * @return @c true si el SDK se inicializó correctamente.
	 */
	bool
		InitializeFBXManager();

	/**
	 * @brief Carga y procesa la escena completa desde un archivo FBX.
	 *
	 * @param filePath Ruta del archivo FBX a importar.
	 * @return Colección de @c MeshComponent extraída de los nodos del archivo.
	 */
	std::vector<MeshComponent>
		 LoadFBXModel(const std::string& filePath);

	/**
	 * @brief Procesa de manera recursiva un nodo del árbol del Scene Graph de FBX.
	 *
	 * @param node Puntero al nodo de la escena FBX actualmente en evaluación.
	 */
	void
		ProcessFBXNode(FbxNode* node);

	/**
	 * @brief Extrae los vértices, normales, UVs e índices de una malla FBX.
	 *
	 * Convierte la topología nativa del SDK de Autodesk al formato interno @c MeshComponent
	 * del motor.
	 *
	 * @param node Puntero al nodo FBX que contiene los atributos de geometría.
	 */
	void
		ProcessFBXMesh(FbxNode* node);

	/**
	 * @brief Extrae y asocia la información de los materiales y texturas.
	 *
	 * @param material Puntero a la superficie del material definida en el archivo FBX.
	 */
	void
		ProcessFBXMaterials(FbxSurfaceMaterial* material);

	/**
	 * @brief Obtiene la lista de texturas referenciadas por el modelo FBX.
	 * @return Colección de nombres de archivo de texturas.
	 */
		std::vector<std::string>
			GetTextureFileNames() const { return textureFileNames; }

private:
		// ============================================================================
		// Recursos de Importación de FBX
		// ============================================================================
		FbxManager* lSdkManager;                        ///< Administrador central del SDK de FBX.
		FbxScene* lScene;                               ///< Contenedor de la escena tridimensional importada por el SDK.
		std::vector<std::string> textureFileNames;      ///< Rutas a las texturas extraídas de los materiales del modelo.

public:
		// ============================================================================
		// Estructura Interna
		// ============================================================================
		ModelType m_modelType;                          ///< Identificador del formato origen de este modelo.
		std::vector<MeshComponent> m_meshes;            ///< Colección de mallas (geometría) generadas a partir del modelo base.
};