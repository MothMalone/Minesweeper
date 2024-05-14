#pragma once

#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics.hpp>
#include <string>

class Button {
public:
    Button(const std::string& texturePath, const sf::Vector2f& position, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f));

    bool isHovered() const;
    bool isClicked() const;

    bool handleEvents(const sf::RenderWindow& window, const sf::Event & event);
    void drawHovered(sf::RenderWindow& window);
    void reset();

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_isHovered;
    bool m_isClicked;
};

#endif // BUTTON_H