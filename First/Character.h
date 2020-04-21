#include "Node.h"
#include "Point2D.h"
#include "Node.h"

const int MSZ2 = 100;
const int RUNAWAYER = 0;
const int KILLER = 1;
const int MEDICATION_LOVER = 2;
const int AMMO_LOVER = 3;


class Character
{
public:
	Character();
	~Character();

	Character(int v, int id, int row, int col);

private:
	int value;
	int groupId;
	int ammoStock;
	int medStock;
	int lifeStatus;
	int locationRow;
	int locationCol;

public:
	int GetValue();
	int getGroupId();
	void setAmmoStock(int amount);
	int getAmmoStock();
	void setLifeStatus(int amount);
	int getLifeStatus();
	void setLocation(int row, int col);
	int getRow();
	int getCol();
	void moveOneStep(Node maze[100][100], int newRow, int newCol);
};
