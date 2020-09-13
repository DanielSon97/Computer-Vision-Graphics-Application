#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <time.h>

using namespace std;

//default input file names
string fileA = "points_input.txt";

//declaring pixel buffer
float* PixelBuffer;

//declaring point struct and constructor
struct point {
	float x;
	float y;
};
point makePoint(float x, float y) {
	point A = { x, y };
	return A;
}

//declaring a vector to store list of control points
vector <point> points;
vector <point> pointsA;
vector <point> pointsB;

//declaring various variables
float tInput = 1; //default t value is 1
int curve_mode = 0; //single curve = 0 | double curve = 1
int windowX = 500;
int windowY = 500;
int chosenPoint = -1;

//declaring basic drawing functions
void resetBuffer();
void readFile();
void display();
void mouseFunction(int button, int state, int x, int y);
void menuDetection(unsigned char key, int x, int y);
void draw();
void drawline(float a0, float b0, float aE, float bE, float* buffer, float r, float g, float b);
void fillPixel(int x, int y, float r, float g, float b, float* buffer);

//declaring specific drawing functions
void drawPolygon(vector <point> pts, float r, float g, float b);
void drawSegments(float t);
void drawCasteljau(float tmax);
point CasteljauPoint(int i, int j, float t);
void drawBezier(float tmax, vector <point> pts, float r, float g, float b);
point BezierPoint(float t, vector <point> pts);

//declaring two curve functions
point CasteljauPointDivide(int i, int j, float t);

//calculation functions
int factorial(int a) {
	if (a <= 1) {
		return 1;
	}
	else {
		return a * factorial(a - 1);
	}
}
int combinations(int n, int r) {
	return factorial(n) / (factorial(r) * factorial(n - r));
}
float dist(float x1, float y1, float x2, float y2) {
	return float(sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0));
}

//declaring menu functions- 
int menu();
void resetControlPoints();
void changeT();
void compareTimes();
void subdivideCurve();
void endProgram();

/*--------------------IMPLEMENTATION--------------------*/

//main function
int main(int argc, char* argv[]) {
	PixelBuffer = new float[windowX * windowY * 3];

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE);

	glutInitWindowSize(windowX, windowY);
	glutInitWindowPosition(25, 25);
	int MainWindow = glutCreateWindow("DISPLAY");
	glutMouseFunc(mouseFunction);
	glutKeyboardFunc(menuDetection);
	glClearColor(0, 0, 0, 0);
	glutDisplayFunc(display);

	//reading input file and drawing in initial buffer
	readFile();
	draw();

	//menu and drawing loop
	glutMainLoop();

	return 0;
}

