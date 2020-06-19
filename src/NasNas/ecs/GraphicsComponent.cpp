/**
* Created by Modar Nasser on 10/06/2020.
**/

#include "NasNas/ecs/BaseEntity.hpp"
#include "NasNas/ecs/GraphicsComponent.hpp"

using namespace ns::ecs;

GraphicsComponent::GraphicsComponent(BaseEntity* entity) : BaseComponent(entity){
    m_entity->m_graphics_components_list.push_back(this);
}

