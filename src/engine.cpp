#include "engine.hpp"

//debug
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

//statics
Engine *Engine::onlyinstance = NULL;
std::vector<int> Engine::m_ItemFilterAll;

Engine::Engine()
{
    m_TileWidth = 8;
    m_TileHeight = 12;

    m_ScreenTilesWidth = 80;
    m_ScreenTilesHeight = 25;
    m_TermCurrentBGColor = COLOR_BLACK;
    m_TermCurrentFGColor = COLOR_WHITE;

    m_Viewport.width = 41;
    m_Viewport.height = 25;
    m_ViewportPosition = sf::Vector2i(0,0);

    m_ItemFilterAll.push_back(OBJ_TOTAL);

    //set seed
    m_Seed = long(time(NULL));
    srand(m_Seed);

}

Engine::~Engine()
{

}

void Engine::start()
{
    //start console thread
    console_thread = new sf::Thread(&Engine::console, this);
    console_thread->launch();

    //init screen
    std::cout << "Initializing screen...";
    if(initScreen()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    //init tile art
    std::cout << "Initializing tile sprites...";
    if(initTileArt()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    std::cout << "Initializing liquids...";
    if(initLiquids()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    std::cout << "Initializing map tiles...";
    if(initMapTiles()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    std::cout << "Initializing monster database list...";
    if(initMonsters()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    std::cout << "Initializing item database list...";
    if(initItems()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    //debug noise
    //noisetest();

    std::cout << "Starting new game.\n";
    newGame();

    //start mainloop
    std::cout << "Starting main loop...\n";
    mainLoop();

    //if player died...
    if(!m_Player->isAlive()) playerDeath();

}

void Engine::playerDeath()
{
    bool quit = false;

    while(!quit)
    {
        sf::Event event;

        m_Screen->clear();

        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
            }
        }

        drawString(0,0, "You are dead!");

        m_Screen->display();
    }
}

bool Engine::initScreen()
{
    //create render window
    m_Screen = new sf::RenderWindow(sf::VideoMode(m_ScreenTilesWidth*m_TileWidth, m_ScreenTilesHeight*m_TileHeight,32), "Test");
    m_Screen->setFramerateLimit(m_FrameRateLimit);
    m_Screen->clear();
    m_Screen->display();

    //create colors
    m_AsciiColors.push_back(sf::Color(0x00, 0x00, 0x00)); // black
    m_AsciiColors.push_back(sf::Color(0xc0, 0xc0, 0xc0)); // white
    m_AsciiColors.push_back(sf::Color(0xff, 0xff, 0xff)); // bright white
    m_AsciiColors.push_back(sf::Color(0x80, 0x00, 0x00)); // red
    m_AsciiColors.push_back(sf::Color(0xff, 0x00, 0x00)); // bright red
    m_AsciiColors.push_back(sf::Color(0x00, 0x00, 0x80)); // blue
    m_AsciiColors.push_back(sf::Color(0x00, 0x00, 0xff)); // bright blue
    m_AsciiColors.push_back(sf::Color(0x00, 0x80, 0x00)); // green
    m_AsciiColors.push_back(sf::Color(0x00, 0xff, 0x00)); // bright green
    m_AsciiColors.push_back(sf::Color(0x00, 0x80, 0x80)); // cyan
    m_AsciiColors.push_back(sf::Color(0x00, 0xff, 0xff)); // bright cyan
    m_AsciiColors.push_back(sf::Color(0x80, 0x00, 0x80)); // magenta
    m_AsciiColors.push_back(sf::Color(0xff, 0x00, 0xff)); // bright magenta
    m_AsciiColors.push_back(sf::Color(0x80, 0x80, 0x00)); // yellow
    m_AsciiColors.push_back(sf::Color(0xff, 0xff, 0x00)); // bright yellow

    return true;
}

bool Engine::initTileArt()
{
    //color mask for foreground and background
    sf::Color colormaskbg(0xff,0x00,0xff);
    sf::Color colormaskfg(0xff,0xff,0xff);

    //create texture for each color combination
    //background color loop
    for(int i = 0; i < COLOR_TOTAL; i++)
    {
        //add column
        m_TileTextures.resize(m_TileTextures.size()+1);

        //foreground color loop
        for(int n = 0; n < COLOR_TOTAL; n++)
        {
            //texture for image
            sf::Texture newtxt;

            //create temporary image file
            sf::Image newimage;
            newimage.loadFromFile("tileart.png");

            //set foreground color
            replaceImageColor(&newimage, colormaskfg, m_AsciiColors[n]);

            //set background color
            replaceImageColor(&newimage, colormaskbg, m_AsciiColors[i]);

            //set texture from image
            newtxt.loadFromImage(newimage);

            //debug
            //sf::Sprite testsprite(newtxt);
            //m_Screen->draw(testsprite);
            //m_Screen->display();

            m_TileTextures[i].push_back(newtxt);
        }
    }

    //clip all the characters for each character type from bg/fg combos
    for(int i = 0; i < m_TileSheetWidth*m_TileSheetHeight; i++)
    {
        //add new character index
        m_TileSprites.resize( m_TileSprites.size()+1);

        //for each character, go through each color combination and clip to a sprite
        //bg color loop
        for(int n = 0; n < COLOR_TOTAL; n++)
        {
            //add new background index
            m_TileSprites[i].resize( m_TileSprites[i].size()+1);

            //fg color loop
            for(int p = 0; p < COLOR_TOTAL; p++)
            {
                sf::IntRect clip;

                clip.width = m_TileWidth;
                clip.height = m_TileHeight;
                clip.left = (i - ( (i/m_TileSheetWidth) * m_TileSheetWidth ) ) * m_TileWidth;
                clip.top = (i/m_TileSheetWidth) * m_TileHeight;

                sf::Sprite newsprite(m_TileTextures[n][p], clip);

                //debug
                //m_Screen->draw(newsprite);
                //m_Screen->display();

                m_TileSprites[i][n].push_back(newsprite);
            }
        }
    }

    return true;
}

bool Engine::initMapTiles()
{
    //note : the first tile should always be a black/black/blank tile as empty space

    MapTile *newtile = new MapTile;

    //tile 0
    newtile->m_Name = "empty space";
    newtile->m_TileID = 0;
    newtile->m_FGColor = COLOR_BLACK;
    newtile->m_BGColor = COLOR_BLACK;
    m_MapTiles.push_back(newtile);

    //tile 1
    newtile = new MapTile;
    newtile->m_Name = "floor";
    newtile->m_TileID = int('.');
    m_MapTiles.push_back(newtile);

    //tile 2
    newtile = new MapTile;
    newtile->m_Name = "wall";
    newtile->m_TileID = int(219);
    newtile->setPassesLight(false);
    newtile->setWalkable(false);
    m_MapTiles.push_back(newtile);

    //**********START GRASS TILES***********
    //tile 3 - START GRASS TILES
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('.');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 4
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int(',');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 5
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('`');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 6
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('\'');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 7
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('\"');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 8 - START LIGHT GRASS
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('.');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_B_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 9
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int(',');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_B_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 10
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('`');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_B_GREEN;
    m_MapTiles.push_back(newtile);

    //tile 11
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('\'');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_B_GREEN;
    m_MapTiles.push_back(newtile);


    //tile 12
    newtile = new MapTile;
    newtile->m_Name = "grass";
    newtile->m_TileID = int('\"');
    newtile->m_BGColor = COLOR_BLACK;
    newtile->m_FGColor = COLOR_B_GREEN;
    m_MapTiles.push_back(newtile);
    //**********END GRASS TILES***********

    //tile 13
    newtile = new MapTile;
    newtile->m_Name = "water";
    newtile->m_TileID = int('~');
    newtile->m_FGColor = COLOR_B_BLUE;
    newtile->setLiquid(m_Liquids[0]);
    newtile->setWalkable(false);
    m_MapTiles.push_back(newtile);

    //tile 14
    newtile = new MapTile;
    newtile->m_Name = "tree";
    newtile->m_TileID = 6;
    newtile->m_FGColor = COLOR_B_GREEN;
    newtile->setWalkable(false);
    newtile->setPassesLight(false);
    m_MapTiles.push_back(newtile);


    std::cout << " - " << m_MapTiles.size() << " map tiles loaded - ";
    return true;
}

bool Engine::initMonsters()
{
    Monster *newmonster = new Monster;
    newmonster->m_Name = "unicorn";
    newmonster->m_TileID = int('U');
    newmonster->m_FGColor = COLOR_YELLOW;
    m_MonsterDB.push_back(newmonster);

    return true;
}

bool Engine::initItems()
{

    //item 0
    Item *newitem = new ContainerLiquid;
    ContainerLiquid *cptr = dynamic_cast<ContainerLiquid*>(newitem);
    cptr->m_Name = "water bottle";
    cptr->setMaxVolume(3);
    cptr->m_TileID = int('!');
    m_ItemDB.push_back(cptr);

    //item 1
    newitem = new MiscItem;
    newitem->m_Name = "rock";
    newitem->m_TileID = int('*');
    m_ItemDB.push_back(newitem);


    //set all item ids as index position
    for(int i = 0; i < int(m_ItemDB.size()); i++)
    {
        m_ItemDB[i]->setItemID(i);
    }

    std::cout << "Initialized " << m_ItemDB.size() << " items - ";
    return true;
}

bool Engine::initLiquids()
{

    Liquid *newliquid = new Liquid;
    newliquid->setName("water");
    newliquid->setColor(COLOR_BLUE);
    m_Liquids.push_back(newliquid);

    for(int i = 0; i < int(m_Liquids.size()); i++) m_Liquids[i]->setLiquidID(i);

    return true;
}

bool Engine::initMap()
{
    if(testmap != NULL) delete testmap;

    testmap = new MapChunk(0,0);

    return true;
}

bool Engine::initPlayer()
{
    if(m_Player != NULL) delete m_Player;

    m_Player = new Player;

    m_Player->m_Name = "test player";
    m_Player->setMaxHealth(10);
    m_Player->setCurrentHealth(m_Player->getMaxHealth());

    return true;
}

bool Engine::newGame(long nseed)
{
    //start new random seed
    m_Seed = nseed;
    srand(m_Seed);
    std::cout << "Randomizing seed : " << m_Seed << std::endl;

    //init map
    std::cout << "Initializing map...";
    if(initMap()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    //init player
    std::cout << "Initializing player...";
    if(initPlayer()) std::cout << "done.\n";
    else std::cout << "failed.\n";

    std::cout << "Initializing message manager...";
    if(m_MessageManager != NULL) delete m_MessageManager;
    m_MessageManager = new MessageManager;
    std::cout << "done.\n";

    //create unicorn
    testmap->addMonster(0, testmap->getRandomValidPosition());

    //randomize player starting position
    m_Player->setPosition(testmap->getRandomValidPosition());

    //add water bottle to player
    Item *pitem = createItem(0);
    ContainerLiquid *cptr = dynamic_cast<ContainerLiquid*>(pitem);
    cptr->fillWithLiquid(m_Liquids[0]);
    m_Player->addToInventory(pitem);

    //add rock
    m_Player->addToInventory( createItem(1));

}

void Engine::mainLoop()
{
    bool quit = false;

    //frame loop
    while(!quit)
    {
        //create event que
        sf::Event event;

        //clear screen
        m_Screen->clear();



        //handle events
        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::Closed) quit = true;
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::F1) debugtest();
                else if(event.key.code == sf::Keyboard::I) showInventory();
                else if(event.key.code == sf::Keyboard::F) fillLiquidContainer();
                else if(event.key.code == sf::Keyboard::D) dropItemUI();
                else if(event.key.code == sf::Keyboard::G) pickupItemFromTileUI(m_Player->getPosition());
                else if(event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Numpad4)
                {
                    m_Player->walkDir(DIR_WEST);
                }
                else if(event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Numpad6)
                {
                    m_Player->walkDir(DIR_EAST);
                }
                else if(event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Numpad8)
                {
                    m_Player->walkDir(DIR_NORTH);
                }
                else if(event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Numpad2)
                {
                    m_Player->walkDir(DIR_SOUTH);
                }
                else if(event.key.code == sf::Keyboard::Numpad1)
                {
                    m_Player->walkDir(DIR_SW);
                }
                else if(event.key.code == sf::Keyboard::Numpad3)
                {
                    m_Player->walkDir(DIR_SE);
                }
                else if(event.key.code == sf::Keyboard::Numpad7)
                {
                    m_Player->walkDir(DIR_NW);
                }
                else if(event.key.code == sf::Keyboard::Numpad9)
                {
                    m_Player->walkDir(DIR_NE);
                }
            }
        }

        //UPDATE
        //center viewport on player
        centerViewport(m_Player->getPosition());


        //draw
        drawGame();

        //update and display screen
        m_Screen->display();

        if(!m_Player->isAlive())
        {
            return;
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////
//  DRAW
bool Engine::posInViewport(int x, int y)
{
    if(x < m_Viewport.left || y < m_Viewport.top ||
       x >= m_Viewport.left + m_Viewport.width || y >= m_Viewport.top + m_Viewport.height)
    {
         return false;
    }

    return true;
}

void Engine::centerViewport(int x, int y)
{
    m_Viewport.left = x - m_Viewport.width/2;
    m_Viewport.top = y - m_Viewport.height/2;

    //if map moves off viewport, keep map in view
    if(m_Viewport.left < 0) m_Viewport.left = 0;
    else if(m_Viewport.left + m_Viewport.width >= testmap->getDimensions().x) m_Viewport.left = testmap->getDimensions().x - m_Viewport.width;

    if(m_Viewport.top < 0) m_Viewport.top = 0;
    else if(m_Viewport.top + m_Viewport.height >= testmap->getDimensions().y) m_Viewport.top = testmap->getDimensions().y - m_Viewport.height;
}

bool Engine::posInViewport(sf::Vector2i pos)
{
    return posInViewport(pos.x, pos.y);
}

void Engine::drawGame()
{
    drawMap();
    drawItems();
    drawPlayer();
    drawMonsters();
    drawStatus();
    drawMessageQue();
}

void Engine::drawTile(int x, int y, int tilenum, int fgcolor, int bgcolor)
{
    //assuming color black = 0, dont do anything
    if(fgcolor == 0 && bgcolor == 0) return;

    //check that color and tile indices are in range
    if(fgcolor < 0 || bgcolor < 0 || fgcolor >= COLOR_TOTAL || bgcolor >= COLOR_TOTAL)
    {
        std::cout << "Error drawing tile, color index out of range : bg=" << bgcolor << " , fg=" << fgcolor << std::endl;
        return;
    }
    else if(tilenum < 0 || tilenum >= int(m_TileSprites.size()) )
    {
        std::cout << "Error drawing tile, sprite index out of range : " << tilenum << std::endl;
        return;
    }

    m_TileSprites[tilenum][bgcolor][fgcolor].setPosition(x*m_TileWidth, y*m_TileHeight);
    m_Screen->draw(m_TileSprites[tilenum][bgcolor][fgcolor]);
}

void Engine::drawTile(int x, int y, char ch, int fgcolor, int bgcolor)
{
    drawTile(x, y, int(ch), fgcolor, bgcolor);
}

void Engine::drawTileInViewport(int x, int y, int tilenum, int fgcolor, int bgcolor)
{
    if(posInViewport(x,y))
    {
        drawTile(x - m_Viewport.left, y - m_Viewport.top, tilenum, fgcolor, bgcolor);
    }
}

void Engine::drawString(int x, int y, std::string tstring, int fgcolor, int bgcolor)
{
    sf::Vector2i cursorpos = sf::Vector2i(x,y);

    for(int i = 0; i < int(tstring.length()); i++)
    {
        drawTile(cursorpos.x, cursorpos.y, tstring.c_str()[i], fgcolor, bgcolor);
        cursorpos.x++;
    }
}

void Engine::drawWindow( int x, int y, int width, int height, int fgcolor, int bgcolor,
                int tl, int hor, int tr, int vert, int bl, int br, int filltile)
{
    //2d array for width/height
    std::vector< std::vector< int> > tbox;
    tbox.resize(height);
    for(int i = 0; i < height; i++) tbox[i].resize(width);

    //set window tiles
    for(int i = 0; i < height; i++)
    {
        for(int n = 0; n < width; n++)
        {
            //top
            if(i == 0)
            {
                //top left
                if(n == 0) tbox[i][n] = tl;
                //top right
                else if(n == width-1) tbox[i][n] = tr;
                //top line
                else tbox[i][n] = hor;
            }
            //bottom
            else if(i == height-1)
            {
                //top left
                if(n == 0) tbox[i][n] = bl;
                //top right
                else if(n == width-1) tbox[i][n] = br;
                //top line
                else tbox[i][n] = hor;
            }
            //sides
            else if(n == 0) tbox[i][n] = vert;
            else if(n == width-1) tbox[i][n] = vert;
            else tbox[i][n] = filltile;

        }
    }

    //draw window tiles
    for(int i = 0; i < height; i++)
    {
        for(int n = 0; n < width; n++)
        {
            drawTile(x+n, y+i, tbox[i][n], fgcolor, bgcolor);
        }
    }
}

void Engine::drawPlayer()
{
    if(m_Player == NULL) return;

    sf::Vector2i ppos = m_Player->getPosition();

    drawTileInViewport(ppos.x, ppos.y, m_Player->getTileID(), m_Player->getFGColor(), m_Player->getBGColor());
}

void Engine::drawMap()
{

    for(int i = 0; i < testmap->getDimensions().y; i++)
    {
        for(int n = 0; n < testmap->getDimensions().x; n++)
        {
            MapTile *ttile = getMapTile( testmap->getTile(n, i));

            if(posInViewport(n, i))
            if( inFOV(m_Player->getPosition().x, m_Player->getPosition().y, n, i) ) drawTileInViewport(n, i, ttile->getTileID(), ttile->getFGColor(), ttile->getBGColor());

        }
    }
}

void Engine::drawMonsters()
{
    for(int i = 0; i < int(testmap->getMapMonsters()->size()); i++)
    {
        Monster *tmonster = (*testmap->getMapMonsters())[i];

        sf::Vector2i tpos = tmonster->getPosition();

        if(posInViewport(tpos))
        if( inFOV(m_Player->getPosition().x, m_Player->getPosition().y, tpos.x, tpos.y) )
            drawTileInViewport(tmonster->getPosition().x, tmonster->getPosition().y, tmonster->getTileID(),
                    tmonster->getFGColor(), tmonster->getBGColor());
    }
}

void Engine::drawItems()
{
    for(int i = 0; i < int(testmap->getMapItems()->size()); i++)
    {
        Item *titem = (*testmap->getMapItems())[i];

        sf::Vector2i tpos = titem->getPosition();

        if(posInViewport(tpos))
        if( inFOV(m_Player->getPosition().x, m_Player->getPosition().y, tpos.x, tpos.y) ) drawTileInViewport(titem->getPosition().x, titem->getPosition().y, titem->getTileID(), titem->getFGColor(),
                 titem->getBGColor());
    }
}

void Engine::drawStatus()
{
    std::stringstream pname;
    pname << "Name : " << m_Player->getName();
    drawString(50, 2, pname.str());

    std::stringstream pturns;
    pturns << "Turns : " << m_Player->getTurns();
    drawString(50,3, pturns.str());

    std::stringstream phealth;
    phealth << "HP : " << m_Player->getCurrentHealth() << "/" << m_Player->getMaxHealth();
    drawString(50,4, phealth.str());

    //if player needs to drink
    if(m_Player->getHydrationLevel() <= 3)
    {
        //if player is getting thirsty
        if(m_Player->getHydrationLevel() >= 2) drawString(50, 5, "Thirsty", COLOR_BLUE);
        else drawString(50,5, "Dehydrated", COLOR_RED);
    }
    else if(m_Player->getHydrationLevel() == m_Player->getMaxHydrationLevel())
    {
        drawString(50,5, "Well Hydrated", COLOR_GREEN);
    }
}

void Engine::drawMessageQue()
{
    std::vector<Message*> *msgque = m_MessageManager->getMessageQue();

    if(msgque == NULL)
    {
        std::cout << "Error - unable to draw message que, que = NULL!\n";
        return;
    }
    if(msgque->empty()) return;

    //cycle through each message and draw
    for(int i = 0; i < int(msgque->size()); i++)
    {
        drawString(0, i, (*msgque)[i]->getString());
    }
}

bool Engine::inFOV(int sx, int sy, int tx, int ty)
{
    //some sight radius calculations should be done first to determine
    //if this check should be performed in the first place...

    //if checking source == destination
    if(sx-tx == 0 && sy-ty == 0) return true;

    //if los lands on y axis
    if(sx-tx == 0)
    {
        //start from target and walk back to plr pos
        for(int i = ty; i != sy; i+= (sy-ty)/abs(sy-ty))
        {
            //ignore first tile
            if(i == ty) continue;
            else
            {
                if( !m_MapTiles[testmap->getTile(tx,i)]->passesLight() ) return false;
            }
        }

        //check passed, return true
        return true;

    }
    else //if tile lands on x axis
    if(sy-ty == 0)
    {
       //start from target and walk back to plr pos
        for(int i = tx; i != sx; i+= (sx-tx)/abs(sx-tx))
        {
            //ignore first tile
            if(i == tx) continue;
            else //if tile is within p map
            {
                if( !m_MapTiles[testmap->getTile(i,ty)]->passesLight()) return false;
            }
        }

        //check passed, return true
        return true;
    }

    //y = mx+b
    float m = (float(ty-sy)/float(tx-sx) );
    float b = sy - (m*sx);

    //if coords are on the same active map as plr
    //if(gx == pgx && gy == pgy)
    //{
        for(int i = tx; i != sx; i+= (sx-tx)/abs(sx-tx))
        {
            //if first tile, ignore
            if(i == tx) continue;

            int tempx = i;
            int tempy = round(m*i+b);

            //check to see if target tile is passable
            if( !m_MapTiles[testmap->getTile(tempx, tempy)]->passesLight()) return false;
        }

        for(int i = ty; i != sy; i+= (sy-ty)/abs(sy-ty))
        {
            //if first tile, ignore
            if(i == ty) continue;

            int tempx = round((i-b)/m);
            int tempy = i;

            //check to see if target tile is passable
            if( !m_MapTiles[testmap->getTile(tempx, tempy)]->passesLight()) return false;
        }

        return true;


    return false;
}

///////////////////////////////////////////////////////////////////

void Engine::showInventory()
{
    bool quit = false;

    int selecteditem = 0;

    std::vector< Item*> *inv = m_Player->getInventory();

    while(!quit)
    {
        m_Screen->clear();

        sf::Event event;

        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Down) selecteditem++;
                else if(event.key.code == sf::Keyboard::Up) selecteditem--;
                else if(event.key.code == sf::Keyboard::Return)
                {
                    //if using liquid container
                    if( (*inv)[selecteditem]->getType() == OBJ_ITEM_CONTAINER_LIQUID)
                    {
                        ContainerLiquid *cptr = dynamic_cast<ContainerLiquid*>( (*inv)[selecteditem]);
                        //if liquid container is not empty
                        if(!cptr->isEmpty() )
                        {
                            if(m_Player->drink(cptr->getLiquidType()))
                            {
                                cptr->removeLiquid(1);
                            }
                        }
                    }
                }
            }
        }

        if(selecteditem < 0) selecteditem = int(inv->size()-1);
        else if(selecteditem >= int(inv->size())) selecteditem = 0;
        //draw



        if(inv->empty())
        {
            drawString(0,0,"You have nothing!\n");
        }
        else
        {
            drawString(0,0, "You are carrying:");
            drawString(0,1, "-----------------");

            for(int i = 0; i < int(inv->size()); i++)
            {
                std::stringstream istring;

                istring << (*inv)[i]->getName();

                //if item is a container
                if( (*inv)[i]->getType() == OBJ_ITEM_CONTAINER)
                {
                    //get container pointer
                    Container *cptr = NULL;
                    cptr = dynamic_cast<Container*>((*inv)[i]);

                    //if container is empty
                    if(cptr->getInventory()->empty()) istring << " (empty)";
                }
                else if( (*inv)[i]->getType() == OBJ_ITEM_CONTAINER_LIQUID)
                {
                    ContainerLiquid *cptr = dynamic_cast<ContainerLiquid*>( (*inv)[i]);
                    if(cptr->getCurrentVolume() <= 0) istring << " (empty)";
                    else
                    {
                        istring << " (" << cptr->getCurrentVolume() << "/" << cptr->getMaxVolume() << " of " << cptr->getLiquidType()->getName() << ")";

                    }
                }

                if(selecteditem == i)
                {
                    drawString(0, i+2, istring.str(), COLOR_BLACK, COLOR_WHITE);
                }
                else drawString(0, i+2, istring.str());
            }
        }



        //update
        m_Screen->display();
    }

}

Item *Engine::selectItemFromInventory(std::vector<Item*> *source, std::string promptstr,  std::vector<int> itemfilter)
{
    bool quit = false;

    Item *selecteditem = NULL;
    int selecteditemindex = 0;

    //get player inventory list
    std::vector<Item*> *inv = source;
    std::vector<Item*> filteredlist;

    //build item list based on filter
    for(int i = 0; i < int(inv->size()); i++)
    {
        //for each inventory item, check to see if it matches filter criteria
        for(int n = 0; n < int(itemfilter.size()); n++)
        {
            //if filter matches or OBJ_TOTAL filter is used, add item to filtered items list
            if( (*inv)[i]->getType() == itemfilter[n] || itemfilter[n] == OBJ_TOTAL)
            {
                filteredlist.push_back( (*inv)[i]);
                break;
            }
        }
    }

    while(!quit)
    {
        sf::Event event;

        m_Screen->clear();

        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Down) selecteditemindex++;
                else if(event.key.code == sf::Keyboard::Up) selecteditemindex--;
                else if(event.key.code == sf::Keyboard::Return)
                {
                    selecteditem = filteredlist[selecteditemindex];

                    //debug
                    std::cout << "selected " << selecteditem->getName() << std::endl;

                    return selecteditem;
                }
            }
        }

        //resolve selected item index if out of bounds
        if(selecteditemindex >= int(filteredlist.size()) ) selecteditemindex = 0;
        else if(selecteditemindex < 0) selecteditemindex = int(filteredlist.size()-1);

        //draw
        drawString(0,0, promptstr);

        //draw all items in filtered items list
        for(int i = 0; i < int(filteredlist.size()); i++)
        {
            std::stringstream inamestr;
            inamestr << filteredlist[i]->getName();

            if(selecteditemindex == i) drawString(0, i+2, inamestr.str(), COLOR_BLACK, COLOR_WHITE);
            else drawString(0, i+2, inamestr.str());
        }

        //display
        m_Screen->display();
    }

    return selecteditem;
}

