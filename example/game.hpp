#ifndef CLASS_GAME
#define CLASS_GAME

#include <SFML\Graphics.hpp>

#include "sfmlcurs.hpp"

//example of using sfmlcurs

class Game
{

private:

    sf::RenderWindow *screen;

    SFMLCurs *sfmlcurses;

    void mainLoop();

public:
    Game();
    ~Game();

    void start();
    bool initScreen();
};
#endif // CLASS_GAME
