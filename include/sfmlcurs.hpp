#ifndef SFML_CURS
#define SFML_CURS

#include <SFML\Graphics.hpp>
#include <string>

//Expecting a sprite sheet that is 16x16 sprites, change this if using something else
#define SFMLCURS_WIDTH 16
#define SFMLCURS_HEIGHT 16

//color index enumeration is as follows
enum {
        SFC_BLACK,
        SFC_WHITE,
        SFC_WHITE_B,
        SFC_RED,
        SFC_RED_B,
        SFC_BLUE,
        SFC_BLUE_B,
        SFC_GREEN,
        SFC_GREEN_B,
        SFC_CYAN,
        SFC_CYAN_B,
        SFC_MAGENTA,
        SFC_MAGENTA_B,
        SFC_YELLOW,
        SFC_YELLOW_B,
        SFC_TRANSPARENT,
        SFC_TOTAL_COLORS
     };


class SFMLCurs
{
private:

    //init
    bool m_Initialized;
    bool m_QuietMode;
    bool m_SmoothTextures;
    int m_Scale;
    bool initColors();
    bool initSprites();

    //resources
    std::string m_SpriteSheetFile;
    std::vector< std::vector< sf::Texture> > m_Textures;
    std::vector <std::vector< std::vector< sf::Sprite> > > m_Sprites;
    sf::Vector2u m_TileSize;

    //colors
    std::vector<sf::Color> m_AsciiColors;

    //tools
    void replaceImageColor(sf::Image *targ_image, sf::Color old_color, sf::Color new_color);
    void printStatus(std::string statstr);

public:
    SFMLCurs(std::string spritesheetfile, int char_scale = 1, bool isSmooth = false);
    ~SFMLCurs();

    bool init();

    //set whether or not to print status messages to console
    void setQuietMode(bool newquietmode) { m_QuietMode = newquietmode;}
    bool isQuietMode() { return m_QuietMode;}

    sf::Vector2u getTileSize();

    //draw functions
    void draw(sf::RenderTarget *rendertarget, int x, int y, int ascii_index,
            int fgcolor = SFC_WHITE, int bgcolor = SFC_BLACK);
    void draw(sf::RenderTarget *rendertarget, int x, int y, char ascii_char,
            int fgcolor = SFC_WHITE, int bgcolor = SFC_BLACK);
    void drawString(sf::RenderTarget *rendertarget, int x, int y, std::string astring,
            int fgcolor = SFC_WHITE, int bgcolor = SFC_BLACK);
    void drawAtPixel(sf::RenderTarget *rendertarget, int x, int y, int ascii_index,
            int fgcolor = SFC_WHITE, int bgcolor = SFC_BLACK);

    //
    sf::Sprite copyCharacter(int ascii_index, int fgcolor, int bgcolor);
};

#endif // SFML_CURS
