#ifndef CLASS_ENGINE
#define CLASS_ENGINE

#include <SFML/Graphics.hpp>
#include <vector>

#include "simplexnoise.h"
#include "defs.hpp"
#include "tools.hpp"
#include "gameobj.hpp"
#include "map.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "itemtypes.hpp"
#include "message.hpp"

struct consolecmd
{
    std::string cmd;
    std::string helptxt;
    consolecmd *parent;
};

class Engine
{
private:

    //singleton
    Engine();
    static Engine *onlyinstance;


    //screen elements
    sf::RenderWindow *m_Screen;
    int m_FrameRateLimit;

    int m_TermCurrentFGColor;
    int m_TermCurrentBGColor;
    sf::Vector2f m_TermCurrentCursorPos;


    //resources
    int m_TileWidth;
    int m_TileHeight;
    int m_TileSheetWidth;
    int m_TileSheetHeight;
    int m_ScreenTilesWidth;
    int m_ScreenTilesHeight;







    //loops
    void mainLoop();


    //draw
    void drawGame();
    void drawTile(int x, int y, int tilenum, int fgcolor = 1, int bgcolor = 0);
    void drawTile(int x, int y, char ch, int fgcolor = 1, int bgcolor = 0);
    void drawTileInViewport(int x, int y, int tilenum, int fgcolor = 1, int bgcolor = 0);
    //void drawTileInViewport(int x, int y, char ch, int fgcolor = 1, int bgcolor = 0);
    void drawString(int x, int y, std::string tstring, int fgcolor = 1, int bgcolor = 0);


public:
    ~Engine();

    static Engine *getInstance()
    {
        if(onlyinstance == NULL) onlyinstance = new Engine;
        return onlyinstance;
    }

    void start();

    sf::RenderWindow *getScreen() { return m_Screen;}

    long getSeed() { return m_Seed;}

    void playerTurnUpdates();

    int getMonsterDBSize() { return int(m_MonsterDB.size()); }
    Monster *createMonster(int monsterid);

    int getItemDBSize() { return int(m_ItemDB.size()); }
    Item *createItem(int itemid);
    bool moveItem(Item *titem, std::vector<Item*> *isource, std::vector<Item*> *idest);

    MapChunk *testmap;
    bool validWalkableTile(int x, int y);
    int getDirectionFromUser(sf::Vector2i *mcoord = NULL);

    sf::Vector2i getGlobalPosition() { return m_Player->getGlobalPos();}

    //noise
    sf::Vector2i getNoiseRelPosition0() { return m_Player->getNoisePosRel0();}
    std::vector< std::vector<int> > genNoise(int width, int height, int xoffset = 0, int yoffset = 0,
                                             float persistence = 0.7, float octaves = 6, float scale = 9,
                                             int minval = 0, int maxval = 255);


    //debug stuff
    void debugtest();
    void noisetest();
    sf::Thread *console_thread;
    void console(); //see console.cpp
    std::vector<consolecmd*> cCommands;
    void parse(std::string ccmd); // see console.cpp
};

#endif // CLASS_ENGINE
