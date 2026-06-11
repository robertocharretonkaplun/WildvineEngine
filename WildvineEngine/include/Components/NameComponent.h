/**
 * @file NameComponent.h
 * @brief Componente de nombre identificador para el ECS.
 * @ingroup components
 */
#pragma once
#include <string>

/**
 * @struct NameComponent
 * @brief Nombre visible de una entidad en el editor (outliner, inspector).
 */
struct
NameComponent {
	NameComponent() = default;
	explicit NameComponent(const std::string& n) : name(n) {}

	std::string name = "Entity";
};
