#pragma once

/**
 * @file    Prerequisites.h
 * @brief   Cabecera central de dependencias, macros y estructuras fundamentales del motor.
 *
 * @details Este archivo concentra las inclusiones de la biblioteca estándar,
 * los encabezados principales de DirectX 11 y las utilidades de memoria/matemáticas
 * del motor (EngineUtilities). Además, define las macros de depuración, los
 * formatos de vértices base y las estructuras de los Constant Buffers (CB)
 * que se comunican directamente con los shaders en la GPU.
 */

 // Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <array>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities\Memory\TSharedPointer.h"
#include "EngineUtilities\Memory\TWeakPointer.h"
#include "EngineUtilities\Memory\TStaticPtr.h"
#include "EngineUtilities\Memory\TUniquePtr.h"

// ============================================================================
// MACROS DE UTILIDAD
// ============================================================================

/**
 * @def     SAFE_RELEASE(x)
 * @brief   Libera de forma segura objetos COM (Component Object Model) de DirectX.
 * @param x Puntero al objeto a liberar.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @def     MESSAGE(classObj, method, state)
  * @brief   Macro de depuración para registrar el estado de creación de recursos.
  * @param classObj Nombre de la clase o módulo que emite el mensaje.
  * @param method   Método desde el cual se llama.
  * @param state    Estado o mensaje descriptivo.
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def     ERROR(classObj, method, errorMSG)
   * @brief   Macro de depuración para registrar errores críticos de forma segura.
   * @param classObj Nombre de la clase donde ocurrió el error.
   * @param method   Método donde ocurrió el error.
   * @param errorMSG Descripción del error.
   */
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

   //--------------------------------------------------------------------------------------
   // Structures (Formatos de Vértices y Constant Buffers)
   //--------------------------------------------------------------------------------------

   /**
    * @struct  SimpleVertex
    * @brief   Estructura estándar de vértice para mallas 3D completas.
    */
struct SimpleVertex
{
       EU::Vector3 Position;             ///< Coordenadas espaciales del vértice.
       EU::Vector3 Normal;               ///< Vector perpendicular para cálculos de iluminación.
       EU::Vector3 Tangent;              ///< Vector tangente para normal mapping.
       EU::Vector3 Bitangent;            ///< Vector bitangente para normal mapping.
       EU::Vector2 TextureCoordinate;    ///< Coordenadas UV para mapeo de texturas.
};

/**
 * @struct  SkyboxVertex
 * @brief   Estructura simplificada de vértice exclusiva para proyecciones de entorno (Skybox).
 */
struct
      SkyboxVertex {
      float x, y, z;                      ///< Coordenadas espaciales básicas.
};

/**
 * @struct  CBNeverChanges
 * @brief   Constant Buffer para datos que raramente cambian durante la ejecución.
 */
struct CBNeverChanges
{
       XMMATRIX mView;                   ///< Matriz de vista (View Matrix).
};

/**
 * @struct  CBSkybox
 * @brief   Constant Buffer específico para la renderización del Skybox.
 */
struct CBSkybox
{
       XMMATRIX mviewProj;               ///< Matriz combinada de Vista y Proyección sin traslación.
};

/**
 * @struct  CBChangeOnResize
 * @brief   Constant Buffer para datos que deben actualizarse al redimensionar la ventana.
 */
struct CBChangeOnResize
{
       XMMATRIX mProjection;             ///< Matriz de proyección en perspectiva.
};

/**
 * @struct  CBMain
 * @brief   Constant Buffer principal utilizado en los Vertex y Pixel shaders.
 * @note    Alineado estrictamente a bloques de 16 bytes según los requerimientos de Direct3D.
 */
 // Constant buffer used in the vertex and pixel shaders.  Align to
 // 16 bytes as required by Direct3D constant buffers.
struct CBMain
{
        //XMFLOAT4X4 World;
        XMFLOAT4X4 View;                  ///< Matriz de vista actual.
        XMFLOAT4X4 Projection;            ///< Matriz de proyección actual.
        EU::Vector3 CameraPos;            ///< Posición absoluta de la cámara en el mundo.
        float pad0;                       ///< Padding de 4 bytes para alineación a 16 bytes.
        EU::Vector3 LightDir;             ///< Vector direccional de la luz principal.
        float pad1;                       ///< Padding de 4 bytes para alineación a 16 bytes.
        EU::Vector3 LightColor;           ///< Color e intensidad de la luz principal.
        float pad2;                       ///< Padding de 4 bytes para alineación a 16 bytes.
};

/**
 * @struct  CBChangesEveryFrame
 * @brief   Constant Buffer para transformaciones individuales de cada entidad por frame.
 */
struct CBChangesEveryFrame
{
    XMMATRIX mWorld;                  ///< Matriz de transformación (Mundo) de la malla actual.
    XMFLOAT4 vMeshColor;              ///< Color base difuso aplicado a la malla.
};

// ============================================================================
// ENUMERACIONES
// ============================================================================

/**
 * @enum    ExtensionType
 * @brief   Identifica los formatos de textura soportados por el motor.
 */
enum ExtensionType {
     DDS = 0,  ///< Formato DirectDraw Surface, optimizado para GPU.
     PNG = 1,  ///< Formato Portable Network Graphics (con canal alfa).
     JPG = 2   ///< Formato JPEG, comprimido con pérdida (sin canal alfa).
};

/**
 * @enum    ShaderType
 * @brief   Define las distintas etapas programables del pipeline gráfico de D3D11.
 */
enum ShaderType {
     VERTEX_SHADER = 0,    ///< Programa encargado de procesar y transformar vértices.
     PIXEL_SHADER = 1      ///< Programa encargado de calcular el color final de los píxeles.
};

/**
 * @enum    ComponentType
 * @brief   Identificadores únicos para el sistema Entity-Component-System (ECS).
 * * @details Permite el casteo seguro y la búsqueda rápida de componentes dentro de un @c Actor o @c Entity.
 */
enum
    ComponentType {
    NONE = 0,      ///< Tipo de componente nulo o no especificado.
    TRANSFORM = 1, ///< Componente de transformación (posición, rotación, escala).
    MESH = 2,      ///< Componente geométrico (vértices e índices).
    MATERIAL = 3,  ///< Componente de propiedades de superficie (texturas, shaders).
    HIERARCHY = 4  ///< Componente de relaciones en el Scene Graph (padre/hijo).
};