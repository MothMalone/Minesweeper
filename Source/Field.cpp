#include <random>
#include <SFML/Graphics.hpp>

#include "Headers/Cell.hpp"
#include "Headers/Field.hpp"
#include "Headers/GetCell.hpp"
#include "Headers/Global.hpp"


Field::Field() :
	game_over(1),
	random_engine(random_device())
{
	for (unsigned char a = 0; a < ROWS; a++)
	{
		for (unsigned char b = 0; b < COLUMNS; b++)
		{
			//swapped the coordinates to fix the bug regarding the coordinates
			cells.push_back(Cell(b, a));
		}
	}

	restart();
}

bool Field::effect_over()
{
	//Check every cell's effect timer
	for (Cell& cell : cells)
	{
		//If any cell's effect timer is not over yet
		if (0 < cell.get_effect_timer())
		{
			//Return 0
			return 0;
		}
	}

	
	return 1;
}

char Field::get_game_over()
{
	return game_over;
}

unsigned short Field::get_flags()
{
	//We just count the total number of flagged cells
	unsigned short total_flags = 0;

	for (Cell& cell : cells)
	{
		total_flags += cell.get_is_flagged();
	}

	//And we return the result
	return total_flags;
}

void Field::draw(sf::RenderWindow& i_window)
{
	//We'll use this to draw cells
	sf::RectangleShape cell_shape(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

	sf::Sprite icon_sprite;

	sf::Texture icons_texture;
	icons_texture.loadFromFile("Resources/Images/Icons" + std::to_string(CELL_SIZE) + ".png");

	icon_sprite.setTexture(icons_texture);

	//Loop through each cell
	for (unsigned char a = 0; a < COLUMNS; a++)
	{
		for (unsigned char b = 0; b < ROWS; b++)
		{
			//Change the position of the cell shape
			cell_shape.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));

			//If the current cell is open
			if (1 == get_cell(a, b, cells)->get_is_open())
			{
				//Get the number of mines surrounding it
				unsigned char mines_around = get_cell(a, b, cells)->get_mines_around();

				cell_shape.setFillColor(sf::Color(146, 182, 255));

				//Draw the cell 
				i_window.draw(cell_shape);

				//If the cell has at least one mine around it
				if (0 < mines_around)
				{
					//Take the number from the icons' texture and draw it
					icon_sprite.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
					icon_sprite.setTextureRect(sf::IntRect(CELL_SIZE * mines_around, 0, CELL_SIZE, CELL_SIZE));

					i_window.draw(icon_sprite);
				}
			}
			else //If the cell is closed
			{
				cell_shape.setFillColor(sf::Color(0, 73, 255));

				//Set the cell's color based on the mouse state
				//Don't change its color if the game is over
				if (0 == game_over)
				{
					if (1 == get_cell(a, b, cells)->get_mouse_state())
					{
						cell_shape.setFillColor(sf::Color(36, 109, 255));
					}
					else if (2 == get_cell(a, b, cells)->get_mouse_state())
					{
						cell_shape.setFillColor(sf::Color(0, 36, 255));
					}
				}

				i_window.draw(cell_shape);

				//If the cell is flagged
				if (1 == get_cell(a, b, cells)->get_is_flagged())
				{
					//Take the flag image and draw it
					icon_sprite.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
					icon_sprite.setTextureRect(sf::IntRect(0, 0, CELL_SIZE, CELL_SIZE));

					i_window.draw(icon_sprite);
				}
			}

			//Reset the cell's mouse state
			get_cell(a, b, cells)->set_mouse_state(0);

			//This is where we draw the effect
			
			//Don't draw the effect if the game is not over or the cell effect timer hasn't yet started.
			if (0 != game_over && EFFECT_DURATION > get_cell(a, b, cells)->get_effect_timer())
			{
				//Calculate the size of the effect
				unsigned char effect_size = static_cast<unsigned char>(2 * round(0.5f * CELL_SIZE * ((EFFECT_DURATION - get_cell(a, b, cells)->get_effect_timer()) / static_cast<float>(EFFECT_DURATION))));

				//The effect timer of each cell will have a random duration
				std::uniform_int_distribution<unsigned short> effect_duration_distribution(1, EFFECT_DURATION - 1);

				//Use the cell shape to draw effects, because I love recycling!
				cell_shape.setPosition(floor(CELL_SIZE * (0.5f + a) - 0.5f * effect_size), floor(CELL_SIZE * (0.5f + b) - 0.5f * effect_size));
				cell_shape.setSize(sf::Vector2f(effect_size, effect_size));

				//The color of the effect will depend on whether the game is lost or won
				if (-1 == game_over)
				{
					cell_shape.setFillColor(sf::Color(255, 36, 0));
				}
				else
				{
					cell_shape.setFillColor(sf::Color(255, 255, 255));
				}

				//Draw the effect
				i_window.draw(cell_shape);

				//Reset the cell shape's size
				cell_shape.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

				//If the effect timer is over
				if (1 == get_cell(a, b, cells)->update_effect_timer())
				{
					//We start each neighboring cell's effect timer
					if (0 <= a - 1 && EFFECT_DURATION == get_cell(a - 1, b, cells)->get_effect_timer())
					{
						get_cell(a - 1, b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (0 <= b - 1 && EFFECT_DURATION == get_cell(a, b - 1, cells)->get_effect_timer())
					{
						get_cell(a, b - 1, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (COLUMNS  > 1 + a && EFFECT_DURATION == get_cell(1 + a, b, cells)->get_effect_timer())
					{
						get_cell(1 + a, b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (ROWS > 1 + b && EFFECT_DURATION == get_cell(a, 1 + b, cells)->get_effect_timer())
					{
						get_cell(a, 1 + b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}
				}
			}
		}
	}
}

void Field::flag_cell(unsigned char i_x, unsigned char i_y)
{
	//We don't let the player to flag cells when the game is over
	if (0 == game_over)
	{
		// Get the current cell
        // Get the current cell
        Cell* cell = get_cell(i_x, i_y, cells);

        // If the cell is flagged, unflag it regardless of the mines left count
        if (cell->get_is_flagged() == 1)
        {
            cell->flag();
        }
        // If the cell is not flagged, check if there are mines left to flag
        else if (MINES - get_flags() > 0)
        {
            cell->flag();
        }
	}
}

void Field::open_cell(unsigned char i_x, unsigned char i_y)
{
	//If this is the first cell we're opening
	if (0 == first_click)
	{
		//Declare coordinate distributions
		std::uniform_int_distribution<unsigned short> x_distribution(0, COLUMNS  - 1);
		std::uniform_int_distribution<unsigned short> y_distribution(0, ROWS  - 1);

		first_click = 1;

		//Then generate mines
		for (unsigned short a = 0; a < MINES; a++)
		{
			unsigned char mine_x = static_cast<unsigned char>(x_distribution(random_engine));
			unsigned char mine_y = static_cast<unsigned char>(y_distribution(random_engine));

			//If the chosed cell already has a mine in it or it's a cell that the player wants to open
			if (1 == get_cell(mine_x, mine_y, cells)->get_is_mine() || (i_x == mine_x && i_y == mine_y))
			{
				//Try again
				a--;
			}
			else
			{
				//Otherwise, set the mine
				get_cell(mine_x, mine_y, cells)->set_mine();
			}
		}

		//After we generate mines, each cell counts how many mines are surrounding it
		for (Cell& cell : cells)
		{
			cell.count_mines_around(cells);
		}
	}

	//We don't open the cell when the game is over or when the cell is flagged
	if (0 == game_over && 0 == get_cell(i_x, i_y, cells)->get_is_flagged())
	{
		if (1 == get_cell(i_x, i_y, cells)->open(cells))
		{
			//When the player opens a cell with a mine, we set the game over to -1
			game_over = -1;
		}
		else
		{
			unsigned short total_closed_cells = 0;

			//We count how many cells are closed
			for (Cell& cell : cells)
			{
				total_closed_cells += 1 - cell.get_is_open();
			}

			//If the number of closed cells equals the total number of mines, we'll consider that the game is won
			if (MINES == total_closed_cells)
			{
				//We set the game over to 1
				game_over = 1;

				//Then we start the effect
				get_cell(i_x, i_y, cells)->set_effect_timer(EFFECT_DURATION - 1);
			}

			else if (get_cell(i_x, i_y, cells)->get_is_open() && get_cell(i_x, i_y, cells)->get_mines_around() > 0 && 
                 are_all_mines_correctly_flagged(i_x, i_y))
			{
				open_safe_cells_around(i_x, i_y);
				total_closed_cells = 0;
				for (Cell& cell : cells)
				{
					total_closed_cells += 1 - cell.get_is_open();
				}
				
				// Check if the game is won after opening the safe cells
				if (MINES == total_closed_cells)
				{
					game_over = 1;
					get_cell(i_x, i_y, cells)->set_effect_timer(EFFECT_DURATION - 1);
				}
			}
           
		}
	}
}

void Field::restart()
{
	//We only restart the game when it's over
	if (0 != game_over)
	{
		first_click = 0;

		game_over = 0;

		for (Cell& cell : cells)
		{
			cell.reset();
		}
	}
}

//Since we can't call the cell's function directly, we must use this function
void Field::set_mouse_state(unsigned char i_mouse_state, unsigned char i_x, unsigned char i_y)
{
	
	get_cell(i_x, i_y, cells)->set_mouse_state(i_mouse_state);
}



bool Field::are_all_mines_correctly_flagged(unsigned char i_x, unsigned char i_y)
{
    int mines_around = get_cell(i_x, i_y, cells)->get_mines_around();
    int correctly_flagged_cells = 0;
    int flags_around = 0;

    for (char a = -1; a < 2; a++)
    {
        for (char b = -1; b < 2; b++)
        {
            // Don't go outside the game field
            if ((0 == a && 0 == b) || (0 > a + i_x || 0 > b + i_y || COLUMNS == a + i_x || ROWS == b + i_y))
            {
                continue;
            }
            if (get_cell(a + i_x, b + i_y, cells)->get_is_flagged())
            {
                flags_around++;
                if (get_cell(a + i_x, b + i_y, cells)->get_is_mine())
                {
                    correctly_flagged_cells++;
                }
            }
        }
    }

    // Return true only if the number of correctly flagged mines is equal to the number of mines around the cell
    // and the number of flags around the cell is less than or equal to the number of mines around the cell
    return correctly_flagged_cells == mines_around && flags_around == mines_around;
}



void Field::open_safe_cells_around(unsigned char i_x, unsigned char i_y)
{
    for (char a = -1; a < 2; a++)
    {
        for (char b = -1; b < 2; b++)
        {
            // Don't go outside the game field
            if ((0 == a && 0 == b) || (0 > a + i_x || 0 > b + i_y || COLUMNS == a + i_x || ROWS == b + i_y))
            {
                continue;
            }
            // Open the cell if it's not flagged
            if (!get_cell(a + i_x, b + i_y, cells)->get_is_flagged())
            {
                get_cell(a + i_x, b + i_y, cells)->open(cells);
            }
        }
    }
}