std::vector<Item*> Engine::selectMultipleItemsFromInventory(std::vector<Item*> *source, std::string promptstr,  std::vector<int> itemfilter)
{
    bool quit = false;

    std::vector<Item*> selecteditems;
    std::vector<bool> bselecteditems;
    int selecteditemindex = 0;

    //get player inventory list
    std::vector<Item*> *inv = source;
    std::vector<Item*> filteredlist;

    //build item list based on filter
    for(int i = 0; i < int(inv->size()); i++)
    {
        //for each inventory item, check to see if it matches filter criteria
        for(int n = 0; n < int(itemfilter.size()); n++)
        {
            //if filter matches or OBJ_TOTAL filter is used, add item to filtered items list
            if( (*inv)[i]->getType() == itemfilter[n] || itemfilter[n] == OBJ_TOTAL)
            {
                filteredlist.push_back( (*inv)[i]);
                bselecteditems.push_back(false);
                break;
            }
        }
    }

    while(!quit)
    {
        sf::Event event;

        m_Screen->clear();

        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Down) selecteditemindex++;
                else if(event.key.code == sf::Keyboard::Up) selecteditemindex--;
                else if(event.key.code == sf::Keyboard::Return)
                {
                    //toggle selected item
                    bselecteditems[selecteditemindex] = !bselecteditems[selecteditemindex];
                }
            }
        }

        //resolve selected item index if out of bounds
        if(selecteditemindex >= int(filteredlist.size()) ) selecteditemindex = 0;
        else if(selecteditemindex < 0) selecteditemindex = int(filteredlist.size()-1);

        //draw
        drawString(0,0, promptstr);

        //draw all items in filtered items list
        for(int i = 0; i < int(filteredlist.size()); i++)
        {
            std::stringstream inamestr;
            if(bselecteditems[i]) inamestr << " + ";
            else inamestr << " - ";
            inamestr << filteredlist[i]->getName();

            if(selecteditemindex == i) drawString(0, i+2, inamestr.str(), COLOR_BLACK, COLOR_WHITE);
            else drawString(0, i+2, inamestr.str());
        }

        //display
        m_Screen->display();
    }

    //put selected items into list
    for(int i = 0; i < int(filteredlist.size()); i++)
    {
        if(bselecteditems[i]) selecteditems.push_back(filteredlist[i]);
    }

    return selecteditems;
}

