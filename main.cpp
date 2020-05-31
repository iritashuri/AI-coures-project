#include "GLUT.h"
#include <math.h>
#include <time.h>
#include "Node.h"
#include "Room.h"
#include <vector>
#include <iostream>
#include <queue>
#include "CompareNodes.h"
#include "Bullet.h"
#include "Granade.h"
#include "Character.h"

using namespace std;

const int W = 600; // window width
const int H = 600; // window height
const int SHOT_LENGTH = 30;

const int NUM_ROOMS = 20;

bool run_bfs = false;

Node maze[MSZ][MSZ];
double map[MSZ][MSZ] = { 0 };
Room rooms[NUM_ROOMS];
int numExistingRooms = 0;

Bullet* pb = NULL;
Granade* pg;
bool move_on = false;
int medRow1, medCol1, medRow2, medCol2, ammoRow1, ammoCol1, ammoRow2, ammoCol2;
vector <Character*> group1; // gray nodes
vector <Character*> group2; // gray nodes
bool play_game = false;
vector <Node> gray; // gray nodes
vector <Point2D*> exits;
Character* player1;

void SetupMaze();
void checkDeadPlayers();
void moveToSpcPoint(Character* ch, int row, int col);
void AddNeighboursChar(Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq);
double evaluateDistance(int source_row, int source_col, int dest_row, int dest_col);
bool shoot(Character* shooter, vector <Character*> enemies);
//void moveShot(Bullet* shot, Character* enemy);
void AddNeighboursStep(Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq);
void AddNodeStep(int row, int col, Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq);

void init()
{
	srand(time(0)); // pseudo randomizer

	glClearColor(0.7, 0.7, 0.7, 0);

	SetupMaze();

	glOrtho(-1, 1, -1, 1, -1, 1);

}

Room GenerateRoom()
{
	int w, h, ci, cj;
	Room* pr = nullptr;
	bool isOveralaping;

	do
	{
		delete pr;
		isOveralaping = false;
		w = 6 + rand() % 10;
		h = 6 + rand() % 10;

		ci = h / 2 + rand() % (MSZ - h);
		cj = w / 2 + rand() % (MSZ - w);

		pr = new Room(ci, cj, w, h);
		//		cout << "check new Room " << "center: (" << ci << "," << cj << "), width: " << w << ", height" << h << endl;
		for (int i = 0; i < numExistingRooms && !isOveralaping; i++)
		{
			//			cout << "room # " << i << " ";
			//			rooms[i].toString();
			if (rooms[i].CheckOverlapping(pr))
				isOveralaping = true;

		}
	} while (isOveralaping);

	// pr is not overlapping with other rooms
	for (int i = pr->getLeftTop().getRow(); i <= pr->getRightBottom().getRow(); i++)
		for (int j = pr->getLeftTop().getCol(); j <= pr->getRightBottom().getCol(); j++)
			maze[i][j].SetValue(ROOM_SPACE);
	return *pr;
}

// check if the node at row,col is white or gray that is better then the previous one
// and if so add it to pq
void AddNode(int row, int col, Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	Point2D pt;
	Node* pn1;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double cost;

	pt.setRow(row);
	pt.setCol(col);
	if (maze[row][col].GetValue() == SPACE || maze[row][col].GetValue() == ROOM_SPACE)
		cost = 0.1; // space cost
	else if (maze[row][col].GetValue() == WALL)
		cost = 3;
	// cost depends on is it a wall or a space
	pn1 = new Node(pt, pn->getTarget(), maze[pt.getRow()][pt.getCol()].GetValue(), pn->getG() + cost, pn);

	black_it = find(black.begin(), black.end(), *pn1);
	gray_it = find(gray.begin(), gray.end(), *pn1);
	if (black_it == black.end() && gray_it == gray.end()) // it is not black and not gray!
	{// i.e. it is white
		pq.push(pn1);
		gray.push_back(*pn1);
	}
}

void AddNeighbours(Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	// try down
	if (pn->getPoint().getRow() < MSZ - 1)
		AddNode(pn->getPoint().getRow() + 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try up
	if (pn->getPoint().getRow() > 0)
		AddNode(pn->getPoint().getRow() - 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try left
	if (pn->getPoint().getCol() > 0)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() - 1, pn, gray, black, pq);
	// try right
	if (pn->getPoint().getCol() < MSZ - 1)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() + 1, pn, gray, black, pq);
}

