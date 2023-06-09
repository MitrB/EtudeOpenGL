#pragma once

#include "octreeManager.hpp"
namespace etudegl {
class Scene {
public:
    OctManager octManager{};
    void init();
};
}  // namespace etudegl