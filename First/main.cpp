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
vector <Character> group1; // gray nodes
vector <Character> group2; // gray nodes

vector <Node> gray; // gray nodes


void SetupMaze();


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
				if (maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != AMMO &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != MEDICATION &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != ROOM_SPACE &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != GROUP_1&&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != GROUP_2)
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
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
			// try up
/*			if (pn->getPoint().getRow() < MSZ - 1)
			{
				Point2D pt;
				pt.setCol(pn->getPoint().getCol());
				pt.setRow(pn->getPoint().getRow() + 1); // going up
				int value = maze[pt.getRow()][pt.getCol()].GetValue();
				double cost;
				if (value == SPACE) cost = space_cost;
				else cost = wall_cost;
				pn1 = new Node(pt, &target, value, pn->getG() + cost, pn);
				// check if this is not black neighbour
				black_it = find(black.begin(), black.end(), pn1); // operator == must be implemented in Node
				if (black_it != black.end())
				{
					// check if pn1 is gray
					gray_it = find(gray.begin(), gray.end(), pn1); // operator == must be implemented in Node
					if (gray_it != gray.end()) // it is already gray
					{
						// check if pn1 has lower f then what was foud before
						if (pn1->getF() < (*gray_it)->getF())
						{
							(*gray_it) = pn1;
							// and update it in PQ!!!!!
						}
					}
						// add pn1 to pq and to gray
				}
			}*/

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

	for (k = 0; k < 30; k++)
	{
		i = rand() % MSZ;
		j = rand() % MSZ;
		maze[i][j].SetValue(WALL);
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
				int v = rand() % 4 + 1;     // v in the range 1 to 4
				//create character1-2
				 pc = new Character(v, 1, i, j);
				 group1.push_back(*pc);
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
				int v = rand() % 4 + 1;     // v in the range 1 to 4
				//create character2
				pc = new Character(v, 1, i, j);
				group2.push_back(*pc);
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
				glColor3d(1,0.5,0); // orange
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
	else if (choice == 3) // generate security map
	{
		glutDisplayFunc(display);
		pb->SetIsMoving(true);
		pg->shoot(pb);
		move_on = true;
	}
}

void mouse(int button, int state, int x, int y)
{
	double xx, yy;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		xx = 2 * (double)x / W - 1;
		yy = 2 * ((double)H - y) / H - 1;

		pb = new Bullet(xx,yy);
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


void nextStep(Point2D start, Point2D target)
{
	priority_queue <Node*, vector<Node*>, CompareNodes> pq;
	vector<Node> gray;
	vector<Node> black;
	Node *pn;
	bool stop = false;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
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
				if (maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != AMMO &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != MEDICATION &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != ROOM_SPACE &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != GROUP_1 &&
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].GetValue() != GROUP_2)
					maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
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
			AddNeighboursChar(pn, gray, black, pq);
		}
	}
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
	glutAddMenuEntry("Shoot", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);



	init();

	glutMainLoop();
}