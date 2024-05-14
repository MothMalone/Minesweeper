#include <chrono>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>


#include "Headers/Cell.hpp"
#include "Headers/DrawText.hpp"
#include "Headers/Field.hpp"
#include "Headers/GetCell.hpp"
#include "Headers/Global.hpp"
#include "Headers/Game.hpp"



// Function to convert time string to seconds
int timeToSeconds(const std::string& timeStr) {
    int hours, minutes, seconds;
    char colon;
    std::stringstream ss(timeStr);
    ss >> hours >> colon >> minutes >> colon >> seconds;
    return hours * 3600 + minutes * 60 + seconds;
}
//function to save best time
void saveBestTime(const std::string& difficulty, const std::string& bestTime) {
    // Open the best time file
    std::vector<std::string> bestTimes;
    std::ifstream bestTimeFile("Resources/besttime.txt");
    if (bestTimeFile) {
        std::string line;
        while (std::getline(bestTimeFile, line)) {
            bestTimes.push_back(line);
        }
        bestTimeFile.close();
    }

    // Update the best time based on the difficulty
    if (difficulty == "EASY") {
        bestTimes[0] = bestTime;
    } else if (difficulty == "NORMAL") {
        bestTimes[1] = bestTime;
    } else if (difficulty == "HARD") {
        bestTimes[2] = bestTime;
    }

    // Save the updated best times to the file
    std::ofstream bestTimeFileOut("Resources/besttime.txt");
    if (bestTimeFileOut) {
        for (const auto& line : bestTimes) {
            bestTimeFileOut << line << "\n";
        }
        bestTimeFileOut.close();
    }
}


