/**
* Created by Modar Nasser on 15/04/2020.
**/


#pragma once

#include <memory>
#include "NasNas/data/Drawable.hpp"
#include "NasNas/ecs/ComponentGroup.hpp"

namespace ns {

    class BaseEntity : public ecs::ComponentGroup {
    public:
        explicit BaseEntity(std::string name);

        auto transform() const -> ecs::Transform*;
        auto inputs() const -> ecs::Inputs*;
        auto physics() const -> ecs::Physics*;
        auto collider() const -> ecs::Collider*;

    };

}