bool checkIfPointIsRoomExit(Point2D* point) {
	if (point->getRow() != MSZ && point->getCol() != MSZ) {
		if (maze[point->getRow() + 1][point->getCol()].GetValue() == ROOM_SPACE ||
			maze[point->getRow() - 1][point->getCol()].GetValue() == ROOM_SPACE ||
			maze[point->getRow()][point->getCol() + 1].GetValue() == ROOM_SPACE ||
			maze[point->getRow()][point->getCol() - 1].GetValue() == ROOM_SPACE)
			return true;
	}
	return false;
}

bool checkIfPointIsWALL(Point2D* point) {
	if (maze[point->getRow()][point->getCol()].GetValue() != AMMO &&
		maze[point->getRow()][point->getCol()].GetValue() != MEDICATION &&
		maze[point->getRow()][point->getCol()].GetValue() != ROOM_SPACE &&
		maze[point->getRow()][point->getCol()].GetValue() != GROUP_1 &&
		maze[point->getRow()][point->getCol()].GetValue() != GROUP_2) {
		return true;
	}
		
		return false;
}

// implement A* from start to target
void GeneratePath(Point2D start, Point2D target)
{
	priority_queue <Node*, vector<Node*>, CompareNodes> pq;
	vector<Node> gray;
	vector<Node> black;
	Node *pn;
	bool stop = false;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double wall_cost = 10;
	double space_cost = 0.2;
	pn = new Node(start, &target, maze[start.getRow()][start.getCol()].GetValue(), 0, nullptr);
	pq.push(pn);
	gray.push_back(*pn);
	while (!pq.empty() && !stop)
	{
		// take the best node from pq
		pn = pq.top();
		// remove top Node from pq
		pq.pop();
		if (pn->getPoint() == target) // the path has been found
		{
			stop = true;
			// restore path to dig tunnels
			// set SPACE instead of WALL on the path
			while (!(pn->getPoint() == start))
			{
				if (checkIfPointIsWALL(&(pn->getPoint()))){
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
					if (checkIfPointIsRoomExit(&(pn->getPoint()))) {
						//add to exits 
						exits.push_back(&(pn->getPoint()));
					}			
				}
				pn = pn->getParent();
			}
			return;
		}
		else // pn is not target
		{
			// remove Node from gray and add it to black
			gray_it = find(gray.begin(), gray.end(), *pn); // operator == must be implemented in Node
			if (gray_it != gray.end())
				gray.erase(gray_it);
			black.push_back(*pn);
			// check the neighbours
			AddNeighbours(pn, gray, black, pq);
		}
	}
}

void DigTunnels()
{
	int i, j;

	for (i = 0; i < NUM_ROOMS; i++)
	{
		cout << "Path from " << i << endl;
		for (j = i + 1; j < NUM_ROOMS; j++)
		{
			cout << " to " << j << endl;
			GeneratePath(rooms[i].getCenter(), rooms[j].getCenter());
		}
	}
}

void SetupMaze()
{
	int i, j, k;
	bool isRoom = false;
	Character* pc;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
			maze[i][j].SetValue(WALL);

	for (numExistingRooms = 0; numExistingRooms < NUM_ROOMS; numExistingRooms++)
		rooms[numExistingRooms] = GenerateRoom();

	int counterWalls = 0;
	while (counterWalls < 50) {
		i = rand() % MSZ;
		j = rand() % MSZ;
		if(maze[i][j].GetValue() != WALL)
			maze[i][j].SetValue(WALL);
		counterWalls++;
	}
	
	DigTunnels();
	//set ammo1
	while (!isRoom) {
		i = rand() % MSZ;
		j = rand() % MSZ;
		if (maze[i][j].GetValue() == ROOM_SPACE) {
			maze[i][j].SetValue(AMMO);
			ammoRow1 = i;
			ammoCol1 = j;
			//printf("ammo row = %d , ammo col = %d", ammoRow, ammoCol);
			isRoom = true;
		}
	}
	isRoom = false;
	//set ammo2
	while (!isRoom) {
		i = rand() % MSZ;
		j = rand() % MSZ;
		if (maze[i][j].GetValue() == ROOM_SPACE) {
			maze[i][j].SetValue(AMMO);
			ammoRow2 = i;
			ammoCol2 = j;
			//printf("ammo row = %d , ammo col = %d", ammoRow, ammoCol);

			isRoom = true;
		}
	}
	isRoom = false;
	//set medication1
	while (!isRoom) {
		i = rand() % MSZ;
		j = rand() % MSZ;
		if (maze[i][j].GetValue() == ROOM_SPACE) {
			maze[i][j].SetValue(MEDICATION);
			medRow1 = i;
			medCol1 = j;
			//printf("ammo row = %d , ammo col = %d", ammoRow, ammoCol);
			isRoom = true;
		}
	}
	isRoom = false;
	//set medication2
	while (!isRoom) {
		i = rand() % MSZ;
		j = rand() % MSZ;
		if (maze[i][j].GetValue() == ROOM_SPACE) {
			maze[i][j].SetValue(MEDICATION);
			medRow2 = i;
			medCol2 = j;
			//printf("ammo row = %d , ammo col = %d", ammoRow, ammoCol);

			isRoom = true;
		}
	}
	isRoom = false;
	//set group1
	int count = 0;
	while (count < 2) {
		while (!isRoom) {
			i = rand() % MSZ;
			j = rand() % MSZ;
			if (maze[i][j].GetValue() == ROOM_SPACE) {
				maze[i][j].SetValue(GROUP_1);
				int v = rand() % 4;     // v in the range 1 to 4
				//create character1-2
				pc = new Character(v, GROUP_1, i, j, ROOM_SPACE);
				pc->set_older_room_center(rooms);
				group1.push_back(pc);
				isRoom = true;
			}
		}
		isRoom = false;
		count++;
	}
	//set group2
	isRoom = false;
	count = 0;
	while (count < 2) {
		while (!isRoom) {
			i = rand() % MSZ;
			j = rand() % MSZ;
			if (maze[i][j].GetValue() == ROOM_SPACE) {
				maze[i][j].SetValue(GROUP_2);
				int v = rand() % 4;     // v in the range 1 to 4
				//create character2
				pc = new Character(v, GROUP_2, i, j, ROOM_SPACE);
				pc->set_older_room_center(rooms);
				group2.push_back(pc);
				isRoom = true;
			}
		}
		isRoom = false;
		count++;
	}
	
}

