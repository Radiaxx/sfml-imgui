#include "game.hpp"

const std::string  WINDOW_TITLE        = "SFML + imGui";
const sf::Vector2u INITIAL_WINDOW_SIZE = {720u, 480u};
const unsigned int FRAME_RATE_LIMIT    = 60u;

int main()
{
    Game game = Game(WINDOW_TITLE, INITIAL_WINDOW_SIZE, FRAME_RATE_LIMIT);
    game.run();

    return EXIT_SUCCESS;
}