bool Engine::fillLiquidContainer()
{
    //query item from player to fill
    std::vector<int> filter;
    filter.push_back(OBJ_ITEM_CONTAINER_LIQUID);
    Item *titem = selectItemFromInventory(m_Player->getInventory(), "Fill what?", filter);
    ContainerLiquid *tcl = dynamic_cast<ContainerLiquid*>(titem);
    if(tcl == NULL) return false;

    //get direction of source
    m_MessageManager->addMessage("Fill from what source?");
    m_Screen->clear();
    drawGame();
    //drawString(0, m_ScreenTilesHeight-1, "Fill from what source?");
    m_Screen->display();

    //get tile to draw liquid from
    sf::Vector2i ppos = m_Player->getPosition();
    getDirectionFromUser(&ppos);

    //check that tile is valid liquid source
    if(m_MapTiles[testmap->getTile(ppos.x, ppos.y)]->hasLiquid() )
    {
        //check if item container type can fill from source liquid
        Liquid *source = m_MapTiles[testmap->getTile(ppos.x, ppos.y)]->getLiquid();

        //if container is not empty
        if(!tcl->isEmpty())
        {
            //if liquid types do not match
            if(tcl->getLiquidType() != source)
            {
                std::stringstream fmsg;
                fmsg << "Need to empty " << tcl->getName() << " before filling with " << source->getName();
                m_MessageManager->addMessage(fmsg.str() );
                return false;
            }
        }

        //go ahead and fill with source liquid
        tcl->fillWithLiquid(source);
        std::stringstream fmsg;
        fmsg << "Filled " << tcl->getName() << " with " << source->getName();
        m_MessageManager->addMessage(fmsg.str() );

    }
    else
    {
        m_MessageManager->addMessage("There is no liquid source there!");
        return false;
    }

    return true;

}

