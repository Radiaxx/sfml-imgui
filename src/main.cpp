#include <iostream>

#include "app.hpp"

int main()
{
    const sf::Vector2u initialWindowSize = {1280u, 720u};
    const sf::Vector2u minimumWindowSize = {854u, 480u};
    const unsigned int frameRateLimit    = 60u;
    const App::Config config = {"SFML+imGui Scalar Field Renderer", initialWindowSize, minimumWindowSize, frameRateLimit};

    try
    {
        App app(config);
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << "Application error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}