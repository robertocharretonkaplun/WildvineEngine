/**
 * @file Transform.h
 * @brief Componente de transformacion (dato puro) para el ECS.
 * @ingroup components
 */
#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"

/**
 * @struct Transform
 * @brief Posicion, rotacion y escala de una entidad.
 *
 * Componente de datos del ECS (sin herencia ni logica de frame).
 * La matriz local se recompone con @c updateMatrix() y la matriz de mundo
 * la propaga el @c SceneGraph a partir de la jerarquia.
 */
struct
Transform {
  Transform() : position(0.0f, 0.0f, 0.0f),
                rotation(0.0f, 0.0f, 0.0f),
                scale(1.0f, 1.0f, 1.0f),
                matrix(XMMatrixIdentity()),
                worldMatrix(XMMatrixIdentity()) {}

  // Recompone la matriz local en el orden: scale -> rotation -> translation
  void
  updateMatrix() {
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
    worldMatrix = matrix;
  }

  const EU::Vector3&
  getPosition() const { return position; }

  void
  setPosition(const EU::Vector3& newPos) { position = newPos; }

  const EU::Vector3&
  getRotation() const { return rotation; }

  void
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }

  const EU::Vector3&
  getScale() const { return scale; }

  void
  setScale(const EU::Vector3& newScale) { scale = newScale; }

  void
  setTransform(const EU::Vector3& newPos,
               const EU::Vector3& newRot,
               const EU::Vector3& newSca) {
    position = newPos;
    rotation = newRot;
    scale = newSca;
  }

  EU::Vector3 position;  // Posicion del objeto
  EU::Vector3 rotation;  // Rotacion del objeto (radianes)
  EU::Vector3 scale;     // Escala del objeto

  XMMATRIX matrix;       // Matriz de transformacion local
  XMMATRIX worldMatrix;  // Matriz de transformacion world
};
