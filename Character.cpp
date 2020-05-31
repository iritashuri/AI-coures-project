#include "Character.h"


Character::Character()
{
}

Character::~Character()
{
}

Character::Character(int v, int id, int row, int col,int  olderLocationV)
{
	value = v;
	olderLocationValue = olderLocationV;
	groupId = id;
	ammoStock = 20;
	lifeStatus = 100;
	locationRow = row;
	locationCol = col;
	currentAction = SEARCH_FOR_KILL;
	isAlive = true;
	deleted = false;
	older_room_canter = nullptr;
}


int Character::GetValue()
{
	return value;
}

void Character::setOlderLocationValue(int value)
{
	this->olderLocationValue = value;
}

int Character::GetOlderLocationValue()
{
	return olderLocationValue;
}

int Character::getGroupId()
{
	return groupId;
}

void Character::setAmmoStock(int amount)
{
	this->ammoStock = ammoStock + amount;
	if (this->ammoStock > 20)
	{
		this->ammoStock = 20;
	}
}

int Character::getAmmoStock()
{
	return ammoStock;
}

void Character::setLifeStatus(int amount)
{
	this->lifeStatus = lifeStatus +  amount;
	if (this->lifeStatus > 100)
	{
		this->lifeStatus = 100;
	}
}

int Character::getLifeStatus()
{
	return lifeStatus;
}

void Character::setLocation(int row, int col)
{
	this->locationRow = row;
	this->locationCol = col;
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
		//printf("I seted older location value with %d", maze[locationRow][locationCol].GetValue());
		setLocation(newRow, newCol);
	}
}

int Character::decideAction(Node maze[100][100], Room rooms[20], vector <Character*> enemy)
{
	Character* check_enemy_in_the_room = enemyInTheRoom(maze, rooms, enemy);
	bool is_any_enemy_close = isEnemyClose(maze);
	printf("Group id = %d, player val = %d\n", this->groupId, this->value);
	switch (value) {
	case RUNAWAYER:
		if (lifeStatus < 50 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//run away
			this->currentAction = RUN_AWAY;
			return RUN_AWAY;
		}
		else if (lifeStatus > 50 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//shoot
			this->currentAction = SHOOT;
			return SHOOT;
		}
		else if (lifeStatus < 10) {
			//search for med
			this->currentAction = SEARCH_FOR_MED;
			return SEARCH_FOR_MED;
		}
		else if (ammoStock < 3) {
			//search for ammo
			this->currentAction = SEARCH_FOR_AMMO;
			return SEARCH_FOR_AMMO;
		}
		else {
			//search for kill
			this->currentAction = SEARCH_FOR_KILL;
			return SEARCH_FOR_KILL;
		}
		break;
	case KILLER:
		if (ammoStock >= 1 && (check_enemy_in_the_room || is_any_enemy_close) && lifeStatus > 10) {
		//shoot
			this->currentAction = SHOOT;
			return SHOOT;
		}
		else if (ammoStock < 1 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//run away
			this->currentAction = RUN_AWAY;
			return RUN_AWAY;
		}
		else if (ammoStock > 2 && lifeStatus < 15) {
			//search for med
			this->currentAction = SEARCH_FOR_MED;
			return SEARCH_FOR_MED;
		}
		else if (ammoStock < 2) {
			//search  for ammo
			this->currentAction = SEARCH_FOR_AMMO;
			return SEARCH_FOR_AMMO;
		}
		else {
			//seach for kill
			this->currentAction = SEARCH_FOR_KILL;
			return SEARCH_FOR_KILL;
		}
		break;
	case MEDICATION_LOVER:
		if (lifeStatus < 50 && !(check_enemy_in_the_room || is_any_enemy_close)) {
		//search  for meds
			this->currentAction = SEARCH_FOR_MED;
			return SEARCH_FOR_MED;
		}
		else if (ammoStock >= 1 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//shoot
			this->currentAction = SHOOT;
			return SHOOT;
		}
		else if (lifeStatus < 20 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//runaway
			this->currentAction = RUN_AWAY;
			return RUN_AWAY;


		}
		else if (ammoStock < 2 ) {
			//search for ammo
			this->currentAction = SEARCH_FOR_AMMO;
			return SEARCH_FOR_AMMO;
		}
		else {
			//search for kill
			this->currentAction = SEARCH_FOR_KILL;
			return SEARCH_FOR_KILL;
		}
		break;
	case AMMO_LOVER:
		if (ammoStock < 5) {
			//search for ammo
			this->currentAction = SEARCH_FOR_AMMO;
			return SEARCH_FOR_AMMO;
		}
		else if (lifeStatus > 15 && (check_enemy_in_the_room || is_any_enemy_close) && ammoStock>= 2) {
		//shoot
			this->currentAction = SHOOT;
			return SHOOT;
		}
		else if (lifeStatus < 15 && (check_enemy_in_the_room || is_any_enemy_close)) {
		//run away
			this->currentAction = RUN_AWAY;
			return RUN_AWAY;
		}
		else if (lifeStatus < 10) {
			//search for meds
			this->currentAction = SEARCH_FOR_MED;
			return SEARCH_FOR_MED;
		}
		else {
			//search for kill
			this->currentAction = SEARCH_FOR_KILL;
			return SEARCH_FOR_KILL;
		}
		break;
	}
	//return SEARCH_FOR_KILL;
}




