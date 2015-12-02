#ifndef SFML_CURS
#define SFML_CURS

#include <SFML\Graphics.hpp>

//Expecting a sprite sheet that is 16x16 sprites, change this if using something else
#define SFMLCURS_WIDTH 16
#define SFMLCURS_HEIGHT 16

class SFMLCurs
{
private:

    //init
    bool initColors();
    bool initSprites();

    //resources
    std::vector< std::vector< sf::Texture> > m_Textures;
    std::vector <std::vector< std::vector< sf::Sprite> > > m_Sprites;

    //colors
    std::vector<sf::Color> m_AsciiColors;

public:
    SFMLCurs();
    ~SFMLCurs();

};

#endif // SFML_CURS
