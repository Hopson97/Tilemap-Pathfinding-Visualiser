#include <iostream>
#include <print>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui_sfml/imgui-SFML.h>

#include "Application.h"
#include "Util/Array2D.h"
#include "Util/Keyboard.h"
#include "Util/Profiler.h"
#include "Util/TimeStep.h"

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested);
} // namespace

int main()
{
    sf::RenderWindow window(sf::VideoMode({1600, 900}),
                            "Path Finding Visualiser - Press F1 for debug - Press F2 to hide grid");
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    if (!ImGui::SFML::Init(window))
    {
        std::println(std::cerr, "Failed to init ImGUI::SFML.");
        return EXIT_FAILURE;
    }

    TimeStep fixed_updater{50};
    Profiler profiler;
    bool show_debug = false;
    bool c = false;

    Application app{window};
    Keyboard keyboard;

    sf::Clock clock;
    while (window.isOpen())
    {
        bool close_requested = false;
        auto dt = clock.restart();
        while (auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            keyboard.update(*event);
            app.on_event(*event);
            handle_event(*event, window, show_debug, close_requested);
        }


        // Update

        {
            auto& update_profiler = profiler.begin_section("Update");
            ImGui::SFML::Update(window, dt);
            app.on_update(keyboard, dt);
            update_profiler.end_section();
        }

        // Fixed-rate update
        {
            auto& fixed_update_profiler = profiler.begin_section("Fixed Update");
            fixed_updater.update([&](sf::Time dt) { app.on_fixed_update(dt); });
            fixed_update_profiler.end_section();
        }
        // Render
        window.clear({100, 200, 255});
        {
            auto& render_profiler = profiler.begin_section("Render");
            app.on_gui(window, fixed_updater, show_debug);
            app.on_render(window);
            render_profiler.end_section();
        }

        // Show profiler
        profiler.end_frame();
        if (show_debug)
        {
            profiler.gui();
        }

        // End frame
        ImGui::SFML::Render(window);
        window.display();
        if (close_requested)
        {
            window.close();
        }
    }

    app.save_tile_maps();

    ImGui::SFML::Shutdown(window);
}

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested)
    {
        if (event.is<sf::Event::Closed>())
        {
            close_requested = true;
        }
        else if (auto* key = event.getIf<sf::Event::KeyPressed>())
        {
            switch (key->code)
            {
                case sf::Keyboard::Key::Escape:
                    close_requested = true;
                    break;

                case sf::Keyboard::Key::F1:
                    show_debug_info = !show_debug_info;
                    break;

                default:
                    break;
            }
        }
    }
} // namespace