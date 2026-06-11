#pragma once

// ============================================================
//  ECS.h — Header de conveniencia
//  Incluye todo el sistema en un solo #include.
//
//  En tu motor:   #include "ECS/ECS.h"
// ============================================================

#include "Types.h"
#include "SparseSet.h"
#include "ComponentPool.h"
#include "View.h"
#include "System.h"
#include "Registry.h"
// Serializer requiere nlohmann/json — inclúyelo por separado
// si no quieres esa dependencia en todos los TUs:
// #include "Serializer.h"