void DrawMaze()
{
	int i, j;
	double sz, x, y;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			// set color
			switch (maze[i][j].GetValue())
			{
			case SPACE:
				glColor3d(1, 1, 1); // white
				break;
			case ROOM_SPACE:
				glColor3d(1, 1, 1); // white
				break;
			case WALL:
				glColor3d(0.4, 0, 0); // dark red
				break;
			case START:
				glColor3d(0.4, 0.8, 1); // light blue
				break;
			case TARGET:
				glColor3d(1, 0, 0); // red
				break;
			case AMMO:
				glColor3d(0, 1, 0); // green
				break;
			case MEDICATION:
				glColor3d(0, 0, 1); // blue
				break;
			case GROUP_1:
				glColor3d(1, 0, 1); // puple
				break;
			case GROUP_2:
				glColor3d(1, 0.5, 0); // orange
				break;
			}


			// draw rectangle
			sz = 2.0 / MSZ;
			x = j * sz - 1;
			y = i * sz - 1;

			glBegin(GL_POLYGON);
			glVertex2d(x, y);
			glVertex2d(x + sz, y);
			glVertex2d(x + sz, y + sz);
			glVertex2d(x, y + sz);

			glEnd();
		}
}

void DrawMap()
{
	int i, j;
	double sz, xx, yy;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			if (maze[i][j].GetValue() == SPACE || maze[i][j].GetValue() == ROOM_SPACE)
			{
				double c;
				c = 1 - map[i][j];// 1(white) is very safe, 0(black) is very dangerous
				glColor3d(c, c, c);
				// draw rectangle
				sz = 2.0 / MSZ;
				xx = (j * sz - 1);
				yy = i * sz - 1;

				glBegin(GL_POLYGON);
				glVertex2d(xx, yy);
				glVertex2d(xx + sz, yy);
				glVertex2d(xx + sz, yy + sz);
				glVertex2d(xx, yy + sz);

				glEnd();
			}
		}
}

