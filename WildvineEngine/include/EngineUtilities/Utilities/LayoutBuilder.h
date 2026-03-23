#pragma once
#include "Prerequisites.h"

/**
 * @class   LayoutBuilder
 * @brief   Constructor fluido (Builder) para la creación de Input Layouts en DirectX 11.
 *
 * La clase LayoutBuilder facilita la definición estructurada de los
 * formatos de vértice (Input Elements) requeridos por el pipeline gráfico de D3D11.
 * Utiliza el patrón de diseño Builder devolviendo una referencia a sí misma en
 * sus métodos de inserción, permitiendo encadenar múltiples llamadas de forma
 * limpia y legible (Fluent Interface) a la hora de compilar los shaders.
 */
class LayoutBuilder
{
public:
    /**
     * @brief Añade un nuevo elemento al descriptor de formato de vértice (Input Layout).
     *
     * Permite definir atributos genéricos, operando por defecto con datos por vértice
     * (Per-Vertex Data). Devuelve la referencia al objeto para permitir encadenamiento.
     *
     * @param semantic          Nombre de la semántica en el shader (ej. "POSITION", "TEXCOORD").
     * @param format            Formato de los datos usando enumeradores de DXGI (ej. DXGI_FORMAT_R32G32B32_FLOAT).
     * @param semanticIndex     Índice numérico de la semántica (para distinguir múltiples atributos del mismo tipo).
     * @param inputSlot         Ranura del buffer de entrada (Input Slot) del cual se leerán los datos.
     * @param alignedByteOffset Desplazamiento en bytes desde el inicio del vértice (D3D11_APPEND_ALIGNED_ELEMENT por defecto).
     * @param slotClass         Clasificación de los datos: por vértice o por instancia.
     * @param instanceStepRate  Frecuencia de avance de los datos de instancia (0 para datos por vértice).
     * @return                  Referencia a @c LayoutBuilder para permitir llamadas encadenadas.
     */
     // **Add() base** (per-vertex por defecto)
    LayoutBuilder& Add(
                       const char* semantic,
                       DXGI_FORMAT format,
                       UINT semanticIndex = 0,
                       UINT inputSlot = 0,
                       UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                       D3D11_INPUT_CLASSIFICATION slotClass = D3D11_INPUT_PER_VERTEX_DATA,
                       UINT instanceStepRate = 0)
    {
                       D3D11_INPUT_ELEMENT_DESC d{};
                       d.SemanticName = semantic;
                       d.SemanticIndex = semanticIndex;
                       d.Format = format;
                       d.InputSlot = inputSlot;
                       d.AlignedByteOffset = alignedByteOffset;
                       d.InputSlotClass = slotClass;
                       d.InstanceDataStepRate = instanceStepRate;
                       m_elems.push_back(d);
    return *this;
    }

    /**
     * @brief Añade un elemento específico para renderizado por instancias (Instancing).
     *
     * Método de conveniencia que configura automáticamente la clasificación a
     * @c D3D11_INPUT_PER_INSTANCE_DATA y asigna valores predeterminados comunes
     * para el slot y el ratio de avance.
     *
     * @param semantic          Nombre de la semántica en el shader (ej. "TEXCOORD" o "TRANSFORM").
     * @param format            Formato de los datos usando enumeradores de DXGI.
     * @param semanticIndex     Índice numérico de la semántica.
     * @param inputSlot         Ranura del buffer de entrada dedicada a los datos de instancia (1 por defecto).
     * @param alignedByteOffset Desplazamiento en bytes.
     * @param instanceStepRate  Número de instancias a dibujar antes de avanzar un elemento en el buffer (1 por defecto).
     * @return                  Referencia a @c LayoutBuilder para encadenar llamadas.
     */
     // **Atajo** para instancing
    LayoutBuilder& AddInstance(
                                const char* semantic,
                                DXGI_FORMAT format,
                                UINT semanticIndex = 0,
                                UINT inputSlot = 1,
                                UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                                UINT instanceStepRate = 1)
    {
        return Add(semantic, format, semanticIndex, inputSlot, alignedByteOffset,
                   D3D11_INPUT_PER_INSTANCE_DATA, instanceStepRate);
    }

    /**
     * @brief Obtiene la colección completa de descriptores construidos.
     * @return Referencia constante al vector interno de @c D3D11_INPUT_ELEMENT_DESC.
     */
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Get() const { return m_elems; }

    /**
     * @brief Obtiene el número total de elementos registrados en el Layout.
     * @return La cantidad de descriptores de entrada (Input Elements).
     */
    UINT Count() const { return (UINT)m_elems.size(); }

private:
    // ============================================================================
    // Estructuras de Almacenamiento Interno
    // ============================================================================
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems; ///< Colección de descriptores de elementos de entrada en formato nativo de DirectX 11.
};