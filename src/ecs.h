#pragma once

#include <bits/types/FILE.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <bitset>
#include <array>
#include <iostream>
#include <memory>
#include <pstl/glue_execution_defs.h>
#include <queue>
#include <cassert>
#include <set>
#include <unordered_map>


using Entity = uint32_t;
const Entity MAX_ENTITIES{1000000};
inline constexpr Entity INVALID_ENTITY = 0;

using ComponentType = uint8_t;
const ComponentType MAX_COMPONENTS{8};

using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager {
    public:
        EntityManager() {
            for (Entity i = 1; i <= MAX_ENTITIES; i++) {
                available_entities.push(i);
            }
        }

        Entity create_entity() {
            assert(!available_entities.empty() && "No available entities");

            Entity entity = available_entities.front();
            available_entities.pop();
            return entity;
        }

        void destroy_entity(Entity entity) {
            entity_signature[entity].reset();
            available_entities.push(entity);
        }

        void set_signature(Entity entity, Signature signature) {
            entity_signature[entity] = signature;
        }

        Signature get_signature(Entity entity) {
            return entity_signature[entity];
        }

        void print_available_entities(){
            std::queue<Entity> q_copy = available_entities;

            while(!q_copy.empty()) {
                printf("%i,",q_copy.front());
                q_copy.pop();
            }
            printf("\n");
        }

    private:
        std::queue<Entity> available_entities{};
        std::array<Signature, MAX_ENTITIES> entity_signature{};

};

class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void entity_destroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
    public:
        void link_data(Entity entity, T component) {
            components[size] = component;
            index_to_entity[size] = entity; // min(entity) == 1
            entity_to_index[entity] = size;
            size++;
        }

        void unlink_data(Entity entity) {
            size_t unlinked_index = entity_to_index[entity];
            Entity last_entity = index_to_entity[size - 1];
            size_t last_entity_index = entity_to_index[last_entity];

            components[unlinked_index] = components[last_entity_index];
            index_to_entity[unlinked_index] = last_entity;
            entity_to_index[last_entity] = unlinked_index;

            entity_to_index.erase(entity);
            index_to_entity.erase(last_entity_index);
            components[size - 1] = T{};
            size--;

        }

        T& get_data(Entity entity) {
            assert(entity_to_index.find(entity) != entity_to_index.end() && "Trying to get data for unlinked enitty");
            return components[entity_to_index[entity]];
        }

        void entity_destroyed(Entity entity) override {
            if (entity_to_index.find(entity) != entity_to_index.end()) {
                unlink_data(entity);
            }
        }

        void print_info() {
            printf("-----\n");
            printf("Components:\n");
            for (T component : components) {
               printf("%i,", component.value);
            }
            printf("\n");
            printf("Entity to Index:\n");
            for (const auto &pair : entity_to_index) {
                printf("%i -> %li\t", pair.first, pair.second);
            }
            printf("\n");
            printf("Index to Entity:\n");
            for (const auto &pair: index_to_entity) {
                printf("%li -> %i\t", pair.first, pair.second);
            }
            printf("\n");
        }

    private:
        std::array<T, MAX_ENTITIES> components{};
        std::unordered_map<Entity, size_t> entity_to_index{};
        std::unordered_map<size_t, Entity> index_to_entity{};
        size_t size{};
};

class ComponentManager {
    public:
        template<typename T>
        void register_component() {
            const char* type_name = typeid(T).name();

            assert(component_types.end() == component_types.find(type_name) && "Registered type matches more than once");

            component_types.insert({type_name, next_component_type});

            component_arrays.insert({type_name, std::make_shared<ComponentArray<T>>()});

            next_component_type++;
        }

        template<typename T>
        ComponentType get_component_type() {
            const char* type_name = typeid(T).name();
            return component_types[type_name];
        }

        template<typename T>
        void add_component(Entity entity, T component) {
            get_component_array<T>()->link_data(entity, component);
        }

