#include "game.hpp"
#include <iostream>

Game::Game()
{


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
    screen = new sf::RenderWindow(sf::VideoMode(640,480,32), "Test");

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
            }
        }

        //draw

        screen->display();

    }
}
