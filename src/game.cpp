#include "game.hpp"
#include <iostream>

Game::Game()
{
    //create sfml curses object using the "tileart.png" file as the sprite sheet
    //note, it is expecting that the sprite sheet is 16x16 (256) tiles
    //      it is also expecting the the foreground color = white, and the background color is magenta
    //      this needs to be fixed however to account for greyscale foreground graphics as well as
    //      actually trying to use magenta as a legitimate color for drawing
    sfmlcurses = new SFMLCurs("tileart.png", 1);
    //sfmlcurses = new SFMLCurs("tileart.png", 2); // setting the char scale to 2 makes it twice as big

    //initialize
    if(!sfmlcurses->init())
    {
        std::cout << "Error initializing SFML curses..\n";
    }
}

Game::~Game()
{


}

void Game::start()
{
    //start initialization
    if(!initScreen())
    {
        std::cout << "Unable to initialize render window.\n";
        return;
    }

    //start main loop
    mainLoop();
}

bool Game::initScreen()
{
    //create render window
    //create a standard terminal height and width
    int termwidth = sfmlcurses->getTileSize().x * 80;
    int termheight = sfmlcurses->getTileSize().y * 25;

    std::cout << "term width and height = " << termwidth << "," << termheight << std::endl;

    screen = new sf::RenderWindow(sf::VideoMode(termwidth, termheight ,32), "Test");

    //did render window create as expected?
    if(!screen->isOpen())
    {
        std::cout << "Error creating render window.\n";
        return false;
    }

    //all was good..
    return true;
}

void Game::mainLoop()
{
    bool quit = false;

    sf::Vector2i playerpos(6,6);

    while(!quit)
    {
        sf::Event event;

        screen->clear();

        while(screen->pollEvent(event))
        {
            if(event.type == sf::Event::Closed) quit = true;
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Up) playerpos.y--;
                else if(event.key.code == sf::Keyboard::Down) playerpos.y++;
                else if(event.key.code == sf::Keyboard::Left) playerpos.x--;
                else if(event.key.code == sf::Keyboard::Right) playerpos.x++;
            }
        }

        //draw
        sfmlcurses->drawString(screen, 0, 0, "This is a test!", SFC_GREEN_B);
        sfmlcurses->draw(screen, 5, 5, 3, SFC_RED_B, SFC_WHITE);
        sfmlcurses->draw(screen, playerpos.x, playerpos.y, '@', SFC_RED_B);

        screen->display();

    }
}