void Engine::dropItemUI()
{
    Item *titem = selectItemFromInventory(m_Player->getInventory(), "Drop what?");

    //no valid item selected to drop
    if(titem == NULL) return;

    //move item from player inventory to map items
    moveItem(titem, m_Player->getInventory(), testmap->getMapItems());

    //set item's position to players position
    titem->setPosition( m_Player->getPosition());

    //drop message
    std::stringstream dmsg;
    dmsg << "Dropped " << titem->getName() << ".";
    m_MessageManager->addMessage(dmsg.str());
}

void Engine::playerTurnUpdates()
{
    m_MessageManager->update();
}

void Engine::pickupItemFromTileUI(int x, int y)
{
    std::vector< Item*> ilist = testmap->getMapItemsAtTile(x, y);
    std::vector< Item*> selecteditems;

    if(ilist.empty()) return;

    //if theres only one item there, add it to selected items list
    if( int(ilist.size()) == 1) selecteditems = ilist;

    //if there are multiple items use UI to select which items to pickup
    else
    {
        selecteditems = selectMultipleItemsFromInventory(&ilist, "Get what items?");
    }

    //move selected items to player inv
    for(int i = 0; i < int(selecteditems.size()); i++)
    {
        if(moveItem(selecteditems[i], testmap->getMapItems(), m_Player->getInventory()) )
        {
            std::stringstream pstr;
            pstr << "Picked up " << selecteditems[i]->getName();
            m_MessageManager->addMessage(pstr.str());
        }
    }



}

