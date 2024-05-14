#pragma once

#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <string>



void startGame(sf::RenderWindow& window, unsigned char newColumns, unsigned char newRows, unsigned short numMines, const std::string& difficulty);

#endif