        template<typename T>
        void remove_component(Entity entity) {
            get_component_array<T>()->unlink_data(entity);
        }

        template<typename T>
        T& get_component(Entity entity) {
            return get_component_array<T>()->get_data(entity);
        }

        void entity_destroyed(Entity entity) {
            for (auto const &pair : component_arrays) {
                auto const &component = pair.second;
                component->entity_destroyed(entity);
            }
        } 

    private:
        std::unordered_map<const char*, ComponentType> component_types{};
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> component_arrays{};
        ComponentType next_component_type{};

        template<typename T>
        std::shared_ptr<ComponentArray<T>> get_component_array() {
            const char* type_name = typeid(T).name();

            assert(component_types.find(type_name) != component_types.end() && "Component not registered before use.");

            return std::static_pointer_cast<ComponentArray<T>>(component_arrays[type_name]);
        }
};

class Coordinator;

struct Update {
    float delta;
};


class System {
    public:
        std::set<Entity> entities{};
        virtual void update(Update update){};
        Coordinator* coordinator;
};



class SystemManager {
    public:
        template<typename T>
        std::shared_ptr<T> register_system() {
            const char* type_name = typeid(T).name();
            auto system = std::make_shared<T>();
            systems.insert({type_name, system});
            return system;
        }

        template<typename T>
        void set_signature(Signature signature) {
            const char* type_name = typeid(T).name();
            signatures[type_name] = signature;
        }

        void entity_destroyed(Entity entity) {
            for (auto const &pair : systems) {
                auto const &system = pair.second;

                system->entities.erase(entity);
            }
        }

        void entity_signature_change(Entity entity, Signature entity_signature) {
            for (auto const &pair : systems) {
                auto const &type = pair.first;
                auto const &system = pair.second;
                auto const &system_signature = signatures[type];

                if ((entity_signature & system_signature) == system_signature) {
                    system->entities.insert(entity);
                }
                else {
                    system->entities.erase(entity);
                }
            } 
        }


    private:
        std::unordered_map<const char *, Signature> signatures{};
        std::unordered_map<const char *, std::shared_ptr<System>> systems{};
};

class Coordinator {
    public:
        void init() {
            entity_manager = std::make_unique<EntityManager>();
            component_manager = std::make_unique<ComponentManager>();
            system_manager = std::make_unique<SystemManager>();
        }

        Entity create_entity() {
            return entity_manager->create_entity();
        }

        void destroy_entity(Entity entity) {
            entity_manager->destroy_entity(entity);
            component_manager->entity_destroyed(entity);
            system_manager->entity_destroyed(entity);
        }

        template<typename T>
        void register_component() {
            component_manager->register_component<T>();
        }

        template<typename T>
        void add_component(Entity entity, T component) {
            component_manager->add_component<T>(entity, component);

            auto signature = entity_manager->get_signature(entity);
            signature.set(component_manager->get_component_type<T>(), true);
            entity_manager->set_signature(entity, signature);

            system_manager->entity_signature_change(entity, signature);
        }

        template<typename T>
        void remove_component(Entity entity) {
            component_manager->remove_component<T>(entity);    
            
            auto signature = entity_manager->get_signature(entity);
            signature.set(component_manager->get_component_type<T>(), false);
            entity_manager->set_signature(entity, signature);

            system_manager->entity_signature_change(entity, signature);
        }

        template<typename T>
        T& get_component(Entity entity) {
            return component_manager->get_component<T>(entity);
        }

        template<typename T>
        ComponentType get_component_type() {
            return component_manager->get_component_type<T>();
        }

        template<typename T>
        std::shared_ptr<T> register_system() {
            return system_manager->register_system<T>();
        }

        template<typename T>
        void set_system_signature(Signature signature) {
            system_manager->set_signature<T>(signature);
        }

    private:
        std::unique_ptr<EntityManager> entity_manager;
        std::unique_ptr<ComponentManager> component_manager;
        std::unique_ptr<SystemManager> system_manager;
};