void Engine::pickupItemFromTileUI(sf::Vector2i tpos)
{
    pickupItemFromTileUI(tpos.x, tpos.y);
}

///////////////////////////////////////////////////////////////////
MapTile *Engine::getMapTile(int tileid)
{
    if(m_MapTiles.empty())
    {
        std::cout << "ERROR!  There are no maptiles stored in memory!!\n";
        return NULL;
    }

    if(tileid < 0 || tileid >= int(m_MapTiles.size()) )
    {
        std::cout << "Error getting tile " << tileid << ", outside range of available map tiles!\n";
        return m_MapTiles[0];
    }

    return m_MapTiles[tileid];
}

bool Engine::validWalkableTile(int x, int y)
{
    sf::Vector2i tpos(x,y);

    //check that tile is within map dimensions
    sf::Vector2i mapdim = testmap->getDimensions();
    if(x < 0 || y < 0 || x >= mapdim.x || y >= mapdim.y) return false;

    //check that tile is walkable
    MapTile *ttile = getMapTile( testmap->getTile(x,y) );
    if(!ttile->isWalkable()) return false;

    //check that there are no actors there
    std::vector<Monster*> *actors = testmap->getMapMonsters();
    for(int i = 0; i < int(actors->size()); i++)
    {
        if((*actors)[i]->getPosition() == tpos) return false;
    }

    //check if player is there
    if( m_Player->getPosition() == tpos) return false;

    return true;
}

