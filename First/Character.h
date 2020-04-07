const int RUNAWAYER = 0;
const int KILLER = 1;
const int MEDICATION_LOVER = 2;
const int AMMO_LOVER = 3;


class Character
{
public:
	Character();
	~Character();

	Character(int v, int id);

private:
	int value;
	int groupId;
	int ammoStock;
	int medStock;
	int lifeStatus;

public:
	int GetValue();
	int getGroupId();
	void setAmmoStock();
	int getAmmoStock();
	void setMedStock();
	int getMedStock();
	void setLifeStatus();
	int getLifeStatus();

};
