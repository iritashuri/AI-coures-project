#include "Node.h"
#include "Point2D.h"
#include "Node.h"
#include "Room.h"
#include <vector>
#include <iostream>
#include <queue>

using namespace std;


const int MSZ2 = 100;
const int RUNAWAYER = 0;
const int KILLER = 1;
const int MEDICATION_LOVER = 2;
const int AMMO_LOVER = 3;

const int SHOOT = 0;
const int SEARCH_FOR_KILL = 1;
const int SEARCH_FOR_MED = 2;
const int SEARCH_FOR_AMMO = 3;
const int RUN_AWAY = 4;
const int noAction = 5;


class Character
{
public:
	Character();
	~Character();

	Character(int v, int id, int row, int col,int olderLocationValue);

private:
	int value;
	int olderLocationValue;
	int groupId;
	int currentAction;
	int ammoStock;
	int lifeStatus;
	int locationRow;
	int locationCol;
	bool isAlive;
	bool deleted;
	Point2D* older_room_canter;

public:
	int GetValue();
	void setOlderLocationValue(int value);
	int GetOlderLocationValue();
	int getGroupId();
	void setAmmoStock(int amount);
	int getCurrenAction();
	string getStringAction();
	string getCharType();
	int getAmmoStock();
	void setLifeStatus(int amount);
	int getLifeStatus();
	void setLocation(int row, int col);
	int getRow();
	int getCol();
	void moveOneStep(Node maze[100][100], int newRow, int newCol);
	int decideAction(Node maze[100][100], Room rooms[20], vector <Character*> enemy);
	Character* enemyInTheRoom(Node maze[100][100], Room rooms[20], vector <Character*> enemy);
	bool isInRoom(Room room, int  row, int col);
	bool isRoomCenter(Room rooms[20]);
	bool getIsAlive();
	bool getDeleted();
	void setDeleted();
	bool operator == (const Character &other) {
		return locationRow == other.locationRow && locationCol == other.locationCol;
	}
	bool isEnemyClose(Node maze[100][100]);
	Point2D* get_older_room_center();
	void set_older_room_center(Room rooms[20]);
};