void GenerateMap()
{
	int num_tries = 1000;
	int i;
	int col, row;
	double x, y, sz;
	Granade* pg = nullptr;

	for (i = 0; i < num_tries; i++)
	{
		do
		{
			col = rand() % MSZ;
			row = rand() % MSZ;
		} while (maze[row][col].GetValue() != SPACE && maze[row][col].GetValue() != ROOM_SPACE);
		sz = 2.0 / MSZ;
		x = col * sz - 1;
		y = row * sz - 1;
		pg = new Granade(x, y);
		pg->SimulateExplosion(map, maze);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer

	DrawMaze();

	if (pg != NULL)
	{
		//	pb->showMe();
		pg->showMe();
		pg->buletShowMe(pb);
	}

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void displayMap()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer

	DrawMaze();
	DrawMap();

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

// checks if dx,dy is on SPACE in maze
bool CheckIsSpace(double dx, double dy)
{
	int i, j;

	i = MSZ * (dy + 1) / 2;
	j = MSZ * (dx + 1) / 2;
	return  (maze[i][j].GetValue() == SPACE || maze[i][j].GetValue() == ROOM_SPACE);
}

bool CheckIsROOMSpace(double dx, double dy)
{
	int i, j;

	i = MSZ * (dy + 1) / 2;
	j = MSZ * (dx + 1) / 2;
	return  (maze[i][j].GetValue() == ROOM_SPACE);
}

int calcDamage(double len)
{
	return (SHOT_LENGTH / len) * (rand() % 10);
}

bool ClearSight(int srcx, int srcy, int enemyTeam)
{
	int x, y;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			x = srcx;
			y = srcy;
			x += i;
			y += j;
			while (maze[x][y].GetValue() == SPACE || maze[x][y].GetValue() == ROOM_SPACE)
			{
				x += i;
				y += j;
			}
			if (maze[x][y].GetValue() == enemyTeam)
			{
				return true;
			}
		}
	}
	return false;
}

void NextStepFarAway(Point2D* start, Point2D* target) {
/*
	//printf("gost number %d\n", gostNumber);
	double smallestDis;
	double tempDis;
	double dist[4];


	Point2D* neigbors[4];
	Point2D* down = new Point2D (start->getRow() - 1,start->getCol());
	Point2D* up = new Point2D(start->getRow() + 1, start->getCol());
	Point2D* right = new Point2D(start->getRow() , start->getCol() + 1);
	Point2D* left = new Point2D(start->getRow() , start->getCol() - 1);

	neigbors[0] = down;
	neigbors[1] = up;
	neigbors[2] = left;
	neigbors[3] = right;
	Point2D* next = start;	
	*/
}

Point2D* nextStep(Point2D* start, Point2D* target)
{	
	
	//printf("Entered function\n");
	//printf("Start row = %d, col = %d\n", start->getRow(), start->getCol());
	//printf("Target row = %d, col = %d\n", target->getRow(), target->getCol());
	priority_queue <Node*, vector<Node*>, CompareNodes> pq;
	vector<Node> gray;
	vector<Node> black;
	Node *pn;
	bool stop = false;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	Point2D *next = nullptr;
	pn = new Node(*start, target, maze[start->getRow()][start->getCol()].GetValue(), 0, nullptr);
	pq.push(pn);
	gray.push_back(*pn);
	while (!pq.empty() && !stop)
	{
		// take the best node from pq
		pn = pq.top();
		// remove top Node from pq
		pq.pop();
		if (pn->getPoint() == *target) // the path has been found
		{
			stop = true;
			//restore path
			while (!(pn->getPoint() == *start))
			{
				if (pn->getParent()->getPoint() == *start)
				{
					next = &pn->getPoint();
				}
				pn = pn->getParent();
			}
		}
		else // pn is not target
		{
			// remove Node from gray and add it to black
			gray_it = find(gray.begin(), gray.end(), *pn); // operator == must be implemented in Node
			if (gray_it != gray.end())
				gray.erase(gray_it);
			black.push_back(*pn);
			// check the neighbours
			AddNeighboursStep(pn, gray, black, pq);
		}
	}
	return next;
}

void AddNeighboursStep(Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	// try down
	if (pn->getPoint().getRow() < MSZ - 1)
		AddNodeStep(pn->getPoint().getRow() + 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try up
	if (pn->getPoint().getRow() > 0)
		AddNodeStep(pn->getPoint().getRow() - 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try left
	if (pn->getPoint().getCol() > 0)
		AddNodeStep(pn->getPoint().getRow(), pn->getPoint().getCol() - 1, pn, gray, black, pq);
	// try right
	if (pn->getPoint().getCol() < MSZ - 1)
		AddNodeStep(pn->getPoint().getRow(), pn->getPoint().getCol() + 1, pn, gray, black, pq);
}

void AddNodeStep(int row, int col, Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	Point2D pt;
	Node* pn1;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double cost;


	pt.setRow(row);
	pt.setCol(col);
	if (maze[row][col].GetValue() != SPACE && maze[row][col].GetValue() != ROOM_SPACE && maze[row][col].GetValue() != maze[pn->getTarget()->getRow()][pn->getTarget()->getCol()].GetValue())
		return;

	cost = pn->getG() + map[row][col];
	pn1 = new Node(pt, pn->getTarget(), maze[pt.getRow()][pt.getCol()].GetValue(), cost, pn);

	black_it = find(black.begin(), black.end(), *pn1);
	gray_it = find(gray.begin(), gray.end(), *pn1);
	if (black_it == black.end() && gray_it == gray.end()) // it is not black and not gray!
	{// i.e. it is white
		pq.push(pn1);
		gray.push_back(*pn1);
	}
}

bool isWallToHide(int row, int col) {
	//Check that the wall is space arount it - it means its inside the room
	//check 2 nodes from the rhight
	if ((maze[row + 1][col].GetValue() != WALL && maze[row + 2][col].GetValue() != WALL) &&
		//check 2 nodes from the rhight
		(maze[row - 1][col].GetValue() != WALL && maze[row - 2][col].GetValue() != WALL) &&
		//check 2 nodes up
		(maze[row][col + 1].GetValue() != WALL && maze[row][col + 2].GetValue() != WALL) &&
		//check 2 nodes down
		(maze[row][col - 1].GetValue() != WALL && maze[row][col - 2].GetValue() != WALL))
		return true;
	return false;
}

Point2D* searchForCover(int myRow, int myCol) {
	Point2D* minPoint = new Point2D(myRow, myCol);
	int minRow = myRow;
	int minCol = myCol;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (maze[i][j].GetValue() != WALL || (maze[i][j].GetValue() == WALL && isWallToHide(i, j))) {
				if (map[i][j] < map[minRow][minCol]) {
					minPoint = new Point2D(i, j);
					minRow = i;
					minCol = j;
				}
			}
		}
	}
	return minPoint;
}

