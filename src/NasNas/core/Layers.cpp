/**
* Created by Modar Nasser on 15/04/2020.
**/


#include "NasNas/core/Layers.hpp"

using namespace ns;

Layer::Layer(const std::string& name) {
    m_name = name;
}

void Layer::add(const std::shared_ptr<sf::Shape>& drawable) {
    m_drawables.emplace_back(drawable);
}

void Layer::add(const std::shared_ptr<sf::Text>& drawable) {
    m_drawables.emplace_back(drawable);
}

void Layer::add(const std::shared_ptr<ns::Drawable>& drawable) {
    m_drawables.emplace_back(drawable);
}

void Layer::ySort() {
    std::sort(std::begin(m_drawables), std::end(m_drawables),
              [](auto const& lhs, auto const& rhs){
            return std::visit([](auto const& s) { return s->getPosition().y; }, lhs)
                   <std::visit([](auto const& s) { return s->getPosition().y; }, rhs);
        }
    );
}

auto Layer::getName() -> const std::string& {
    return m_name;
}

auto Layer::getDrawables() -> std::vector<LayerDrawablesTypes>& {
    return m_drawables;
}