int Engine::getDirectionFromUser(sf::Vector2i *mcoord)
{
    bool quit = false;
    int dir = DIR_NONE;

    sf::Vector2i npos;
    //if coordinate supplied, copy coordinate information
    //this is used to make adjustments using direction from player
    if(mcoord != NULL)
    {
        npos.x = mcoord->x;
        npos.y = mcoord->y;
    }

    while(!quit)
    {
        sf::Event event;

        //handle events
        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Numpad4)
                {
                    dir = DIR_WEST;
                    npos.x--;
                }
                else if(event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Numpad6)
                {
                    dir = DIR_EAST;
                    npos.x++;
                }
                else if(event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Numpad8)
                {
                    dir = DIR_NORTH;
                    npos.y--;
                }
                else if(event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Numpad2)
                {
                    dir = DIR_SOUTH;
                    npos.y++;
                }
                else if(event.key.code == sf::Keyboard::Numpad1)
                {
                    dir = DIR_SW;
                    npos.x--;
                    npos.y++;
                }
                else if(event.key.code == sf::Keyboard::Numpad3)
                {
                    dir = DIR_SE;
                    npos.x++;
                    npos.y++;
                }
                else if(event.key.code == sf::Keyboard::Numpad7)
                {
                    dir = DIR_NW;
                    npos.x--;
                    npos.y--;
                }
                else if(event.key.code == sf::Keyboard::Numpad9)
                {
                    dir = DIR_NE;
                    npos.x++;
                    npos.y--;
                }

                quit = true;
            }
        }

    }

    if(mcoord != NULL)
    {
        *mcoord = npos;
    }

    return dir;
}