Point2D* getClosestExitInSameRoom(Point2D* myLocation) {
	double minDis = evaluateDistance(myLocation->getRow(),myLocation->getCol(), exits[0]->getRow(), exits[0]->getCol());
	Point2D* closesetExit = exits[0];
	double tempDis = 0;
	for each (Point2D* ex in exits)
	{
		tempDis = evaluateDistance(myLocation->getRow(), myLocation->getCol(), ex->getRow(), ex->getCol());
		if (tempDis < minDis) {
			minDis = tempDis;
			closesetExit = ex;
		}
	}
	return closesetExit;
}

Point2D* escape(Point2D* location)
{
	Point2D* BestCover = searchForCover(location->getRow(), location->getCol());
	Point2D* closestExit = getClosestExitInSameRoom(location);
	double best = evaluateDistance(location->getRow(), location->getCol(), BestCover->getRow(), BestCover->getCol());
	if (evaluateDistance(location->getRow(), location->getCol(), closestExit->getRow(), closestExit->getCol()) < best) {
		return closestExit;
	}
	return BestCover;
}

double evaluateDistance(int source_row,int source_col, int dest_row, int dest_col)
{
	return sqrt(pow(source_row - dest_row, 2) +
		pow(source_col - dest_col, 2));
}

void go_back_to_center(Character* chr) {
	bool finish = chr->isRoomCenter(rooms);
	Point2D* myPoint = new Point2D(chr->getRow(), chr->getCol());
	Point2D* next;
	while (!finish)
	{
		myPoint->setRow(chr->getRow());
		myPoint->setCol(chr->getCol());
		next = nextStep(myPoint, chr->get_older_room_center());
		if (next != nullptr) {
			if (chr->isRoomCenter(rooms)) {
				finish = true;
			}
			else {
				moveToSpcPoint(chr, next->getRow(), next->getCol());
				glutPostRedisplay();
			}
		}
		else {
			finish = true;
		}
	}
	

	
}

