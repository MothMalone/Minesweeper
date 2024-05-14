#include <vector>

#include "Headers/Cell.hpp"
#include "Headers/GetCell.hpp"
#include "Headers/Global.hpp"

Cell::Cell(unsigned char i_x, unsigned char i_y) :
	mines_around(0),
	x(i_x),
	y(i_y)
{
	reset();
}

bool Cell::get_is_flagged()
{
	//Return whether the cell is flagged or not
	return is_flagged;
}

bool Cell::get_is_mine()
{
	//Return whether the cell has a mine or not
	return is_mine;
}

bool Cell::get_is_open()
{
	//Return whether the cell is open or not
	return is_open;
}

bool Cell::open(std::vector<Cell>& i_cells)
{
 
	if (0 == is_open)
	{
		is_open = 1;

		//If there are no mines in this cell and the cells around it, open them all.
		if (0 == is_mine && 0 == mines_around)
		{
			for (char a = -1; a < 2; a++)
			{
				for (char b = -1; b < 2; b++)
				{
					//Don't go outside the game field
					if ((0 == a && 0 == b) || (0 > a + x || 0 > b + y || COLUMNS == a + x || ROWS  == b + y))
					{
						continue;
					}

					//Open the neighboring cells 
					get_cell(a + x, b + y, i_cells)->open(i_cells);
				}
			}
		}

		//Start the effect if the cell has a mine
		effect_timer -= is_mine;

		//Return whether or not the cell has a mine
		return is_mine;
	}

	return 0;
}

bool Cell::update_effect_timer()
{
	//Is the effect timer over?
	if (0 < effect_timer)
	{
		//No? Then decrement it
		effect_timer--;

		if (0 == effect_timer)
		{
			//We'll only return 1 once, right after the timer ends.
			return 1;
		}
	}

	return 0;
}

unsigned char Cell::get_effect_timer()
{
	return effect_timer;
}

unsigned char Cell::get_mines_around()
{
	return mines_around;
}

unsigned char Cell::get_mouse_state()
{
	return mouse_state;
}

void Cell::count_mines_around(std::vector<Cell>& i_cells)
{
	//We start counting from 0
	mines_around = 0;

	//If this cell has a mine, then there's no point in counting mines around it
	if (0 == is_mine)
	{
		for (char a = -1; a < 2; a++)
		{
			for (char b = -1; b < 2; b++)
			{
				if ((0 == a && 0 == b) || (0 > a + x || 0 > b + y || COLUMNS == a + x || ROWS == b + y))
				{
					continue;
				}

				//If the neighbor has a mine...
				if (1 == get_cell(a + x, b + y, i_cells)->get_is_mine())
				{
					//...increment this variable
					mines_around++; 
					
				}
			}
		}
	}
}

void Cell::flag()
{
	if (0 == is_open)
	{
		is_flagged = 1 - is_flagged;
	}
}

void Cell::reset()
{
	//Set every variable to 0
	is_flagged = 0;
	is_mine = 0;
	is_open = 0;

	//Except the effect timer
	effect_timer = EFFECT_DURATION;
	mouse_state = 0;
}

//I'm pretty sure you're smart enough to understand what these functions do below
void Cell::set_effect_timer(unsigned char i_effect_timer)
{
	effect_timer = i_effect_timer;
}

void Cell::set_mine()
{
	is_mine = 1;
}

void Cell::set_mouse_state(unsigned char i_mouse_state)
{
	mouse_state = i_mouse_state;
}