//function to reset elements in the buffer
void resetBuffer() {
	for (int i = 0; i < windowX * windowY * 3; i++) {
		PixelBuffer[i] = 0;
	}
}
//reads in list of points from input file
void readFile() {
	//reading text file
	ifstream inFile;
	inFile.open(fileA);

	//check if input file is valid
	//enter new file if invalid or exit out
	while (!inFile) {
		cout << "Unable to open file." << endl;
		cout << "Enter a valid input file or \"exit\" to exit: ";
		cin >> fileA;
		if (fileA == "exit") {
			exit(1);
		}
		else {
			inFile.open(fileA);
		}
		cout << endl;
	}

	//declaring temp variables
	string line;
	float x, y;

	//filling in list of points
	while (!inFile.eof()) {
		inFile >> x;
		inFile >> y;

		points.push_back(makePoint(x, y));
	}
	inFile.close();
}
//default glut display function with menu inplementation
void display() {
	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//draws pixel on screen, width and height must match pixel buffer dimension
	glDrawPixels(windowX, windowY, GL_RGB, GL_FLOAT, PixelBuffer);

	//window refresh
	glFlush();
	glutPostRedisplay();
	
	/*
	//menu
	switch (menu()) {
	case 1: changeT();
		break;
	case 2: compareTimes();
		break;
	case 6: endProgram();
		break;
	}
	*/

	//draw();
}
//mouse click functions
void mouseFunction(int button, int state, int x, int y) {
	//temp variables
	float closest_dist = 10;
	float temp_dist = 0;
	bool point_chose = false;

	y = windowY - y;

	//if working with a single curve
	if (curve_mode == 0) {
		//if left click down
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//check if any point was chosen
			for (unsigned int i = 0; i < points.size(); i++) {
				temp_dist = dist(float(x), float(y), points.at(i).x, points.at(i).y);
				//updating chosen point
				if (temp_dist <= closest_dist) {
					closest_dist = temp_dist;
					chosenPoint = i;
					point_chose = true;
				}
			}
			//if no point was chosen
			if (point_chose == false) {
				//add a point
				points.push_back(makePoint(float(x), float(y)));

				//draw and update
				draw();
				glClear(GL_COLOR_BUFFER_BIT);
				glLoadIdentity();
				glDrawPixels(windowX, windowY, GL_RGB, GL_FLOAT, PixelBuffer);
				glFlush();
				glutPostRedisplay();
			}
		}
		//if left click up
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			//if any point was chosen previously, then move the point
			if (chosenPoint != -1) {
				points.at(chosenPoint).x = float(x);
				points.at(chosenPoint).y = float(y);
				chosenPoint = -1;

				//draw and update
				draw();
				glClear(GL_COLOR_BUFFER_BIT);
				glLoadIdentity();
				glDrawPixels(windowX, windowY, GL_RGB, GL_FLOAT, PixelBuffer);
				glFlush();
				glutPostRedisplay();
			}
		}
		//if right click down
		else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
			//check if any point was chosen
			int temp_chosen = -1;
			for (unsigned int i = 0; i < points.size(); i++) {
				temp_dist = dist(float(x), float(y), points.at(i).x, points.at(i).y);
				//marking chosen point
				if (temp_dist <= closest_dist) {
					closest_dist = temp_dist;
					temp_chosen = i;
				}
			}
			//if a point was chose
			if (temp_chosen != -1) {
				//delete chosen point
				points.erase(points.begin() + temp_chosen);

				//draw and update
				draw();
				glClear(GL_COLOR_BUFFER_BIT);
				glLoadIdentity();
				glDrawPixels(windowX, windowY, GL_RGB, GL_FLOAT, PixelBuffer);
				glFlush();
				glutPostRedisplay();
			}
		}
	}
}
//check if the menu has been called
void menuDetection(unsigned char key, int x, int y) {
	//menu key is 'm'
	if (key == 'm') {
		//menu
		switch (menu()) {
		case 0: resetControlPoints();
			break;
		case 1: changeT();
			break;
		case 2: compareTimes();
			break;
		case 3: subdivideCurve();
			break;
		case 4: endProgram();
			break;
		}
	}
	draw();
	return;
}
//fill in PixelBuffer using specific computing method
void draw() {
	//resetting PixelBuffer
	resetBuffer();

	//if no points, dont draw
	if (points.size() < 1) {
		return;
	}

	//if two curve mode
	if (curve_mode == 1) {
		//draw both polygons
		drawPolygon(pointsA, 0, 0, 255);
		drawPolygon(pointsB, 100, 0, 255);

		//draw using Bernstein-Bezier definition
		drawBezier(1, pointsA, 0, 255, 0);
		drawBezier(1, pointsB, 0, 255, 100);
		return;
	}

	//drawing original polygon in blue
	drawPolygon(points, 0, 0, 255);

	//control point minimum checking
	if (points.size() < 3) {
		return;
	}

	//drawing segments in red
	drawSegments(tInput);

	//drawing curve in green
	drawCasteljau(tInput);
	
	return;
}
//function for drawing a straight line in between two points
void drawline(float x0, float y0, float xE, float yE, float* buffer, float r, float g, float b) {
	int dx = int(xE - x0), dy = int(yE - y0);
	int steps, k;
	float xInc, yInc;
	float x = x0, y = y0;

	if (fabs(dx) > fabs(dy)) {
		steps = int(fabs(dx));
	}
	else {
		steps = int(fabs(dy));
	}

	xInc = float(dx) / float(steps);
	yInc = float(dy) / float(steps);

	fillPixel(int(round(x)), int(round(y)), r, g, b, buffer);
	for (k = 0; k < steps; k++) {
		x += xInc;
		y += yInc;
		fillPixel(int(round(x)), int(round(y)), r, g, b, buffer);
	}

	return;
}
//function to fill pixel in PixelBuffer
void fillPixel(int x, int y, float r, float g, float b, float* buffer) {
	buffer[(y * 500 * 3) + (x * 3)] = r;
	buffer[(y * 500 * 3) + (x * 3) + 1] = g;
	buffer[(y * 500 * 3) + (x * 3) + 2] = b;

	return;
}