///////////////////////////////////////////////////////////////////
Monster *Engine::createMonster(int monsterid)
{
    if(monsterid < 0 || monsterid >= getMonsterDBSize())
    {
        std::cout << "Error copying monster, monster ID out of range!\n";
        return NULL;
    }

    Monster *newmonster = new Monster();
    *newmonster = *m_MonsterDB[monsterid];

    return newmonster;
}

Item *Engine::createItem(int itemid)
{
    if(itemid < 0 || itemid > getItemDBSize())
    {
        std::cout << "Error creating item.  Item ID is out of range!\n";
        return NULL;
    }

    Item *newitem = NULL;

    //determine which type to create
    switch( m_ItemDB[itemid]->getType())
    {
    case OBJ_ITEM_CONTAINER:
        newitem = new Container();
        //*newitem = *m_ItemDB[itemid];
        break;
    case OBJ_ITEM_CONTAINER_LIQUID:
        newitem = new ContainerLiquid();
        //*newitem = *m_ItemDB[itemid];
        break;
    case OBJ_ITEM_MISC:
        newitem = new MiscItem;
        //*newitem = *m_ItemDB[itemid];
        break;
    default:
        std::cout << "Error creating item, unrecognized type!\n";
        break;
    }

    if(newitem == NULL) return false;
    else *newitem = *m_ItemDB[itemid];

    return newitem;
}

