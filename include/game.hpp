#ifndef CLASS_GAME
#define CLASS_GAME

#include <SFML\Graphics.hpp>

#include "sfmlcurs.hpp"

class Game
{

private:

    sf::RenderWindow *screen;

    void mainLoop();

public:
    Game();
    ~Game();

    void start();
    bool initScreen();
};
#endif // CLASS_GAME