void moveGroup(vector <Character*> allies, vector <Character*> enemies)
{	
	if (allies.size() == 0)
		play_game = false;
	vector<Character*>::iterator chrDel;
	Point2D* next;
	int char_id = 0;
	for each (Character* chr in allies)
	{
		next = nullptr;
		printf("-------------------------------------------------------------------------\n");
		printf("I am character %d, from group %d\n", char_id, chr->getGroupId());
		Point2D* myPoint = new Point2D(chr->getRow(), chr->getCol());
		printf("my location is (%d,%d) ", chr->getRow(),chr->getCol());
		printf("my life status is %d, my ammo status is %d\n", chr->getLifeStatus(), chr->getAmmoStock());
		printf("my current action is: %s\n", chr->getStringAction().c_str());
		Point2D* target = nullptr;
		if (chr->getIsAlive()) {
			switch (chr->decideAction(maze,rooms,enemies))
			{
			case RUN_AWAY:
				next = escape(myPoint);
				break;
			case SEARCH_FOR_KILL:
				//look for closest enemy
				if (enemies.size() > 1) {
					if (evaluateDistance(chr->getRow(), chr->getCol(), enemies[0]->getRow(), enemies[0]->getCol()) < evaluateDistance(chr->getRow(), chr->getCol(), enemies[1]->getRow(), enemies[1]->getCol()))
					{
						target = new Point2D(enemies[0]->getRow(), enemies[0]->getCol());
						//Point2D* nextStep(Point2D* start, Point2D* target)
						next = nextStep(myPoint, target);
					}
					else
					{
						target = new Point2D(enemies[1]->getRow(), enemies[1]->getCol());
						//Point2D* nextStep(Point2D* start, Point2D* target)
						next = nextStep(myPoint, target);
					}
				}
				else if(enemies.size() == 1){
					target = new Point2D(enemies[0]->getRow(), enemies[0]->getCol());
					//Point2D* nextStep(Point2D* start, Point2D* target)
					next = nextStep(myPoint, target);
				}
				else {
					play_game = false;
				}
				break;
			case SHOOT:

				if (!shoot(chr, enemies))
				{
					if (enemies.size() == 2) {
						if (evaluateDistance(chr->getRow(), chr->getCol(), enemies[0]->getRow(), enemies[0]->getCol()) < evaluateDistance(chr->getRow(), chr->getCol(), enemies[1]->getRow(), enemies[1]->getCol()))
						{
							target = new Point2D(enemies[0]->getRow(), enemies[0]->getCol());
							//Point2D* nextStep(Point2D* start, Point2D* target)
							next = nextStep(myPoint, target);
						}
						else
						{
							target = new Point2D(enemies[1]->getRow(), enemies[1]->getCol());
							//Point2D* nextStep(Point2D* start, Point2D* target)
							next = nextStep(myPoint, target);
						}
					}
					else if(enemies.size() == 1){
						target = new Point2D(enemies[0]->getRow(), enemies[0]->getCol());
						//Point2D* nextStep(Point2D* start, Point2D* target)
						next = nextStep(myPoint, target);
					}
					else {
						play_game == false;
					}
					
				}
				break;
			case SEARCH_FOR_AMMO:
				if (evaluateDistance(chr->getRow(),chr->getCol(), ammoRow1, ammoCol1) < evaluateDistance(chr->getRow(),chr->getCol(), ammoRow2, ammoCol2))
				{
					target = new Point2D(ammoRow1, ammoCol1);
					next = nextStep(myPoint, target);
				}
				else
				{
					target = new Point2D(ammoRow2, ammoCol2);
					next = nextStep(myPoint, target);
				}
				break;
			case SEARCH_FOR_MED:
				if (evaluateDistance(chr->getRow(),chr->getCol(), medRow1,medCol1) < evaluateDistance(chr->getRow(),chr->getCol(),medRow2,medCol2))
				{
					target = new Point2D(medRow1, medCol1);
					next = nextStep(myPoint, target);
				}
				else
				{
					target = new Point2D(medRow2, medCol2);
					next = nextStep(myPoint, target);
				}
				break;
			default:
				break;
			}
			//if not shooting moving
			if (next != nullptr) {
				int enemyGoupId;
				if (chr->getGroupId() == GROUP_1)
					enemyGoupId = GROUP_2;
				else
					enemyGoupId = GROUP_1;
				int nextVal = maze[next->getRow()][next->getCol()].GetValue();
				int myVal = chr->GetOlderLocationValue();
				if (nextVal == AMMO) {
					chr->setAmmoStock(5);
					printf("I got ammo my ammo stock status is %d", chr->getAmmoStock());
					}
						
				else if (nextVal == MEDICATION) {
					chr->setLifeStatus(10);
					printf("I got meds my life status is %d", chr->getLifeStatus());
				}
				//If there is someone in the channel - nextVal == GROUP1/GROUP2
				else if (nextVal == enemyGoupId && myVal == SPACE) {
					int nextRow = next->getRow();
					int nextCol = next->getCol();
					Character* myEnemy = nullptr;
					for (int i = 0; i < enemies.size(); i++) {
						if ((enemies[i]->getRow() == nextRow) && (enemies[i]->getCol() == nextCol))
						{
							myEnemy = enemies[i];
							break;
						}
					}
					if (myEnemy) {
						printf("I am going to meet my enemy in a tunnel\n");
						//If its enemy there is 3 senarios: 
						// If  I have ammo -SHOOT.
						if (chr->getAmmoStock() > 0) {
							printf("One of us most die - I have ammo and I saw him first \n");
							//enemy dead
							myEnemy->setLifeStatus(-300);
						}
						else if (chr->getAmmoStock() < 0 && myEnemy->getAmmoStock() > 0) {
							printf("I have no ammo and he is shooting, I am going to die\n");
						}
						//If we both don't have ammo - go back to center of the room
						else {
							printf("I have no ammo but he is not shooting as well, I have time to run away\n");
							go_back_to_center(chr);
						}
					}
				}
				else if (nextVal == chr->getGroupId() && myVal == SPACE) {
					//If its someone from my group - go back to center of the room
					printf("I sow my friend, I will give him to continue before me\n");
					go_back_to_center(chr);

				}						
				else
				{
					moveToSpcPoint(chr, next->getRow(), next->getCol());
				}

				printf("After action--> my life status = %d, my ammo status = %d\n", chr->getLifeStatus(), chr->getAmmoStock());
				printf("-------------------------------------------------------------------------\n");
				char_id++;
			}	
			
		}
	}
}