//function to draw the polygon
void drawPolygon(vector <point> pts, float r, float g, float b) {
	//declaring temp variables
	int x, y;

	//draw lines in between each point
	for (unsigned int i = 0; i < pts.size() - 1; i++) {
		drawline(pts.at(i).x, pts.at(i).y, pts.at(i + 1).x, pts.at(i + 1).y, PixelBuffer, r, g, b);
	}

	//draw a visible point for each point
	for (unsigned int i = 0; i < pts.size(); i++) {
		x = int(pts.at(i).x);
		y = int(pts.at(i).y);

		fillPixel(x - 1, y, 255, 255, 255, PixelBuffer);
		fillPixel(x + 1, y, 255, 255, 255, PixelBuffer);
		fillPixel(x, y - 1, 255, 255, 255, PixelBuffer);
		fillPixel(x, y + 1, 255, 255, 255, PixelBuffer);
	}
}
//function to draw the individual segments
void drawSegments(float t) {
	if (t == 0 || t == 1) {
		return;
	}
	//declaring temp points
	point p1;
	point p2;

	//declaring n
	int n = points.size() - 1; //3

	//solve for each CasteljauPoint
	for (int j = 1; j <= n - 1; j++) {
		//getting each point after and drawing a line
		for (int k = 0; k <= n - 1 - j; k++) {
			p1 = CasteljauPoint(k, j, t);
			p2 = CasteljauPoint(k + 1, j, t);
			drawline(p1.x, p1.y, p2.x, p2.y, PixelBuffer, 255, 0, 0);
		}
	}
}
//function to draw the curve at user specified t value using Casteljau algorithm
void drawCasteljau(float tmax) {
	//declaring temp points
	point p1;
	point p2;

	//declaring increment size
	float inc = 1 / float(1000);

	//declaring n
	int n = points.size() - 1;

	//solve for p1 using t = 0
	p1 = CasteljauPoint(0, n, 0);

	//drawing curve until user inputted t value
	//curve color is green
	for (float t = 0 + inc; t <= tmax; t = t + inc) {
		p2 = CasteljauPoint(0, n, t);
		drawline(p1.x, p1.y, p2.x, p2.y, PixelBuffer, 0, 255, 0);
		p1 = p2;
	}

	return;
}
//function to determine points using Casteljau algorithm
point CasteljauPoint(int i, int j, float t) {
	if (j == 0) {
		return points.at(i);
	}
	else {
		point a = CasteljauPoint(i, j - 1, t);
		point b = CasteljauPoint(i + 1, j - 1, t);
		a.x = a.x * (1 - t);
		a.y = a.y * (1 - t);
		b.x = b.x * t;
		b.y = b.y * t;
		a.x = a.x + b.x;
		a.y = a.y + b.y;
		return a;
	}
}
//function to draw the curve at user specified t value using Bernstein-Bezier definition
void drawBezier(float tmax, vector <point> pts, float r, float g, float b) {
	//declaring temp points
	point p1;
	point p2;

	//declaring increment size
	float inc = 1 / float(1000);

	//solve for p1 using t = 0
	p1 = BezierPoint(0, pts);

	//drawing curve until user inputted t value
	//curve color is green
	for (float t = 0 + inc; t <= tmax; t = t + inc) {
		p2 = BezierPoint(t, pts);
		drawline(p1.x, p1.y, p2.x, p2.y, PixelBuffer, r, g, b);
		p1 = p2;
	}

	return;
}
//function to determine points using Bernstein-Bezier defintion
point BezierPoint(float t, vector <point> pts) {
	//declaring temp variables
	point tempP;
	tempP.x = 0;
	tempP.y = 0;
	float temp;

	//declaring n
	int n = pts.size() - 1;

	//figuring out point by Bezier definition
	for (int i = 0; i <= n; i++) {
		temp = combinations(n, i) * pow((1 - t), (n - i)) * pow(t, i);
		tempP.x += temp * pts.at(i).x;
		tempP.y += temp * pts.at(i).y;
	}

	return tempP;
}

