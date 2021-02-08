// Created by Modar Nasser on 06/02/2021.

#pragma once

#include "NasNas/data/Drawable.hpp"
#include "NasNas/ecs/BaseComponent.hpp"
#include "NasNas/ecs/GraphicsComponent.hpp"
#include "NasNas/ecs/TransformComponent.hpp"
#include "NasNas/ecs/PhysicsComponent.hpp"
#include "NasNas/ecs/InputsComponent.hpp"
#include "NasNas/ecs/ColliderComponent.hpp"

namespace ns::ecs {

    class ComponentGroup : public ns::Drawable {
    public:
        explicit ComponentGroup(std::string name);
        ~ComponentGroup() override;

        template <typename T, typename... TArgs>
        void add(TArgs... component_args);

        template <typename T>
        void add(T* new_component);

        template <typename T>
        auto get() const -> T*;

        void addChild(const std::string& name);
        auto getChild(const std::string& name) const -> ComponentGroup*;

        auto getParent() const -> ComponentGroup*;

        virtual void update();

        auto getPosition() const -> sf::Vector2f override;
        auto getGlobalBounds() const -> ns::FloatRect override;

    protected:
        auto graphics() -> const std::vector<GraphicsComponent*>&;

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        std::string m_name;

        std::unordered_map<int, BaseComponent*> m_components;
        std::vector<GraphicsComponent*> m_graphics;

        std::map<std::string, ComponentGroup*> m_childs;
        ComponentGroup* m_parent = nullptr;

    };

    template<class T, typename... TArgs>
    void ComponentGroup::add(TArgs... component_args) {
        add(new T(std::forward<TArgs>(component_args)...));
    }

    template<typename T>
    void ComponentGroup::add(T* new_component) {
        new_component->m_owner = this;
        m_components[T::getId()] = new_component;
        if constexpr (std::is_base_of_v<GraphicsComponent, T>)
            m_graphics.push_back(new_component);
    }

    template <typename T>
    auto ComponentGroup::get() const -> T* {
        if (m_components.count(T::getId()))
            return static_cast<T*>(m_components.at(T::getId()));
        return nullptr;
    }

}
