#pragma once

#include "structs.hpp"
#include <iostream>
#include <glad/glad.h>

class ModelManager {
    public:
        static ModelManager& get_instance() {
            static ModelManager model_manager;
            return model_manager;
        }

        static void load_mesh(const char* path, Mesh& mesh);
        static void cleanup(Mesh& mesh);
    
    private:

};