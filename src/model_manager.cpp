#include "model_manager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../third_party/tiny_obj_loader/tiny_obj_loader.h"

void ModelManager::load_mesh(const char* path, Mesh& mesh) {

    std::string mesh_file = path;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "../assets";

    tinyobj::ObjReader reader;

    reader.ParseFromFile(mesh_file);
    if (!reader.ParseFromFile(mesh_file, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::unordered_map<Vertex, uint32_t> unique_vertices;

    // Loop over shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            // if (index.texcoord_index >= 0) {
            //     vertex.texture_coords = {
            //         attrib.texcoords[2 * index.texcoord_index + 0],
            //         attrib.texcoords[2 * index.texcoord_index + 1]
            //     };
            // }

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }
            mesh.indices.push_back(unique_vertices[vertex]);
        }
    }
}

void ModelManager::cleanup(Mesh &mesh) {
    glDeleteVertexArrays(1, &mesh.VAO);
    glDeleteBuffers(1, &mesh.VBO);
    glDeleteBuffers(1, &mesh.EBO);
}