//function to determine points using Casteljau algorithm and to divide when necessary
point CasteljauPointDivide(int i, int j, float t) {
	if (j == 0) {
		//inputting points into 
		if (i == 0) {
			pointsA.push_back(makePoint(points.at(i).x, points.at(i).y));
		}
		else if (i == points.size() - 1) {
			pointsB.push_back(makePoint(points.at(i).x, points.at(i).y));
		}
		return points.at(i);
	}
	else {
		point a = CasteljauPointDivide(i, j - 1, t);
		point b = CasteljauPointDivide(i + 1, j - 1, t);
		a.x = a.x * (1 - t);
		a.y = a.y * (1 - t);
		b.x = b.x * t;
		b.y = b.y * t;
		a.x = a.x + b.x;
		a.y = a.y + b.y;
		if (i == 0) {
			pointsA.push_back(makePoint(a.x, a.y));
		}
		if (i == points.size() - 1 - j) {
			pointsB.push_back(makePoint(a.x, a.y));
		}
		return a;
	}
}

//implementing menu functions
int menu() {
	//declaring temp input variable
	int input;

	//printing out menu options and taking input
	cout << "Menu" << endl;
	cout << "0. Reset display and control points" << endl;
	cout << "1. Change \"t\"" << endl;
//FIX	cout << "2. Change the computing method from de Casteljau algorithm to Bernstein-Bezier defintion and vice versa" << endl;
	cout << "2. Compare times between de Casteljau algorithm and Bernstein-Bezier defintion" << endl;
	cout << "3. Subdivide the curve" << endl;
	cout << "4. Exit program" << endl;
	cout << "Enter the number of the choice: ";
	cin >> input;

	//input verification
	while (input < 0 || input > 4) {
		cout << "Enter a valid option: ";
		cin >> input;
	}

	cout << endl;
	return input;
}
//function to reset display and all control points
void resetControlPoints() {
	//clearing all indexes of points
	points.clear();
	pointsA.clear();
	pointsB.clear();

	//resetting curve_mode
	curve_mode = 0;

	//draw and update
	draw();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glDrawPixels(windowX, windowY, GL_RGB, GL_FLOAT, PixelBuffer);
	glFlush();
	glutPostRedisplay();
}
//function to change "t"
void changeT() {
	//control point minimum checking
	if (points.size() < 3) {
		cout << "You need atleast three control points." << endl;
		cout << endl;
		return;
	}
		 
	//declaring temp input variable
	float input;

	//getting input from user for new "t"
	cout << "Enter a new value for \"t\" between 0 and 1: ";
	cin >> input;

	//input verification
	while (input < 0 || input > 1) {
		cout << "Enter a valid value for \"t\" between 0 and 1: ";
		cin >> input;
	}
	cout << "The new value for \"t\" is " << input << "." << endl;

	//updating t
	tInput = input;

	cout << endl;
	return;
}
//function to compare processing times between de Casteljau algorithm and Bernstein-Bezier representation
void compareTimes() {
	//control point minimum checking
	if (points.size() < 3) {
		cout << "You need atleast three control points." << endl;
		cout << endl;
		return;
	}

	//initializing clocks
	clock_t clockA;
	clock_t clockB;

	//checking time of de Casteljau algorithm
	clockA = clock();
	drawCasteljau(1);
	clockA = clock() - clockA;

	//checking time of Bernstein-Bezier defintion
	clockB = clock();
	drawBezier(1, points, 0, 255, 0);
	clockB = clock() - clockB;

	//
	cout << "The time it takes to run the de Casteljau algorithm is " << clockA << " milliseconds." << endl;
	cout << "The time it takes to run the Bernstein-Bezier definition is " << clockB << " milliseconds." << endl;
	cout << endl;
	return;
}
//function to subdivide the curve
void subdivideCurve() {
	//control point minimum checking
	if (points.size() < 3) {
		cout << "You need atleast three control points." << endl;
		cout << endl;
		return;
	}

	//checking for one curve mode
	if (curve_mode != 0) {
		cout << "There is already a split curve." << endl;
		cout << "Reset the display and try again." << endl;
		return;
	}

	//get user input for "t"
	float tIn;
	cout << "Enter a \"t\" value between 0 and 1 to subdivide the curve at: ";
	cin >> tIn;

	//input verification
	while (tIn <= 0 || tIn >= 1) {
		cout << "Enter a valid value for \"t\" between 0 and 1 non-inclusive: ";
		cin >> tIn;
	}

	//setting curve mode to two curve mode
	curve_mode = 1;

	//resetting both vectors that hold control points
	pointsA.clear();
	pointsB.clear();

	//splitting curve
	CasteljauPointDivide(0, points.size() - 1, tIn);

	cout << endl;
	return;
}
//function to exit program
void endProgram() {
	exit(EXIT_SUCCESS);
}
