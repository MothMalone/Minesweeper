#include <SFML/Graphics.hpp>

#include "Headers/DrawText.hpp"

void draw_text(bool i_black, unsigned short i_x, unsigned short i_y, const std::string& i_text, sf::RenderWindow& i_window, unsigned int font_size)
{
    // We're gonna align the text to the left top
    short character_x = i_x;
    short character_y = i_y;

    sf::Sprite character_sprite;

    sf::Texture font_texture;
    font_texture.loadFromFile("Resources/Images/Font.png");

    // Calculate the character width and height based on the font size
    unsigned int character_width = font_texture.getSize().x / 96;
    unsigned int character_height = font_texture.getSize().y;

    // Calculate the scale factor based on the desired font size and the original character height
    float scale_factor = static_cast<float>(font_size) / character_height;

    character_sprite.setTexture(font_texture);

    // We can use this to change the color of the text
    if (1 == i_black)
    {
        character_sprite.setColor(sf::Color(0, 0, 0));
    }

    for (const char a : i_text)
    {
        if ('\n' == a)
        {
            // After every newline, we increase the y-coordinate and reset the x-coordinate
            character_x = i_x;
            character_y += character_height * scale_factor;

            continue;
        }

        // Change the position and scale of the next character
        character_sprite.setPosition(character_x, character_y);
        character_sprite.setScale(scale_factor, scale_factor);

        // Pick the character from the font image
        character_sprite.setTextureRect(sf::IntRect(character_width * (a - 32), 0, character_width, character_height));

        // Increase the x-coordinate
        character_x += character_width * scale_factor;

        // Draw the character
        i_window.draw(character_sprite);
    }
}