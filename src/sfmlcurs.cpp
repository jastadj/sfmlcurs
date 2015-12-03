#include "sfmlcurs.hpp"
#include <iostream>

SFMLCurs::SFMLCurs(std::string spritesheetfile, int char_scale, bool isSmooth)
{
    //set default flags
    m_Initialized = false;
    m_QuietMode = false;
    m_SmoothTextures = false;

    //set sprite sheet file to use
    m_SpriteSheetFile = spritesheetfile;

    //use smooth textures?
    m_SmoothTextures = isSmooth;

    //set character scale
    m_Scale = char_scale;
    if(m_Scale < 1) m_Scale = 1;

}

SFMLCurs::~SFMLCurs()
{

}

bool SFMLCurs::init()
{
    //check if already initialized
    if(m_Initialized)
    {
        printStatus("Already initialized!\n");
        return m_Initialized;
    }

    //initialize colors
    initColors();

    //initialize sprites
    if(!initSprites())
    {
        printStatus("Error initializing sprites.\n");
        return m_Initialized;
    }

    //initialization successful
    m_Initialized = true;

    return m_Initialized;
}

bool SFMLCurs::initColors()
{
    printStatus("SFMLCurs:Initializing colors...");

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
    m_AsciiColors.push_back(sf::Color(0x00, 0x00, 0x00, 0xff)); // transparent

    //check that initialized color count matches enumerated color count
    if( int(m_AsciiColors.size()) != SFC_TOTAL_COLORS)
    {
        printStatus("colors failed to initialize properly.\n");
        return false;
    }

    printStatus("done.\n");
    return true;
}

//initialize all the ascii graphics for each available color fore/background color
bool SFMLCurs::initSprites()
{
    printStatus("SFMLCurs:Initializing sprites...");

    //color mask for foreground and background
    sf::Color colormaskbg(0xff,0x00,0xff);
    sf::Color colormaskfg(0xff,0xff,0xff);

    //load sprite sheet image from file
    sf::Image spritesheetimage;
    if(!spritesheetimage.loadFromFile(m_SpriteSheetFile))
    {
        printStatus(std::string("\nFailed to load sprite sheet image : ") + m_SpriteSheetFile);
        return false;
    }

    sf::Vector2u imagesize = spritesheetimage.getSize();
    m_TileSize = sf::Vector2u( imagesize.x / SFMLCURS_WIDTH, imagesize.y / SFMLCURS_HEIGHT);

    printStatus("creating textures...");

    //create texture for each color combination
    //background color loop
    for(int i = 0; i < SFC_TOTAL_COLORS; i++)
    {
        //add column
        m_Textures.resize(m_Textures.size()+1);

        //foreground color loop
        for(int n = 0; n < SFC_TOTAL_COLORS; n++)
        {
            //texture for image
            sf::Texture newtxt;

            //set smoothing
            newtxt.setSmooth(m_SmoothTextures);

            //create temporary copy of image file for manipulation
            sf::Image newimage = spritesheetimage;
            //newimage.loadFromFile("tileart.png");

            //set foreground color
            replaceImageColor(&newimage, colormaskfg, m_AsciiColors[n]);

            //set background color
            replaceImageColor(&newimage, colormaskbg, m_AsciiColors[i]);

            //set texture from image
            newtxt.loadFromImage(newimage);

            //add colorized texture to list
            m_Textures[i].push_back(newtxt);
        }
    }

    printStatus("creating sprites...");

    //clip all the characters for each character type from bg/fg combos
    for(unsigned int i = 0; i < (SFMLCURS_WIDTH * SFMLCURS_HEIGHT); i++)
    {
        //add new character index
        m_Sprites.resize( m_Sprites.size()+1);

        //for each character, go through each color combination and clip to a sprite
        //bg color loop
        for(int n = 0; n < SFC_TOTAL_COLORS; n++)
        {
            //add new background index
            m_Sprites[i].resize( m_Sprites[i].size()+1);

            //fg color loop
            for(int p = 0; p < SFC_TOTAL_COLORS; p++)
            {
                sf::IntRect clip;

                clip.width = m_TileSize.x;
                clip.height = m_TileSize.y;
                clip.left = (i - (  ( i / SFMLCURS_WIDTH ) * SFMLCURS_WIDTH ) ) * m_TileSize.x;
                clip.top = ( i / SFMLCURS_WIDTH ) * m_TileSize.y;

                sf::Sprite newsprite(m_Textures[n][p], clip);
                newsprite.setScale(m_Scale, m_Scale);

                m_Sprites[i][n].push_back(newsprite);
            }
        }
    }

    printStatus("done.\n");

    return true;
}

