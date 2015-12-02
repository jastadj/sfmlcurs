#include "game.hpp"

Game::Game()
{


}

Game::~Game()
{


}

void Game::start()
{

}

bool Game::initScreen()
{
    screen = new sf::RenderWindow(sf::VideoMode(640,480,32), "Test");

    if(!screen->isOpen())
    {
        std::cout << "Error creating render "
        return false;
    }
}