void startGame(sf::RenderWindow& window, unsigned char newColumns, unsigned char newRows, unsigned short numMines, const std::string& difficulty) {
    COLUMNS = newColumns;
    ROWS = newRows;
    MINES = numMines;

    //Used to make the game framerate-independent
    unsigned lag = 0;

    //Alexander has 4 states:
    //0) Idle
    //1) When you're about to open a cell
    //2) When the game is over and you've lost
    //3) When the game is over and you've won
    unsigned char Alexander_state = 0;

    //Similar to lag, used to make the game framerate-independent
    std::chrono::time_point<std::chrono::steady_clock> previous_time;

    sf::Event event;

    while(window.pollEvent(event));

    //SFML window
    //sf::RenderWindow window(sf::VideoMode(CELL_SIZE * COLUMNS * SCREEN_RESIZE, SCREEN_RESIZE * (FONT_HEIGHT + CELL_SIZE * COLUMNS)), "Minesweeper", sf::Style::Close);
    //Here we're resizing the window

    sf::Vector2u newSize(CELL_SIZE * COLUMNS * SCREEN_RESIZE, SCREEN_RESIZE * (FONT_HEIGHT + CELL_SIZE * COLUMNS ));

    window.setSize(newSize);
    window.setView(sf::View(sf::FloatRect(0, 0, CELL_SIZE * COLUMNS , FONT_HEIGHT + CELL_SIZE * ROWS )));

    //Alexander's sprite.
    sf::Sprite Alexander;

    //Alexander's texture.
    sf::Texture Alexander_texture;
    //We get the texture from an image
    Alexander_texture.loadFromFile("Resources/Images/Alexander.png");

    //We're putting Alexander in the bottom right corner
    Alexander.setPosition(static_cast<float>(CELL_SIZE * COLUMNS - Alexander_texture.getSize().y), CELL_SIZE * ROWS );
    Alexander.setTexture(Alexander_texture);

    
    Field field;

    //Get the current time and store it in a variable
    previous_time = std::chrono::steady_clock::now();
    std::string previous_timer_text = "";
    sf::Clock game_clock;
    unsigned short timer_x = static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 8 * FONT_WIDTH)));
    unsigned short timer_y = CELL_SIZE * ROWS - FONT_HEIGHT;
    int elapsed_time;
    std::string bestTime = "0:00:00";

   
    bool isNewBest = false;

    

    while (window.isOpen())
    {
        //Here we're calculating the lag
        unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();

        lag += delta_time;

        previous_time += std::chrono::microseconds(delta_time);
       
        //While the lag exceeds the maximum allowed frame duration
        while (FRAME_DURATION <= lag)
        {
            //We get the coordinates of the cell under the cursor
            unsigned char mouse_cell_x = std::clamp(static_cast<int>(floor(sf::Mouse::getPosition(window).x / static_cast<float>(CELL_SIZE * SCREEN_RESIZE))), 0, COLUMNS  - 1);
            unsigned char mouse_cell_y = std::clamp(static_cast<int>(floor(sf::Mouse::getPosition(window).y / static_cast<float>(CELL_SIZE * SCREEN_RESIZE))), 0, ROWS  - 1);

            //We use one of the basic arithmetic operations to perform this complex calculation
            lag -= FRAME_DURATION;

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape) 
                {
                    window.setView(window.getDefaultView());
                    isNewBest = false;
                    return;   
                }

                switch (event.type)
                {
                    //If the user clicked the close window button
                    case sf::Event::Closed:
                    {
                        window.close();
                        break;
                    }
                    case sf::Event::KeyReleased:
                    {
                        switch (event.key.code)
                        {
                            case sf::Keyboard::Enter:
                            {
                                //We restart the game if the Enter key was released
                                isNewBest = false;
                                field.restart();
                            }
                        }
                        break;
                    }
                    case sf::Event::MouseButtonReleased:
                    {
                        switch (event.mouseButton.button)
                        {
                            //If the LMB is released
                            case sf::Mouse::Left:
                            {
                                //We open the cell under the cursor
                                field.open_cell(mouse_cell_x, mouse_cell_y);

                                break;
                            }
                            //If the RMB is released
                            case sf::Mouse::Right:
                            {
                                //We flag the cell under the cursor
                                field.flag_cell(mouse_cell_x, mouse_cell_y);
                            }
                        }
                    }
                }
            }

            //Here we're deciding the state of Alexander's face
            //If any cell is currently pressed
            if (1 == sf::Mouse::isButtonPressed(sf::Mouse::Left) || 1 == sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                //Alexander opens his mouth
                Alexander_state = 1;

                //We also change the mouse state of the cell
                field.set_mouse_state(2, mouse_cell_x, mouse_cell_y);
            }
            else
            {
                //Alexander closes his mouth
                Alexander_state = 0;
                field.set_mouse_state(1, mouse_cell_x, mouse_cell_y);
            }
            //If the game is lost
            if (-1 == field.get_game_over())
            {
                //Alexander is disappointed 
                Alexander_state = 2;
            }
            //If the game is won
            else if (1 == field.get_game_over())
            {
                //Alexander is proud!
                Alexander_state = 3;
            }

            //Let's draw everything!
            if (FRAME_DURATION > lag)
            {
                //First we clear the window
                window.clear();

                //Then we draw the game field
                field.draw(window);

                //Here we show the text informing the player that he lost or won the game
                if (1 == field.effect_over())
                {
                    
                    if (1 == field.get_game_over())
                    {
                         //draw the victory text
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS  - 8 * FONT_WIDTH)) + 10), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS  - FONT_HEIGHT)) - 18), "VICTORY!\n", window, 15);
                        

                        // Display the time it took to complete the game
                        int hours = static_cast<int>(elapsed_time / (1000 * 60 * 60));
                        int minutes = static_cast<int>((elapsed_time / (1000 * 60)) % 60);
                        int seconds = static_cast<int>((elapsed_time / 1000) % 60);
                        std::string time_text = "TIME: " + std::to_string(hours) + ":" + 
                                                (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
                                                (seconds < 10 ? "0" : "") + std::to_string(seconds);
                        
                        game_clock.restart();
                       
                        
                        // Save the high score
                        std::string currentTime = difficulty + time_text.substr(4); 
                        int currentTimeSeconds = timeToSeconds(time_text.substr(time_text.find(":") + 2));

                        // Open the best time file
                        std::string easyBestTime = "", normalBestTime = "", hardBestTime = "";
                        std::ifstream bestTimeFile("Resources/besttime.txt");
                        if (bestTimeFile) {
                            std::string line;
                            int numLine = 1;
                            while (std::getline(bestTimeFile, line)) {
                                size_t colonPos = line.find(":");
                            if (colonPos != std::string::npos && colonPos + 2 < line.length()) {
                                if (numLine == 1) {
                                    easyBestTime = line.substr(colonPos + 2);
                                } else if (numLine == 2) {
                                    normalBestTime = line.substr(colonPos + 2);
                                } else if (numLine == 3) {
                                    hardBestTime = line.substr(colonPos + 2);
                                }
                            }
                                numLine++;
                            }
                            bestTimeFile.close();
                        } 
                        
                        // Compare the current time with the best time for the current difficulty level
                        if (difficulty == "EASY" && (easyBestTime.empty() || currentTimeSeconds < timeToSeconds(easyBestTime))) {
                            isNewBest = true;
                            saveBestTime("EASY", currentTime);
                        } else if (difficulty == "NORMAL" && (normalBestTime.empty() || currentTimeSeconds < timeToSeconds(normalBestTime))) {
                            isNewBest = true;
                            saveBestTime("NORMAL", currentTime);
                        } else if (difficulty == "HARD" && (hardBestTime.empty() || currentTimeSeconds < timeToSeconds(hardBestTime))) {
                            isNewBest = true;
                            saveBestTime("HARD", currentTime);
                        }

                        if(isNewBest) {
                            draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 8 * FONT_WIDTH)) - 20), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS - FONT_HEIGHT)) - 5), 
                                    "NEW BEST: " + time_text.substr(time_text.find(":") + 2) + "!\n", window, 13);
                        } else if (!isNewBest) {
                            draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 8 * FONT_WIDTH)) - 10), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS - FONT_HEIGHT)) - 5), 
                                    time_text + "\n", window, 15);
                        }
                                                
                        // Display the replay and return options
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 4 * FONT_WIDTH) - 33)), static_cast<unsigned short>(round(0.5f * (CELL_SIZE *  ROWS  - 2 * FONT_HEIGHT) + 18)), "Press Enter To Replay\n", window, 11);
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 4 * FONT_WIDTH) - 22)), static_cast<unsigned short>(round(0.5f * (CELL_SIZE *  ROWS  - 2 * FONT_HEIGHT) + 31)), "Or ESC To Return", window, 11);
                        
                    }
                    else 
                    {
                        game_clock.restart();
                        //same thing for losing
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS  - 4 * FONT_WIDTH)) - 17), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS - 2 * FONT_HEIGHT))), "GAME OVER!\n", window, 15);
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 4 * FONT_WIDTH) - 35)), static_cast<unsigned short>(round(0.5f * (CELL_SIZE *  ROWS  - 2 * FONT_HEIGHT) + 18)), "Press Enter To Replay", window, 11);
                        draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 4 * FONT_WIDTH) - 24)), static_cast<unsigned short>(round(0.5f * (CELL_SIZE *  ROWS  - 2 * FONT_HEIGHT) + 36)), "Or ESC To Return", window, 11);
                    }
                }


                //How many mines are left?
                draw_text(0, 0, CELL_SIZE * ROWS , "Mines:" + std::to_string(MINES - field.get_flags()), window, 12);

                if (!field.effect_over())
                {
                    elapsed_time = static_cast<int>(game_clock.getElapsedTime().asMilliseconds());
                    int hours = static_cast<int>(elapsed_time / (1000 * 60 * 60));
                    int minutes = static_cast<int>((elapsed_time / (1000 * 60)) % 60);
                    int seconds = static_cast<int>((elapsed_time / 1000) % 60);
                    std::string timer_text = std::to_string(hours) + ":" + 
                                            (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
                                            (seconds < 10 ? "0" : "") + std::to_string(seconds);
                    draw_text(0, timer_x + 25, timer_y + 16, timer_text, window, 12);
                }
            
                //We choose Alexander's face from a texture based on Alexander_state
                Alexander.setTextureRect(sf::IntRect(Alexander_state * Alexander_texture.getSize().y, 0, Alexander_texture.getSize().y, Alexander_texture.getSize().y));

                //We're drawing Alexander on the top of everything because he DESERVES it
                window.draw(Alexander);
                window.display();
            }
        }
    }
}