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
    void handle_event(const sf::Event& e, sf::Window& window, bool& show_debug);
} // namespace

int main()
{
    sf::RenderWindow window({1600, 900}, "Path Finding Visualiser - Press F1 for debug - Press F2 to hide grid");
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

    Application app{window};
    Keyboard keyboard;

    sf::Clock clock;
    while (window.isOpen())
    {
        for (sf::Event e{}; window.pollEvent(e);)
        {
            ImGui::SFML::ProcessEvent(e);
            keyboard.update(e);
            app.on_event(e);
            handle_event(e, window, show_debug);
        }
        auto dt = clock.restart();

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
            app.on_render(window, show_debug);
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
    }

    ImGui::SFML::Shutdown(window);
}

namespace
{
    void handle_event(const sf::Event& e, sf::Window& window, bool& show_debug)
    {
        switch (e.type)
        {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyReleased:
                switch (e.key.code)
                {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;

                    case sf::Keyboard::F1:
                        show_debug = !show_debug;
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
} // namespace