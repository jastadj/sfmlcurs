#include "sfmlcurs.hpp"

SFMLCurs::SFMLCurs()
{

}

SFMLCurs::~SFMLCurs()
{

}

bool SFMLCurs::initColors()
{
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

bool SFMLCurs::initSprites()
{
    //color mask for foreground and background
    sf::Color colormaskbg(0xff,0x00,0xff);
    sf::Color colormaskfg(0xff,0xff,0xff);

    //create texture for each color combination
    //background color loop
    for(int i = 0; i < COLOR_TOTAL; i++)
    {
        //add column
        m_Textures.resize(m_Textures.size()+1);

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

            m_Textures[i].push_back(newtxt);
        }
    }

    //clip all the characters for each character type from bg/fg combos
    for(int i = 0; i < m_TileSheetWidth*m_TileSheetHeight; i++)
    {
        //add new character index
        m_Sprites.resize( m_Sprites.size()+1);

        //for each character, go through each color combination and clip to a sprite
        //bg color loop
        for(int n = 0; n < COLOR_TOTAL; n++)
        {
            //add new background index
            m_Sprites[i].resize( m_Sprites[i].size()+1);

            //fg color loop
            for(int p = 0; p < COLOR_TOTAL; p++)
            {
                sf::IntRect clip;

                clip.width = m_TileWidth;
                clip.height = m_TileHeight;
                clip.left = (i - ( (i/m_TileSheetWidth) * m_TileSheetWidth ) ) * m_TileWidth;
                clip.top = (i/m_TileSheetWidth) * m_TileHeight;

                sf::Sprite newsprite(m_Textures[n][p], clip);

                //debug
                //m_Screen->draw(newsprite);
                //m_Screen->display();

                m_Sprites[i][n].push_back(newsprite);
            }
        }
    }

    return true;
}
