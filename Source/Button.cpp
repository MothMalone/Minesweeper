
#include <SFML/Graphics.hpp>
#include "Headers/Button.hpp"
#include <string>

Button::Button(const std::string& texturePath, const sf::Vector2f& position, const sf::Vector2f& scale)
    : m_isHovered(false), m_isClicked(false) {
    if (!m_texture.loadFromFile(texturePath)) {
        // Handle error if the texture fails to load
        return;
    }
    m_sprite.setTexture(m_texture);
    m_sprite.setScale(scale);
    if (scale != sf::Vector2f(1.f, 1.f)) 
        m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2, m_sprite.getLocalBounds().height / 2);
    m_sprite.setPosition(position);
}



bool Button::isHovered() const {
    return m_isHovered;
}

bool Button::isClicked() const {
    return m_isClicked;
}
 
//Handle hovering and clicking events
bool Button::handleEvents(const sf::RenderWindow& window, const sf::Event & event) {
    m_isHovered = m_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
    if (event.type == sf::Event::MouseButtonReleased)
    {   
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            m_isClicked = m_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
            return m_isClicked;
        }
    }
    return false;
}


//just to draw the hovered event
void Button::drawHovered(sf::RenderWindow& window) {
    if (m_isHovered) {
        sf::Color color = m_sprite.getColor();
        color.a = 195;
        m_sprite.setColor(color);
    } else {
        m_sprite.setColor(sf::Color::White);
    }
    window.draw(m_sprite);
}

 void Button::reset() {
    m_isClicked = false;
}