/**
* Created by Modar Nasser on 26/04/2020.
**/

#pragma once


#include "../NasNas.h"

class Game: public ns::App {
public:
    Game();
    ~Game();

    void onEvent(sf::Event event) override ;
    void update() override ;

private:
    ns::Scene* scene;
    ns::Camera* game_camera;
    std::shared_ptr<ns::BaseEntity> player;
    ns::tm::TiledMap tiled_map;
    int frame_counter = 0;
};
