#include "Character.h"


Character::Character()
{
}

Character::~Character()
{
}

Character::Character(int v, int id, int row, int col)
{
	value = v;
	groupId = id;
	ammoStock = 20;
	lifeStatus = 100;
	locationRow = row;
	locationCol = col;
}


int Character::GetValue()
{
	return value;
}

int Character::getGroupId()
{
	return groupId;
}

void Character::setAmmoStock(int amount)
{
	ammoStock += amount;
}

int Character::getAmmoStock()
{
	return ammoStock;
}

void Character::setLifeStatus(int amount)
{
	lifeStatus += amount;
}

int Character::getLifeStatus()
{
	return lifeStatus;
}

void Character::setLocation(int row, int col)
{
	locationRow = row;
	locationCol = col;
}

int Character::getRow()
{
	return locationRow;
}

int Character::getCol()
{
	return locationCol;
}

void Character::moveOneStep(Node maze[100][100], int newRow, int newCol)
{
	
	if (maze[newRow][newCol].GetValue() == SPACE || maze[newRow][newCol].GetValue() == ROOM_SPACE ||
		maze[newRow][newCol].GetValue() == AMMO || maze[newRow][newCol].GetValue() == MEDICATION)
	{
		setLocation(newRow, newCol);
	
	}
}
