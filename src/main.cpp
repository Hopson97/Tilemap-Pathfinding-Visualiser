#include <iostream>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui_sfml/imgui-SFML.h>

#include "Application.h"
#include "Util/Array2D.h"
#include "Util/Profiler.h"
#include "Util/TimeStep.h"

int main()
{
    sf::RenderWindow window({1280, 720}, "SFML");
    window.setPosition({85, 75});
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Failed to init ImGUI::SFML\n";
        return -1;
    }

    TimeStep fixed_updater{50};
    Profiler profiler;
    bool show_profiler = false;

    Application app{window};

    sf::Clock clock;
    while (window.isOpen())
    {
        for (sf::Event e{}; window.pollEvent(e);)
        {
            ImGui::SFML::ProcessEvent(e);
            app.on_event(e);
            if (e.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (e.type == sf::Event::KeyReleased && e.key.code == sf::Keyboard::F1)
            {
                show_profiler = !show_profiler;
            }
        }
        auto dt = clock.restart();

        // Update
        ImGui::SFML::Update(window, dt);

        // Update
        {
            auto& update_profiler = profiler.begin_section("Update");
            app.on_update(dt);
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
            app.on_render(window);
            render_profiler.end_section();
        }

        // Show profiler
        profiler.end_frame();
        if (show_profiler)
        {
            profiler.gui();
        }

        // End frame...
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown(window);
}