sf::Vector2u SFMLCurs::getTileSize()
{
    return sf::Vector2u( m_TileSize.x * m_Scale, m_TileSize.y * m_Scale);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  DRAW FUNCTIONS
void SFMLCurs::draw(sf::RenderTarget *rendertarget, int x, int y, int ascii_index, int fgcolor, int bgcolor)
{
    if(rendertarget == NULL) return;

    //check if ascii index is out or range
    if(ascii_index < 0 || ascii_index >= SFMLCURS_WIDTH * SFMLCURS_HEIGHT)
    {
        char buf[256];

        printStatus("SFMLCURS:Error drawing ascii index.  Index out of range : ");
        printStatus(std::string( itoa(ascii_index, buf,10) ) );
        printStatus("\n");
        return;
    }

    //position and draw sprite
    m_Sprites[ascii_index][bgcolor][fgcolor].setPosition( x * m_TileSize.x * m_Scale, y * m_TileSize.y * m_Scale);

    rendertarget->draw( m_Sprites[ascii_index][bgcolor][fgcolor] );
}

void SFMLCurs::draw(sf::RenderTarget *rendertarget, int x, int y, char ascii_char, int fgcolor, int bgcolor)
{
    draw(rendertarget, x, y, int(ascii_char), fgcolor, bgcolor);
}

void SFMLCurs::drawString(sf::RenderTarget *rendertarget, int x, int y, std::string astring, int fgcolor, int bgcolor)
{
    for(int i = 0; i < int(astring.size()); i++)
    {
        draw(rendertarget, x+i, y, int(astring[i]), fgcolor, bgcolor);
    }
}

void SFMLCurs::drawAtPixel(sf::RenderTarget *rendertarget, int x, int y, int ascii_index, int fgcolor, int bgcolor)
{
    if(rendertarget == NULL) return;

    //check if ascii index is out or range
    if(ascii_index < 0 || ascii_index >= SFMLCURS_WIDTH * SFMLCURS_HEIGHT)
    {
        char buf[256];

        printStatus("SFMLCURS:Error drawing ascii index.  Index out of range : ");
        printStatus(std::string( itoa(ascii_index, buf,10) ) );
        printStatus("\n");
        return;
    }

    //position and draw sprite
    m_Sprites[ascii_index][bgcolor][fgcolor].setPosition( x * m_Scale, y * m_Scale);

    rendertarget->draw( m_Sprites[ascii_index][bgcolor][fgcolor] );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TOOL / UTILITY FUNCTIONS

//this function takes in an image and replaces a pixel color with new pixel color
void SFMLCurs::replaceImageColor(sf::Image *targ_image, sf::Color old_color, sf::Color new_color)
{
    if(targ_image == NULL) return;

    //get the image height and width
    int x = targ_image->getSize().x;
    int y = targ_image->getSize().y;

    //check each pixel of image, look for old color, and set to new color
    for(int i = 0; i < y; i++)
    {
        for(int n = 0; n < x; n++)
        {
            if(targ_image->getPixel(n,i) == old_color) targ_image->setPixel(n,i, new_color);
        }
    }
}

void SFMLCurs::printStatus(std::string statstr)
{
    if(m_QuietMode) return;

    else std::cout << statstr;
}

sf::Sprite SFMLCurs::copyCharacter(int ascii_index, int fgcolor, int bgcolor)
{
    return m_Sprites[ascii_index][bgcolor][fgcolor];
}