void checkDeadPlayers() {
	printf("-------------------------------------------------------------------------\n");
	printf("Entered checkDeadPlayers function\n");
	if (group1.size() == 0 || group2.size() == 0) {
		printf("group %d size = %d , group %d size = %d\n", GROUP_1, group1.size(), GROUP_2, group2.size());
		play_game = false;
		printf("play game is false\n");
		return;
	}
	else {
		bool del_element_1 = false;
		bool del_element_2 = false;
		printf("Before iterating group1 -- > group1 size = %d\n", group1.size());
		for (int i = 0; i < group1.size(); i++) {
			printf("char %d - > (%d,%d) player type - %s \n", i, group1[i]->getRow(), group1[i]->getCol(), group1[i]->getCharType().c_str());
		}
		if (group1.size() == 1) {
			if (!group1[0]->getIsAlive()) {
				maze[group1[0]->getRow()][group1[0]->getCol()].SetValue(group1[0]->GetOlderLocationValue());
				group1.pop_back();
				play_game = false;
				return;
			}
		}
		else {
			for (int i = 0; i < group1.size(); i++) {
				if (!group1[i]->getIsAlive()) {
					if (i == 1) {
						maze[group1[i]->getRow()][group1[i]->getCol()].SetValue(group1[i]->GetOlderLocationValue());
						del_element_2 = true;
					}
					if (i == 0) {
						maze[group1[i]->getRow()][group1[i]->getCol()].SetValue(group1[i]->GetOlderLocationValue());
						del_element_1 = true;
					}
				}
			}
		}
		if (del_element_1 && del_element_2) {
			group1.clear();
			play_game = false;
			return;
		}
		else if (del_element_2 && !del_element_1) {
			group1.pop_back();
		}
		else if (del_element_1 && !del_element_2) {
			Character* temp = group1[1];
			group1.clear();
			// push back element1
			group1.push_back(temp);
		}

		printf("-------------------------------------------------------------------------\n");
		printf("Before iterating group2 -- > group2 size = %d\n", group2.size());
		for (int i = 0; i < group2.size(); i++) {
			printf("char %d - > (%d,%d) player type - %s \n", i, group2[i]->getRow(), group2[i]->getCol(), group2[i]->getCharType().c_str());
		}
		if (group2.size() == 1) {
			if (!group2[0]->getIsAlive()) {
				maze[group2[0]->getRow()][group2[0]->getCol()].SetValue(group2[0]->GetOlderLocationValue());
				group2.pop_back();
				play_game = false;
				return;
			}
		}
		else {
			del_element_1 = del_element_2 = false;
			for (int i = 0; i < group2.size(); i++) {
				if (!group2[i]->getIsAlive()) {
					if (i == 1) {
						maze[group2[i]->getRow()][group2[i]->getCol()].SetValue(group2[i]->GetOlderLocationValue());
						del_element_2 = true;
					}
					if (i == 0) {
						maze[group2[i]->getRow()][group2[i]->getCol()].SetValue(group2[i]->GetOlderLocationValue());
						del_element_1 = true;
					}
				}
			}
			if (del_element_1 && del_element_2) {
				group2.clear();
				play_game = false;
				return;
			}
			else if (del_element_2 && !del_element_1) {
				group2.pop_back();
			}
			else if (del_element_1 && !del_element_2) {
				Character* temp = group2[1];
				group2.clear();
				// push back element1
				group2.push_back(temp);
			}
			printf("-------------------------------------------------------------------------\n");
		}
	}
}

void movePlayers()
{	
	checkDeadPlayers();
	if (play_game == false) {
		printf("FALSE\n");
		if (group1.size() == 0) {
			printf("Group 2  win\n");
		}
		else {
			printf("Group 1 win\n");
		}
	}
	else
		moveGroup(group1, group2);
	checkDeadPlayers();
	if (play_game == false) {
		printf("FALSE\n");
		if (group1.size() == 0) {
			printf("Group 2  win\n");
		}
		else {
			printf("Group 1 win\n");
		}
	}
	else
		moveGroup(group2, group1);
}

