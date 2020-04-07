#include "Character.h"

Character::Character()
{
}

Character::~Character()
{
}

Character::Character(int v, int id)
{
	value = v;
	groupId = id;
	ammoStock = 20;
	medStock = 20;
	lifeStatus = 10;
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

void Character::setMedStock(int amount)
{
	medStock += amount;
}

int Character::getMedStock()
{
	return medStock;
}

void Character::setLifeStatus(int amount)
{
	lifeStatus += amount;
}

int Character::getLifeStatus()
{
	return lifeStatus;
}

