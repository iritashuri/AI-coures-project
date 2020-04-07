#include "Character.h"
#include "Point2D.h"
#include "Node.h"

Character::Character()
{
}

Character::~Character()
{
}

Character::Character(int v, int id, Point2D loc)
{
	value = v;
	groupId = id;
	ammoStock = 20;
	lifeStatus = 100;
	location = loc;
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

void Character::setLocation(Point2D loc)
{
	location = loc;
}

Point2D Character::getLocation()
{
	return location;
}

