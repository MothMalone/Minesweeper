#include <chrono>
#include <random>
#include <fstream>
#include <string>

#include <SFML/Graphics.hpp>

#include "Headers/Cell.hpp"
#include "Headers/DrawText.hpp"
#include "Headers/Field.hpp"
#include "Headers/GetCell.hpp"
#include "Headers/Global.hpp"
#include "Headers/Game.hpp"
#include "Headers/Button.hpp"


struct Hailstone {
    sf::Vector2f position;
    float speed;
    float size;
    float angle;
    float opacity;
};




const int scrWidth = CELL_SIZE * COLUMNS * SCREEN_RESIZE;
const int scrHeight = SCREEN_RESIZE * (FONT_HEIGHT + CELL_SIZE * COLUMNS);



int main()
{

    sf::RenderWindow window(sf::VideoMode(scrWidth, scrHeight), "Minesweeper", sf::Style::Close);
    
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Resources/Images/background.png")) {
        // Handle error if the texture fails to load
        return 1;
    }

    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setPosition(0, 0);
    sf::Color backgroundColor = backgroundSprite.getColor();
    backgroundColor.r = static_cast<sf::Uint8>(backgroundColor.r * 0.69f); // Reduce red channel by 20%
    backgroundColor.g = static_cast<sf::Uint8>(backgroundColor.g * 0.69f); // Reduce green channel by 20%
    backgroundColor.b = static_cast<sf::Uint8>(backgroundColor.b * 0.69f); // Reduce blue channel by 20%
    backgroundSprite.setColor(backgroundColor);
    
    // Create a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> positionDistX(-100.0f, static_cast<float>(window.getSize().x + 100.0f));
    std::uniform_real_distribution<float> positionDistY(-100.0f, static_cast<float>(window.getSize().y + 100.0f));
    std::uniform_real_distribution<float> speedDist(15.0f, 20.0f);
    std::uniform_real_distribution<float> sizeDist(2.0f, 3.0f);
    std::uniform_real_distribution<float> angleDist(-45.0f, 45.0f);
    std::uniform_real_distribution<float> opacityDist(0.2f, 0.4f);
    // Create the hailstones
    const int numHailstones = 420;
    std::vector<Hailstone> hailstones(numHailstones);
    for (auto& hailstone : hailstones) {
        hailstone.position.x = positionDistX(gen);
        hailstone.position.y = positionDistY(gen);
        hailstone.speed = speedDist(gen);
        hailstone.size = sizeDist(gen);
        hailstone.angle = angleDist(gen);
        hailstone.opacity = opacityDist(gen);
    }



    sf::Texture timeTexture;
    if (!timeTexture.loadFromFile("Resources/Images/BestTime.png")) {
        return 1;
    }
    sf::Texture timeBoardTexture;
    if (!timeBoardTexture.loadFromFile("Resources/Images/board.png")) {
        return 1;
    }


    

    Button playButton("Resources/Images/PlayButton.png", {(float)scrWidth / 2, (float)scrHeight / 2 + 50}, {0.2f, 0.2f});
    Button easyButton("Resources/Images/Easy.png", {165.f, 120.f});
    Button normalButton("Resources/Images/Normal.png", {165.f, 220.f});
    Button hardButton("Resources/Images/Hard.png", {165.f, 320.f});
    Button bestButton("Resources/Images/BestButton.png", {(float)scrWidth / 2 , (float)scrHeight / 2 + 110}, {0.2f, 0.2f});
    Button returnButton("Resources/Images/ReturnButton.png", {450.f, 500.f}, {0.5f, 0.5f});

    sf::RectangleShape popupBox;
    popupBox.setSize(sf::Vector2f(300.f, 150.f));
    //color picker to match background :3
    popupBox.setFillColor(sf::Color(48, 62, 68, 230));
    popupBox.setScale(1, 2);
    popupBox.setOrigin(popupBox.getLocalBounds().width / 2, popupBox.getLocalBounds().height);
    //some more magic numbers
    popupBox.setPosition(scrWidth/2, scrHeight - 170);


    // Set up the bTime sprite
    sf::Sprite bTime;
    bTime.setTexture(timeTexture);
    // Reduce the red, green, and blue channels of the color by 20% (0.8) to make it a bit darker
    sf::Color bTimeColor = bTime.getColor();
    bTimeColor.r = static_cast<sf::Uint8>(bTimeColor.r * 0.69f);
    bTimeColor.g = static_cast<sf::Uint8>(bTimeColor.g * 0.69f);
    bTimeColor.b = static_cast<sf::Uint8>(bTimeColor.b * 0.69f);
    bTime.setColor(bTimeColor);
    bTime.setScale(0.6, 0.8);
    bTime.setOrigin(bTime.getLocalBounds().width / 2, bTime.getLocalBounds().height);
    bTime.setPosition(scrWidth /2, scrHeight - 450);

    // Set up the TimeBoard sprite
    sf::Sprite TimeBoard;
    TimeBoard.setTexture(timeBoardTexture);
    // Reduce the red, green, and blue channels of the color by 20% (0.8) to make it a bit darker
    sf::Color timeBoardColor = TimeBoard.getColor();
    timeBoardColor.r = static_cast<sf::Uint8>(timeBoardColor.r * 0.69f);
    timeBoardColor.g = static_cast<sf::Uint8>(timeBoardColor.g * 0.69f);
    timeBoardColor.b = static_cast<sf::Uint8>(timeBoardColor.b * 0.69f);
    TimeBoard.setColor(timeBoardColor);
    TimeBoard.setScale(2.7, 3.3);
    TimeBoard.setOrigin(TimeBoard.getLocalBounds().width / 2, TimeBoard.getLocalBounds().height);
    TimeBoard.setPosition(scrWidth/2, scrHeight + 65);

    bool showPopUp = false;
    bool showBest = false;

    std::string bestTimeText;
    // Load the best time text
    std::ifstream bestTimeFile("Resources/besttime.txt");
    if (bestTimeFile.is_open()) {
        std::string line;
        while (std::getline(bestTimeFile, line)) {
            bestTimeText += line + "\n";
        }
        bestTimeFile.close();
    } 

    while (window.isOpen())
    {
    // Reset button states
        playButton.reset();
        easyButton.reset();
        normalButton.reset();
        hardButton.reset();
        bestButton.reset();

        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseMoved)
            {
                if (!showPopUp) {
                    playButton.handleEvents(window, event);
                    bestButton.handleEvents(window, event);
                    returnButton.handleEvents(window, event);
                }
                easyButton.handleEvents(window, event);
                normalButton.handleEvents(window, event);
                hardButton.handleEvents(window, event);
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                if (playButton.handleEvents(window, event) && !showPopUp)
                {
                    showPopUp = true;
                }
                else if (bestButton.handleEvents(window, event) && !showPopUp)
                {
                    showBest = true;
                }
                else if (showBest) {
                    if (returnButton.handleEvents(window, event)) {
                        showBest = false;
                        window.setView(window.getDefaultView());
                    }
                }
                else if (showPopUp) 
                {
                    if (easyButton.handleEvents(window, event))
                    {
                        unsigned char newColumns = 8;
                        unsigned char newRows = 8;
                        unsigned short numMines = 10;
                        startGame(window, newColumns, newRows, numMines, "EASY");
                       
                    }
                    else if (normalButton.handleEvents(window, event))
                    {
                        unsigned char newColumns = 10;
                        unsigned char newRows = 10;
                        unsigned short numMines = 20;
                        startGame(window, newColumns, newRows, numMines, "NORMAL");
                        
                    }
                    else if (hardButton.handleEvents(window, event))
                    {
                        unsigned char newColumns = 13;
                        unsigned char newRows = 13;
                        unsigned short numMines = 30;
                        startGame(window, newColumns, newRows, numMines, "HARD");
                       
                    }
                }
            }
            else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::Escape)
            {
                showBest = false;
                showPopUp = false;
                window.setView(window.getDefaultView());
            }
        }

        // Update the hailstones
        float deltaTime = 1.0f / 60.0f; // Assuming 60 FPS
        for (auto& hailstone : hailstones) {
            hailstone.position.x += hailstone.speed * std::cos(hailstone.angle * M_PI / 180.0f) * deltaTime;
            hailstone.position.y += hailstone.speed * std::sin(hailstone.angle * M_PI / 180.0f) * deltaTime;

            // Keep the hailstones within the screen boundaries
            if (hailstone.position.x < -100.0f || hailstone.position.x > static_cast<float>(window.getSize().x) + 100.0f ||
                hailstone.position.y < -100.0f || hailstone.position.y > static_cast<float>(window.getSize().y) + 100.0f) {
                // Respawn the hailstone
                hailstone.position.x = positionDistX(gen);
                hailstone.position.y = positionDistY(gen);
                hailstone.speed = speedDist(gen);
                hailstone.angle = angleDist(gen);
            }
        }


        // Draw the main menu
        window.clear();
        window.draw(backgroundSprite);
        for (const auto& hailstone : hailstones) {
            sf::CircleShape stoneShape(hailstone.size);
            stoneShape.setPosition(hailstone.position.x - hailstone.size, hailstone.position.y - hailstone.size);
            stoneShape.setFillColor(sf::Color(255, 255, 255, static_cast<int>(hailstone.opacity * 255.0f)));
            stoneShape.setRotation(hailstone.angle);
            window.draw(stoneShape);
        }
        draw_text(true, scrWidth / 2 - 170, scrHeight / 2 - 50, "Minesweeper", window, 60);
        playButton.drawHovered(window);
        bestButton.drawHovered(window);

        if (showBest) {
            // Clear the screen
            window.clear();
            window.draw(backgroundSprite);
           
            window.draw(TimeBoard);
            window.draw(bTime);
            
            // Draw the best time text
            draw_text(true, scrWidth / 2 - 200, scrHeight / 2 - 70, bestTimeText, window, 55);
            returnButton.drawHovered(window);
            
        }

        if (showPopUp) {
            window.draw(popupBox);
            easyButton.drawHovered(window);
            normalButton.drawHovered(window);
            hardButton.drawHovered(window);       
        }
        window.display();
    }
    return 0;
}
    