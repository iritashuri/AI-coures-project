#pragma once
#include "Point2D.h"

const int SPACE = 0;
const int WALL = 1;
const int START = 2;
const int TARGET = 3;
const int PATH = 4; // belongs to the path to target
const int GRAY = 5; // Fringe
const int BLACK = 6; // VISITED
const int AMMO = 7;
const int MEDICATION = 8;
const int ROOM_SPACE = 9;

class Node
{
public:
	Node();
	~Node();

	Node(Point2D & pt, Point2D * t, int v, double g, Node * pr);

private:
	int value;
	double h, g;
	Node* parent;
	Point2D* target;
	Point2D point;


public:
	void SetValue(int value);
	int GetValue();
	double getG();
	double ComputeH();
	double getF();
	Point2D getPoint();
	Node* getParent();
	Point2D* getTarget();
	bool operator == (const Node &other) {
		return point == other.point;
	}
};

