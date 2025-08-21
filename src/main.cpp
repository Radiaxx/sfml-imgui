#include <iostream>

#include "app.hpp"

int main()
{
    const App::Config config = {"SFML+imGui Scalar Field Renderer", {1280u, 720u}, 60u};

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