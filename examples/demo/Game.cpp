/**
* Created by Modar Nasser on 26/04/2020.
**/

#include <cmath>
#include "Game.hpp"


Game::Game() :
ns::App("NasNas demo", {640, 360}, 2, 60, 60) {
    //------------ Game Objects creation ------------------------------------------------
    // load tiled map from file
    this->tiled_map.loadFromFile("assets/test_map.tmx");
    for (auto& rect : this->tiled_map.getGroupLayer("Group 1").getGroupLayer("Group 2").getObjectLayer("collisions").allRectangles()) {
        auto ent = ns::Ecs.create();
        auto& coll = ns::Ecs.attach<ns::ecs::AABBCollider>(ent);
        coll.size = {rect.width, rect.height};
        coll.position = {rect.x + rect.width/2.f, rect.y + rect.height/2.f};
    }

    // generate 100 random octogons
    auto colors = std::vector<sf::Color>{
        sf::Color::Blue, sf::Color::Red, sf::Color::Green, sf::Color::Yellow,
        sf::Color::Cyan, sf::Color::Magenta, sf::Color::White
    };
    for (int i = 0; i < 100; ++i) {
        auto shape = sf::CircleShape(20.0f);
        shape.setPointCount(8);
        shape.setFillColor(colors.at(i%colors.size()));
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1);
        shape.setOrigin(20, 20);
        shape.setPosition((float)(std::rand()%2500), (float)(std::rand()%2000));
        this->shapes.push_back(shape);
    }

    // create Player entity (see class Player for more information)
    this->player.get<ns::ecs::Transform>().setPosition({100, 100});

    // create a BitmapFont
    this->font = new ns::BitmapFont(
        ns::Res::getTexture("font.png"),
        {8, 8},
        " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,:;!?-+=",
        {{"ABCDEFGHIJKMNOPQRSTUVWXYZ?=-", 7}, {"ijlntsofpqrux", 5}},
        6
    );
    // create a BitmapText using the font created above
    auto* bmp_text = new ns::BitmapText("Press E to toggle Shader \nPress R to run Shader Transition\nPress T to run Circle Transition");
    bmp_text->setFont(this->font);
    bmp_text->setPosition(250, 80);

    // create a textbox
    this->textbox = new ns::ui::TypedText("TypedText is useful in RPG games ! It creates a typing animation and can be configured to display text on multiple pages.");
    this->textbox->setFont(this->font);
    this->textbox->setMaxWidth(200);
    this->textbox->setMaxLines(2);
    this->textbox->setTypingDelay(5);
    this->textbox->setPosition(250, 125);

    // setup the particle system and emmit 500 particles
    this->particle_system.setTexture(ns::Res::getTexture("tileset.png"));
    this->particle_system.setEmitRate(30.f);
    this->particle_system.emit({224, 32, 16, 16}, 300, true);

    //-----------------------------------------------------------------------------------

    //------------ Scene and Layers creation --------------------------------------------
    auto& scene = this->createScene("main");
    scene.addLayer("shapes", 0); // create a new Layer
    scene.addLayer("entities", 1);
    scene.addLayer("texts", 2);
    //-----------------------------------------------------------------------------------

    //------------ Camera creation ------------------------------------------------------
    auto game_view = sf::Vector2i(640, 360);
    auto& game_camera = this->createCamera("main", 0, {{0, 0}, game_view});
    game_camera.lookAt(scene);     // tell the Camera to look at the scene
    game_camera.follow(this->player.get<ns::ecs::Transform>());   // tell the Camera to follow our entity
    game_camera.setFramesDelay(2);       // the Camera will have 10 frames delay over the player
    game_camera.setLimitsRect({{0, 0}, sf::Vector2i(this->tiled_map.getSize())});
    this->tiled_map.setCamera(game_camera);
    //-----------------------------------------------------------------------------------

    //------------ Add Drawables to the Scene  ------------------------------------------
    // add tiledmap layers to the scene one by one
    // scene.getDefaultLayer().add(this->tiled_map.getTileLayer("bg"));
    // scene.getDefaultLayer().add(this->tiled_map.getTileLayer("front"));
    // scene.getDefaultLayer().add(this->tiled_map.getObjectLayer("objects"));
    // or add the whole group at once
    scene.getDefaultLayer().add(this->tiled_map.getGroupLayer("Group 1"));

    // add shapes
    for (auto& shape : this->shapes) {
        scene.getLayer("shapes").add(shape);
    }

    // add entities
    scene.getLayer("entities").add(this->player);

    // add the BitmapText to the layer
    scene.getLayer("texts").add(bmp_text);
    scene.getLayer("texts").add(this->textbox);

    // add the particle system
    scene.getLayer("texts").add(this->particle_system);
    //-----------------------------------------------------------------------------------

    //------------ Add DebugTexts to the App -----------------------------------------
    // add a DebugText by using addDebugText method
    this->addDebugText<int>("frame counter:", &this->frame_counter, {10, 10});
    this->addDebugText<int>("nb of transitions:", [&]{return ns::Transition::list.size();}, {10, 50}, sf::Color::Green);

    // you can change debug text global properties by using DebugTextInterface
    // (will be applied to ALL debug texts created afterwards)
    ns::DebugTextInterface::color = sf::Color::Cyan;
    ns::DebugTextInterface::outline_thickness = 1;
    ns::DebugTextInterface::outline_color = sf::Color::Blue;
    this->addDebugText<sf::Vector2f>("mouse pos:", [&]{return getMousePosition(getCamera("main"));}, {10, 90});

    // add DebugText by creating manually a DebugText object, changing its properties and adding it to the app;
    // the app will delete automatically the debug texts, so don't worry about memory
    auto* dbg_txt = new ns::DebugText<sf::Vector2f>("position:", [&]{return player.get<ns::ecs::Transform>().getPosition();}, {500, 10});
    dbg_txt->setFillColor(sf::Color::Black);
    dbg_txt->setOutlineThickness(1);
    dbg_txt->setOutlineColor(sf::Color::White);
    this->addDebugText(dbg_txt);
    //-----------------------------------------------------------------------------------

    this->palette_shader = new sf::Shader();
    this->palette_shader->loadFromMemory(
        "uniform sampler2D texture;"
        "uniform float threshold;"
        "void main()"
        "{"
            "vec2 pos = gl_TexCoord[0].xy;"
            "vec4 col = texture2D(texture, pos);"
            "vec4 new_col = col;"
            "if (col.r <= threshold)"
                "new_col = vec4(0.20, 0.35, 0.1, 1.);"
            "else if (col.r <= threshold + 0.25f)"
                "new_col = vec4(0.4, 0.7, 0.3, 1.);"
            "else if (col.r <= threshold + 0.5f)"
                "new_col = vec4(0.6, 0.8, 0.6, 1.);"
            "else if (col.r <= threshold + 0.75f)"
                "new_col = vec4(0.90, 0.99, 0.90, 1.);"
            "else "
                "new_col = vec4(0.90, 0.99, 0.90, 1.);"
            "gl_FragColor = new_col;"
        "}",
        sf::Shader::Fragment
    );
    this->palette_shader->setUniform("threshold", 0.25f);
    this->setShader(palette_shader);
    this->toggleShader();

}

