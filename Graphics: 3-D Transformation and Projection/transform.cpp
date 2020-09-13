 #include <GL/glut.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

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

//declaring info classes for each option
struct storageA {
	vector <point> points;
	vector <point> adjusted;
	int degree = 0;
};
storageA* infoA;
struct storageB {
	vector <point> points;
	vector <float> polynomials;
	float t = 0;
};
storageB* infoB;

//declaring various variables
bool clickOn = false;		//detects for point manipulation
int windowX = 500;
int windowY = 500;
int opt = 0;
int chosenPoint = -1;

//declaring basic drawing functions
void resetBuffer();
void display();
void draw();
void drawline(float x0, float y0, float xE, float yE, float* buffer, float r, float g, float b);
void fillPixel(int x, int y, float r, float g, float b, float* buffer);
void drawPolygon(vector <point> pts, float r, float g, float b);
void mouseFunction(int button, int state, int x, int y);
void menuDetection(unsigned char key, int x, int y);
void drawBezier(float tmax, vector <point> pts, float r, float g, float b);
point BezierPoint(float t, vector <point> pts);
void raiseDegree();
void Aitken();
vector <float> AitkenPoly(int i, int j);
void drawPoly(vector <float> poly, float r, float g, float b);
float solvePoly(vector <float> poly, float x);

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

