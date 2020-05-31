#pragma once
#include "Point2D.h"

class Room
{
public:
	Room();
	~Room();
	Room(int ci, int cj, int w, int h);
	Point2D getLeftTop();
	Point2D getRightBottom();
	bool CheckOverlapping(Room * pother);
	void toString();
	Point2D getCenter();
	int getWidth();
	int getHeight();
private:
	Point2D leftTop, rightBottom, center;
	int width, height;
};