void Game::onEvent(const sf::Event& event) {
    ns::App::onEvent(event);
    textbox->onEvent(event);
    switch (event.type) {
        case sf::Event::Closed:
            this->getWindow().close();
            break;

        case sf::Event::KeyReleased:
            if(event.key.code == ns::Inputs::getButton("fullscreen"))
                this->toggleFullscreen();

            if (event.key.code == sf::Keyboard::E) {
                this->toggleShader();
            }
            if (event.key.code == sf::Keyboard::R) {
                auto* tr = new ShaderOutTransition("threshold");
                tr->start();
                tr->setOnEndCallback([](){
                    auto* next = new ShaderInTransition("threshold");
                    next->start();
                });
            }
            if (event.key.code == sf::Keyboard::T) {
                auto* tr = new ns::transition::CircleClose();
                tr->start();
                tr->setOnEndCallback([](){
                    auto* next = new ns::transition::CircleOpen();
                    next->start();
                });
            }
            break;

        default:
            break;
    }
}

void Game::update() {
    this->frame_counter++;

    // run the default systems
    ns::Ecs.run(ns::ecs::inputs_system);
    ns::Ecs.run(ns::ecs::physics_system);

    // apply gravity
    ns::Ecs.run<ns::ecs::Physics>([](auto& physics) {
        physics.setVelocity(physics.getVelocity() + sf::Vector2f(0, 0.1f));
    });

    // collisions logic
    auto colls_view = ns::Ecs.view<ns::ecs::AABBCollider>();
    colls_view.for_each_pair([&](auto ent1, auto ent2) {
        auto& coll1 = colls_view.get<ns::ecs::AABBCollider>(ent1);
        auto& coll2 = colls_view.get<ns::ecs::AABBCollider>(ent2);
        if (!coll1.dynamic && !coll2.dynamic)
            return;
        auto bounds1 = ns::FloatRect(coll1.getBounds());
        auto bounds2 = ns::FloatRect(coll2.getBounds());

        if (ns::Ecs.has<ns::ecs::Transform>(ent1))
            bounds1 = ns::Ecs.get<ns::ecs::Transform>(ent1).getTransform().transformRect(bounds1);
        if (ns::Ecs.has<ns::ecs::Transform>(ent2))
            bounds2 = ns::Ecs.get<ns::ecs::Transform>(ent2).getTransform().transformRect(bounds2);

        sf::FloatRect intersect;
        if (bounds1.intersects(bounds2, intersect)) {
            if (intersect.width < intersect.height) {
                if (coll1.dynamic) {
                    ns::Ecs.get<ns::ecs::Physics>(ent1).setVelocityX(0);
                    if (bounds1.left < bounds2.left)
                        ns::Ecs.get<ns::ecs::Transform>(ent1).move(-intersect.width, 0);
                    else
                        ns::Ecs.get<ns::ecs::Transform>(ent1).move(intersect.width, 0);
                }
                if (coll2.dynamic) {
                    ns::Ecs.get<ns::ecs::Physics>(ent2).setVelocityX(0);
                    if (bounds2.left < bounds1.left)
                        ns::Ecs.get<ns::ecs::Transform>(ent2).move(-intersect.width, 0);
                    else
                        ns::Ecs.get<ns::ecs::Transform>(ent2).move(intersect.width, 0);
                }
            }
            else {
                if (coll1.dynamic) {
                    ns::Ecs.get<ns::ecs::Physics>(ent1).setVelocityY(0);
                    if (bounds1.top < bounds2.top)
                        ns::Ecs.get<ns::ecs::Transform>(ent1).move(0, -intersect.height+0.0001f);
                    else
                        ns::Ecs.get<ns::ecs::Transform>(ent1).move(0, intersect.height);
                }
                if (coll2.dynamic) {
                    ns::Ecs.get<ns::ecs::Physics>(ent2).setVelocityY(0);
                    if (bounds2.top < bounds1.top)
                        ns::Ecs.get<ns::ecs::Transform>(ent2).move(0, -intersect.height+0.0001f);
                    else
                        ns::Ecs.get<ns::ecs::Transform>(ent2).move(0, intersect.height);
                }
            }
        }
        ns_LOG(intersect);
    });
    // run a custom system that updates transformable entities position
    ns::Ecs.run<ns::ecs::Transform, ns::ecs::Physics>([](auto& transform, auto& physics) {
        transform.move(physics.getVelocity());
    });
    ns::Ecs.run(ns::ecs::sprite_system);

    this->textbox->update();
    // move the shapes randomly
    for (auto& shape : this->shapes) {
        shape.move((float)(std::rand()%3) - 1.f, (float)(std::rand()%3) - 1.f);
        shape.rotate(1);
    }

    // update the player
    this->player.update();

    // update tiled map
    this->tiled_map.update();

    // sort shapes layer by the y position
    this->getScene("main").getLayer("shapes").ySort();

    // update particles
    this->particle_system.setPosition(getMousePosition(getCamera("main")));
    this->particle_system.update();
}

Game::~Game() {
    delete(this->palette_shader);
    delete(this->font);
}