bool Engine::moveItem(Item *titem, std::vector<Item*> *isource, std::vector<Item*> *idest)
{
    if(isource == NULL || idest == NULL || titem == NULL)
    {
        std::cout << "Error moving item.  Item, source, or destionation is not valid!\n";
        return false;
    }

    //find item in source list
    for(int i = int(isource->size()-1); i >= 0; i--)
    {
        //target object found in source list
        if(titem == (*isource)[i])
        {
            //remove from source list
            isource->erase(isource->begin() + i);
            break;
        }

        if(i == 0)
        {
            std::cout << "Error in moveitem, unable to find target item in source list\n";
            return false;
        }
    }

    //add item to destination list
    idest->push_back(titem);


    std::cout << "Moved item " << titem->getName() << " to destination list.\n";
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////
std::vector< std::vector<int> > Engine::genNoise(int width, int height, int xoffset, int yoffset,
                                float persistence, float octaves, float scale, int minval, int maxval)
{
    //seed
    //srand(m_Seed);

    //create a 2d array to store noise map
    std::vector< std::vector<int> > noisemap;
    noisemap.resize(height);
    for(int i = 0; i < height; i++) noisemap[i].resize(width);

    //clear all data in array
    for(int i = 0; i < int(noisemap.size()); i++)
    {
        for(int n = 0; n < int(noisemap[0].size()); n++)
        {
            noisemap[i][n] = 0;
        }
    }

    //generate noise
    for(int i = 0; i < int(noisemap.size()); i++)
    {
        for(int n = 0; n < int(noisemap[0].size()); n++)
        {
            noisemap[i][n] = scaled_octave_noise_2d(octaves, persistence, scale, minval, maxval, xoffset+n, yoffset+i);
        }
    }

    return noisemap;
}


////////////////////////////////////////////////////////////////////////////////////////
void Engine::debugtest()
{
    std::vector<Item*> *inv = m_Player->getInventory();

    for(int i = 0; i < int(inv->size()); i++)
    {
        std::vector<std::string> teststr = (*inv)[i]->saveDataToString();

        for(int n = 0; n < int(teststr.size()); n++) std::cout << teststr[n];
    }
}

void Engine::noisetest()
{
    bool quit = false;
    bool needsrefresh = false;
    bool drawmask = false;
    bool drawthreshold = false;
    enum _MODES{MODE_SCALE, MODE_PERSISTENCE, MODE_OCTAVES};
    int xshift = 0;
    int yshift = 0;
    int mode = MODE_SCALE;

    float scale = 9;
    float scale_unit = 0.5;
    float octaves = 6;
    float octaves_unit = 1.0;
    float persistence = 0.7;
    float persistence_unit = 0.1;
    int width = m_ScreenTilesHeight;
    int height = m_ScreenTilesHeight;

    //find smallest dimension for mask map
    int mdim = width;
    int threshold = 60;
    float maskintensity = 1.f; // percentage 1.0 = 100%
    if(height < width) mdim = height;
    sf::Vector2i mcenter(mdim/2, mdim/2);
    float centerdistance = getDistance(0,0, mcenter.x, mcenter.y);
    //float gradientdist = centerdistance * maskintensity;
    float scaler = 255.f / centerdistance;



    std::vector< std::vector<int> > noisemap = genNoise(width, height, 0,0, persistence, octaves, scale);
    std::vector< std::vector<int> > maskmap;

    //create mask map
    maskmap.resize(mdim);
    for(int i = 0; i < mdim; i++)
    {
        for(int n = 0; n < mdim; n++) maskmap[i].push_back(0);
    }

    //creat mask map gradient
    for(int i = 0; i < mdim; i++)
    {
        for(int n = 0; n < mdim; n++)
        {
            //if(getDistance(mcenter.x, mcenter.y, n, i) > gradientdist)
            maskmap[i][n] = getDistance(mcenter.x, mcenter.y, n, i) * scaler;
        }
    }

    //draw noise loop

    sf::RectangleShape tile(sf::Vector2f(m_TileWidth, m_TileHeight));

    while(!quit)
    {
        m_Screen->clear();

        sf::Event event;

        while(m_Screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Left) xshift--;
                else if(event.key.code == sf::Keyboard::Right) xshift++;
                else if(event.key.code == sf::Keyboard::Up) yshift--;
                else if(event.key.code == sf::Keyboard::Down) yshift++;
                else if(event.key.code == sf::Keyboard::S) mode = MODE_SCALE;
                else if(event.key.code == sf::Keyboard::P) mode = MODE_PERSISTENCE;
                else if(event.key.code == sf::Keyboard::O) mode = MODE_OCTAVES;
                else if(event.key.code == sf::Keyboard::M) drawmask = !drawmask;
                else if(event.key.code == sf::Keyboard::T) drawthreshold = !drawthreshold;
                else if(event.key.code == sf::Keyboard::Add)
                {
                    switch(mode)
                    {
                    case MODE_SCALE:
                        scale += scale_unit;
                        break;
                    case MODE_PERSISTENCE:
                        persistence += persistence_unit;
                        break;
                    case MODE_OCTAVES:
                        octaves += octaves_unit;
                        break;
                    }
                }
                else if(event.key.code == sf::Keyboard::Subtract)
                {
                    switch(mode)
                    {
                    case MODE_SCALE:
                        scale -= scale_unit;
                        break;
                    case MODE_PERSISTENCE:
                        persistence -= persistence_unit;
                        break;
                    case MODE_OCTAVES:
                        octaves -= octaves_unit;
                        break;
                    }
                }
                needsrefresh = true;

            }
        }

        if(needsrefresh)
        {
            std::cout << "mode = " << mode << std::endl;
            std::cout << "x = " << xshift << "      y = " << yshift << std::endl;
            std::cout << "persistence = " << persistence << std::endl;
            std::cout << "scale       = " << scale << std::endl;
            std::cout << "octaves     = " << octaves << std::endl;
            std::cout << "\n\n";

            noisemap = genNoise(width, height, 0 + xshift, 0 + yshift, persistence, octaves, scale);
            needsrefresh = false;

            //create mask if necessary
            if(drawmask)
            {
                for(int i = 0; i < mdim; i++)
                {
                    for(int n = 0; n < mdim; n++)
                    {
                        noisemap[i][n] -= maskmap[i][n];

                        //if applying threshold
                        if(drawthreshold)
                        {
                            if(noisemap[i][n] < threshold) noisemap[i][n] = 0;
                            else noisemap[i][n] = 255;
                        }
                    }
                }



            }
        }


        //draw
        for(int i = 0; i < int(noisemap.size()); i++)
        {
            for(int n = 0; n < int(noisemap[0].size()); n++)
            {
                tile.setPosition(n*m_TileWidth, i*m_TileHeight);
                int shade = noisemap[i][n];
                if(shade < 0) shade = 0;

                tile.setFillColor(sf::Color(shade,shade,shade));

                m_Screen->draw(tile);
            }
        }

        //display
        m_Screen->display();

    }
}