Character* Character::enemyInTheRoom(Node maze[100][100], Room rooms[20], vector <Character*> enemy)
{
	//Check if I am in a room
	if (this->GetOlderLocationValue() == ROOM_SPACE) {
		//chaeck if enemy is here
		Room myRoom;
		//getMyRoom
		for (int i = 0; i < 20; i++) {
			if (isInRoom(rooms[i], locationRow, locationCol)){
				myRoom = rooms[i];
				break;
			}
		}
		for each(Character* current in enemy) {
			if (isInRoom(myRoom, current->getRow(), current->getCol())) {
				return current;
			}
		}
	}
	return nullptr;
}

//Check if specific point is in specific room
bool Character::isInRoom(Room room, int row, int col)
{
	if (row >= room.getLeftTop().getRow() &&
		row <= room.getRightBottom().getRow() &&
		col <= room.getRightBottom().getCol() &&
		col >= room.getLeftTop().getCol()) {
		return true;
	}
	return false;

}

bool Character::isRoomCenter(Room rooms[20])
{
	if (locationRow == older_room_canter->getRow() && locationCol == older_room_canter->getCol()) {
		return true;
	}
	return false;
}

bool Character::getIsAlive()
{
	return this->lifeStatus>0;
}


bool Character::getDeleted()
{
	return deleted;
}

void Character::setDeleted()
{
	this->deleted = true;
}

bool Character::isEnemyClose(Node  maze[100][100])
{
	int enemy_id;
	if (this->groupId == GROUP_1)
		enemy_id = GROUP_2;
	else
		enemy_id = GROUP_1;
	// Check there is no other charater in range of 5 nodes
	for (int i = locationRow; i < locationRow + 4; i++) {
		for (int j = locationCol; j < locationCol + 4; j++) {
			if (i < MSZ2 - 1 && j < MSZ2 - 1)
			{
				if (maze[i][j].GetValue() == enemy_id)
					return true;
			}
		}
	}
	return false;
}

Point2D * Character::get_older_room_center()
{
	return this->older_room_canter;
}

void Character::set_older_room_center(Room rooms[20])
{
	//Check if I am in a room
	if (this->GetOlderLocationValue() == ROOM_SPACE) {
		//getMyRoom
		for (int i = 0; i < 20; i++) {
			if (isInRoom(rooms[i], locationRow, locationCol)) {
				this->older_room_canter = new Point2D(rooms[i].getCenter().getRow(), rooms[i].getCenter().getCol());
				break;
			}
		}
	}
	
}

int Character::getCurrenAction()
{
	return currentAction;
}

string Character::getStringAction()
{
	string type = "";
	switch (currentAction)
	{
	case SHOOT:
		type = "SHOOT";
		break;
	case SEARCH_FOR_KILL:
		type = "SEARCH_FOR_KILL";
		break;
	case SEARCH_FOR_MED:
		type = "SEARCH_FOR_MED";
		break;
	case SEARCH_FOR_AMMO:
		type = "SEARCH_FOR_AMMO";
		break;
	case RUN_AWAY:
		type = "RUN_AWAY";
		break;
	case noAction:
		type = "WAIT";
		break;
	default:
		break;
	}
	return type;
}

string Character::getCharType()
{
	string type = "";
	switch (value)
	{
	case RUNAWAYER:
		type = "SCARED";
		break;
	case KILLER:
		type = "KILLER";
		break;
	case MEDICATION_LOVER:
		type = "MEDICATION_LOVER";
		break;
	case AMMO_LOVER:
		type = "AMMO_LOVER";
		break;
	default:
		this->value = KILLER;
		type = "KILLER";
		break;
	}
	return type;
}