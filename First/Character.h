#include "Point2D.h"
#include "Node.h"

const int RUNAWAYER = 0;
const int KILLER = 1;
const int MEDICATION_LOVER = 2;
const int AMMO_LOVER = 3;


class Character
{
public:
	Character();
	~Character();

	Character(int v, int id, Point2D loc);

private:
	int value;
	int groupId;
	int ammoStock;
	int medStock;
	int lifeStatus;
	Point2D location;

public:
	int GetValue();
	int getGroupId();
	void setAmmoStock(int amount);
	int getAmmoStock();
	void setLifeStatus(int amount);
	int getLifeStatus();
	void setLocation(Point2D loc);
	Point2D getLocation();
};