//declaring menu functions
void menu();
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

	if (clickOn == false) {
		menu();
	}

	draw();
}
//draw function
void draw() {
	//resetting PixelBuffer
	resetBuffer();
	
	//if opt 1
	if (opt == 1) {
		//if no points, dont draw
		if (infoA->points.size() < 1) {
			return;
		}

		//drawing original polygon in blue
		drawPolygon(infoA->points, 0, 0, 255);

		//control point minimum checking
		if (infoA->points.size() < 3) {
			return;
		}
		else {
			//draw curve
			drawBezier(1, infoA->points, 0, 255, 0);
			//draw new polygon if degree is not 0
			if (infoA->degree != 0) {
				raiseDegree();
				drawPolygon(infoA->adjusted, 255, 0, 0);
			}
		}
	}

	//if opt 2
	if (opt == 2) {
		//if no points, dont draw
		if (infoB->points.size() < 1) {
			return;
		}

		//drawing original polygon in blue
		drawPolygon(infoB->points, 0, 0, 255);

		//control point minimum checking
		if (infoB->points.size() < 3) {
			return;
		}
		else {
			//draw curve
			Aitken();
		}
	}

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
	if (x >= 0 && x < windowX && y >= 0 && y < windowY) {
		buffer[(y * 500 * 3) + (x * 3)] = r;
		buffer[(y * 500 * 3) + (x * 3) + 1] = g;
		buffer[(y * 500 * 3) + (x * 3) + 2] = b;
	}
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
//mouse click functions
void mouseFunction(int button, int state, int x, int y) {
	//temp variables
	float closest_dist = 10;
	float temp_dist = 0;
	bool point_chose = false;

	//reversing y coordinate
	y = windowY - y;

	//setting points for opt 1
	if (opt == 1) {
		//if left click down
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//check if any point was chosen
			for (unsigned int i = 0; i < infoA->points.size(); i++) {
				temp_dist = dist(float(x), float(y), infoA->points.at(i).x, infoA->points.at(i).y);
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
				infoA->points.push_back(makePoint(float(x), float(y)));
			}
		}
		//if left click up
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			//if any point was chosen previously, then move the point
			if (chosenPoint != -1) {
				infoA->points.at(chosenPoint).x = float(x);
				infoA->points.at(chosenPoint).y = float(y);
				chosenPoint = -1;
			}
		}
		//if right click down
		else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
			//check if any point was chosen
			int temp_chosen = -1;
			for (unsigned int i = 0; i < infoA->points.size(); i++) {
				temp_dist = dist(float(x), float(y), infoA->points.at(i).x, infoA->points.at(i).y);
				//marking chosen point
				if (temp_dist <= closest_dist) {
					closest_dist = temp_dist;
					temp_chosen = i;
				}
			}
			//if a point was chose
			if (temp_chosen != -1) {
				//delete chosen point
				infoA->points.erase(infoA->points.begin() + temp_chosen);
			}
		}
	}

	//setting points for opt 2
	if (opt == 2) {
		//if left click down
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//add a point if size 0
			if (infoB->points.size() == 0) {
				infoB->points.push_back(makePoint(float(x), float(y)));
				infoB->t = float(infoB->points.size() - 1);
			}
			else {
				//add to beginning 
				if (x < infoB->points.at(0).x) {
					infoB->points.insert(infoB->points.begin(), makePoint(float(x), float(y)));
					infoB->t = float(infoB->points.size() - 1);
				}
				//add in between
				else {
					for (unsigned int i = 1; i < infoB->points.size(); i++) {
						//check x doesn't repeat and has greater x value than a point (sorting)
						if (x == infoB->points.at(i).x) {
							return;
						}
						else if (x < infoB->points.at(i).x) {
							infoB->points.insert(infoB->points.begin() + i, makePoint(float(x), float(y)));
							infoB->t = float(infoB->points.size() - 1);
							return;
						}
					}
					//add to end
					infoB->points.push_back(makePoint(float(x), float(y)));
					infoB->t = float(infoB->points.size() - 1);
					return;
				}
			}
		}
		//if right click down
		else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
			//check if any point was chosen
			int temp_chosen = -1;
			for (unsigned int i = 0; i < infoB->points.size(); i++) {
				temp_dist = dist(float(x), float(y), infoB->points.at(i).x, infoB->points.at(i).y);
				//marking chosen point
				if (temp_dist <= closest_dist) {
					closest_dist = temp_dist;
					temp_chosen = i;
				}
			}
			//if a point was chose
			if (temp_chosen != -1) {
				//delete chosen point
				infoB->points.erase(infoB->points.begin() + temp_chosen);
				infoB->t = float(infoB->points.size() - 1);
			}
		}
	}

	return;
}
//check if the menu has been called
void menuDetection(unsigned char key, int x, int y) {
	//menu function key is "q"
	if (key == 'q') {
		clickOn = false;
	}
	return;
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
//function to raise degree
void raiseDegree() {
	//declaring n
	unsigned int n = infoA->points.size() - 1;
	//declaring temp points
	float tempx, tempy;
	//declaring iterator
	unsigned int i;
	
	//clear adjusted list
	infoA->adjusted.clear();

	//raise by the first degree
	//inserting first value into adjusted
	infoA->adjusted.push_back(infoA->points.at(0));

	//inserting the rest values
	for (i = 1; i <= n; i++) {
		tempx = ((float(i) / float(n + 1)) * (infoA->points.at(i - 1).x)) + ((float(n + 1 - i) / float(n + 1)) * (infoA->points.at(i).x));
		tempy = ((float(i) / float(n + 1)) * (infoA->points.at(i - 1).y)) + ((float(n + 1 - i) / float(n + 1)) * (infoA->points.at(i).y));
		infoA->adjusted.push_back(makePoint(tempx, tempy));
	}

	//inserting last value
	infoA->adjusted.push_back(infoA->points.at(n));

	//keep raising if needed
	if (infoA->degree > 1) {
		vector <point> temp;

		//copying adjusted into temp and updating adjusted
		for (int j = 1; j < infoA->degree; j++) {
			temp = infoA->adjusted;
			infoA->adjusted.clear();
			n = int(temp.size() - 1);
			//inserting first value into adjusted
			infoA->adjusted.push_back(temp.at(0));

			//inserting the rest values
			for (i = 1; i <= n; i++) {
				tempx = ((float(i) / float(n + 1)) * (temp.at(i - 1).x)) + ((float(n + 1 - i) / float(n + 1)) * (temp.at(i).x));
				tempy = ((float(i) / float(n + 1)) * (temp.at(i - 1).y)) + ((float(n + 1 - i) / float(n + 1)) * (temp.at(i).y));
				infoA->adjusted.push_back(makePoint(tempx, tempy));
			}

			//inserting last value
			infoA->adjusted.push_back(temp.at(n));
		}
	}
}
//function to draw and compute interpolation polynomial using Aitken's algorithm
void Aitken() {
	//declaring n
	int n = infoB->points.size() - 1;

	//setting polynomial
	infoB->polynomials = AitkenPoly(0, n);

	//drawing
	drawPoly(infoB->polynomials, 0, 255, 0);
}
//calculate polynomial using algorithm
vector <float> AitkenPoly(int i, int j) {
	vector <float> poly;
	if (i == j) {
		poly.push_back(infoB->points.at(i).y);
		return poly;
	}
	else {
		vector <float> tempA = AitkenPoly(i, j - 1);
		vector <float> tempB = AitkenPoly(i + 1, j);
		vector <float> tempA2;
		vector <float> tempB2;
		tempA2.push_back(tempA.at(0));
		tempB2.push_back(tempB.at(0));
		for (unsigned int k = 0; k < tempA.size() - 1; k++) {
			tempA2.push_back((tempA.at(k) * (infoB->points.at(j).x * -1)) + tempA.at(k + 1));
			tempB2.push_back((tempB.at(k) * (infoB->points.at(i).x * -1)) + tempB.at(k + 1));
		}
		tempA2.push_back(tempA.at(tempA.size() - 1) * (infoB->points.at(j).x * -1));
		tempB2.push_back(tempB.at(tempB.size() - 1) * (infoB->points.at(i).x * -1));

		for (unsigned int l = 0; l < tempA2.size(); l++) {
			poly.push_back((tempA2.at(l) - tempB2.at(l)) / (infoB->points.at(i).x - infoB->points.at(j).x));
		}
		//if (j - i > 1) {
			//drawPoly(poly, 255, 0, 0);
		//}
		return poly;
	}
}
//drawing polygons
void drawPoly(vector <float> poly, float r, float g, float b) {
	float maxT;

	//setting maxT
	if (infoB->t == infoB->points.size() - 1) {
		maxT = infoB->points.at(infoB->points.size() - 1).x;
	}
	else {
		float diff = infoB->t - int(infoB->t);
		float temp = infoB->points.at(int(infoB->t) + 1).x - infoB->points.at(int(infoB->t)).x;
		temp = temp * diff;
		maxT = infoB->points.at(int(infoB->t)).x + temp;
	}

	//setting increment for 500
	float inc = (maxT - infoB->points.at(0).x) / 500;

	//declaring temp points
	point p1;
	point p2;

	//drawing
	//solve for p1 using t = 0
	p1.x = infoB->points.at(0).x;
	p1.y = solvePoly(poly, p1.x);

	//drawing curve until user inputted t value
	//curve color is green
	for (float i = infoB->points.at(0).x + inc; i <= maxT; i = i + inc) {
		p2.x = i;
		p2.y = solvePoly(poly, p2.x);
		drawline(p1.x, p1.y, p2.x, p2.y, PixelBuffer, r, g, b);
		p1 = p2;
	}
}
//solve for y in polynomial with given x
float solvePoly(vector <float> poly, float x) {
	float result = 0;
	int it = 0;
	for (int i = poly.size() - 1; i >= 0; i--) {
		result += poly.at(i) * pow(x, it);
		it++;
	}
	return result;
}

//implementing menu functions
void menu() {
	//declaring temp input variable
	int input;

	//main menu
	if (opt == 0) {
		//printing out menu options and taking input
		cout << "Menu" << endl;
		cout << "1. Degree-raising and degree-reduction algorithm" << endl;
		cout << "2. Aitken's algorithm" << endl;
		cout << "4. Exit program" << endl;
		cout << "Enter the number of the choice: ";
		cin >> input;

		//input verification
		while (input < 0 || input > 4) {
			cout << "Enter a valid option: ";
			cin >> input;
		}

		//calling appropriate functions
		switch (input) {
		case 1: 
			opt = 1;				//changing new opt
			infoA = new storageA();	//declaring a new storage
			break;
		case 2:
			opt = 2;				//changing new opt
			infoB = new storageB();	//declaring a new storage
			break;
		}

		cout << endl;
		return;
	}

	//opt 1
	if (opt == 1) {
		//printing out menu options and taking input
		cout << "Menu" << endl;
		cout << "1. Manipulate points" << endl;
		cout << "2. Change degree" << endl;
		cout << "3. Clear everything" << endl;
		cout << "4. Exit program" << endl;
		cout << "Enter the number of the choice: ";
		cin >> input;

		//input verification
		while (input < 0 || input > 4) {
			cout << "Enter a valid option: ";
			cin >> input;
		}
		
		cout << endl;

		//calling appropriate functions
		switch (input) {
		case 1: 
			cout << "Left click on an empty space to add a point." << endl;
			cout << "Left click and hold on a point to drag and move the point." << endl;
			cout << "Right click on a point to remove it." << endl;
			cout << "Press \"q\" in the DISPLAY screen to return to the menu." << endl;
			cout << endl;
			clickOn = true;
			break;
		case 2:
			int degreeChange;
			cout << "The current degree is " << infoA->degree << "." << endl;
			cout << "Enter a number a number to increase(+) or decrease(-) the degree by: ";
			cin >> degreeChange;
			while (infoA->degree + degreeChange < 0) {
				cout << "The resulting degree is negative. Enter a valid number: ";
				cin >> degreeChange;
			}
			infoA->degree += degreeChange;
			cout << "The new degree is " << infoA->degree << "." << endl;
			cout << endl;
			break;
		case 3:
			delete infoA;
			infoA = new storageA;
			cout << "Cleared." << endl;
			cout << endl;
			break;
		case 4: 
			opt = 0;
			delete infoA;
			break;
		}
		return;
	}

	//opt 2
	if (opt == 2) {
		//printing out menu options and taking input
		cout << "Menu" << endl;
		cout << "1. Manipulate points" << endl;
		cout << "2. Change \"t\"" << endl;
		cout << "3. Clear everything" << endl;
		cout << "4. Exit program" << endl;
		cout << "Enter the number of the choice: ";
		cin >> input;

		//input verification
		while (input < 0 || input > 4) {
			cout << "Enter a valid option: ";
			cin >> input;
		}

		cout << endl;

		//calling appropriate functions
		switch (input) {
		case 1:
			cout << "Left click on an empty space to add a point." << endl;
			cout << "Right click on a point to remove it." << endl;
			cout << "Press \"q\" in the DISPLAY screen to return to the menu." << endl;
			cout << endl;
			clickOn = true;
			break;
		case 2:
			float newT;
			cout << "Enter a new \"t\" value between 0 and " << infoB->points.size() - 1 << ": ";
			cin >> newT;
			while (newT < 0 || newT > infoB->points.size() - 1) {
				cout << "Enter a valid option: ";
				cin >> newT;
			}
			infoB->t = newT;

			cout << endl;
			break;
		case 3:
			delete infoB;
			infoB = new storageB;
			cout << "Cleared." << endl;
			cout << endl;
			break;
		case 4:
			opt = 0;
			delete infoB;
			break;
		}
		return;
	}
}
//function to exit program
void endProgram() {
	exit(EXIT_SUCCESS);
}