void moveToSpcPoint(Character* ch, int row, int col) {
	maze[ch->getRow()][ch->getCol()].SetValue(ch->GetOlderLocationValue());
	if(ch->GetOlderLocationValue() == ROOM_SPACE)
		ch->set_older_room_center(rooms);
	ch->setOlderLocationValue(maze[row][col].GetValue());
	ch->moveOneStep(maze, row, col);
	maze[row][col].SetValue(ch->getGroupId());
}

bool shoot(Character* shooter, vector <Character*> enemies)
{
	Character* sightedEnemy = shooter->enemyInTheRoom(maze, rooms, enemies);
	double distance;
	if (sightedEnemy)
	{
		if (ClearSight(shooter->getRow(), shooter->getCol(), sightedEnemy->getGroupId()))
		{
			distance = evaluateDistance(shooter->getRow(), shooter->getCol(), sightedEnemy->getRow(), sightedEnemy->getCol());
			if (distance < SHOT_LENGTH)
			{

				sightedEnemy->setLifeStatus(-calcDamage(distance));
				shooter->setAmmoStock(-1);
				return true;
			}
		}
	}
	return false;
}

void idle()
{
	if (move_on && pg != NULL)
	{
		//		pb->SetIsMoving(CheckIsSpace(pb->getX(),pb->getY()));
		//		pb->move();
		pg->moveBullets(maze);
		pg->moveBullet(maze, pb);
		//		move_on = pg->GetIsMoving();
	}
	glutPostRedisplay();// calls indirectly to display
	if (play_game) {
		movePlayers();
		glutPostRedisplay();// calls indirectly to display

	}
}

void Menu(int choice)
{
	if (choice == 1) // generate security map
	{
		move_on = false;
		GenerateMap();
		glutDisplayFunc(displayMap);
	}
	else if (choice == 2) // generate security map
	{
		glutDisplayFunc(display);
		//		pb->SetIsMoving(true);
		pg->explode();
		move_on = true;
	}
	else if (choice == 3) // PlayGame
	{
		play_game = true;
	}
}

void mouse(int button, int state, int x, int y)
{
	double xx, yy;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		xx = 2 * (double)x / W - 1;
		yy = 2 * ((double)H - y) / H - 1;

		pb = new Bullet(xx, yy);
		pg = new Granade(xx, yy);
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		//		pb->SetIsMoving(true);
		//		pg->explode();

		//		move_on = true;
	}

}

// check if the node at row,col is white or gray that is better then the previous one
// and if so add it to pq
void AddNodeMap(int row, int col, Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	Point2D pt;
	Node* pn1;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double cost;

	pt.setRow(row);
	pt.setCol(col);
	if (maze[row][col].GetValue() == WALL)
		return; //do not add walls
	else if (maze[row][col].GetValue() == SPACE || maze[row][col].GetValue() == ROOM_SPACE)
		cost = map[row][col]; // map cost
	// cost depends on is it a wall or a space
	pn1 = new Node(pt, pn->getTarget(), maze[pt.getRow()][pt.getCol()].GetValue(), pn->getG() + cost, pn);

	black_it = find(black.begin(), black.end(), *pn1);
	gray_it = find(gray.begin(), gray.end(), *pn1);
	if (black_it == black.end() && gray_it == gray.end()) // it is not black and not gray!
	{// i.e. it is white
		pq.push(pn1);
		gray.push_back(*pn1);
	}
}

void AddNeighboursChar(Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	// try down
	if (pn->getPoint().getRow() < MSZ - 1)
		AddNodeMap(pn->getPoint().getRow() + 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try up
	if (pn->getPoint().getRow() > 0)
		AddNodeMap(pn->getPoint().getRow() - 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try left
	if (pn->getPoint().getCol() > 0)
		AddNodeMap(pn->getPoint().getRow(), pn->getPoint().getCol() - 1, pn, gray, black, pq);
	// try right
	if (pn->getPoint().getCol() < MSZ - 1)
		AddNodeMap(pn->getPoint().getRow(), pn->getPoint().getCol() + 1, pn, gray, black, pq);
}

void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Dungeon ");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens
	glutMouseFunc(mouse);
	// menu
	glutCreateMenu(Menu);
	glutAddMenuEntry("Generate map", 1);
	glutAddMenuEntry("Explode", 2);
	glutAddMenuEntry("Play", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);



	init();

	glutMainLoop();
}