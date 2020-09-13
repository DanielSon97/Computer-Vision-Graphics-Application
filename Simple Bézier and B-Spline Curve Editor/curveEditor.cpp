#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>

using namespace std;

//declaring input file name (default: "Input File.txt")
string filename = "DefaultInputFile.txt";

//declaring pixel buffer
float* PixelBufferXY;
float* PixelBufferXZ;
float* PixelBufferYZ;

//declaring point struct and constructor
struct point {
	float x;
	float y;
	float z;
};
point makePoint(float x, float y, float z) {
	point A = { x, y, z };
	return A;
}

//making 4D data struct for homogenized control points
struct point2 {
	float x;
	float y;
	float z;
	float w;
};

//declaring user inputted information variables
int m; //max value of i (m)
int n; //max value of j (n)
int k; //order (k)
int l; //order (l)
vector <vector <point>> d; //list of control points (d)
vector <vector <float>> w; //list of weights (w)
vector <float> u; //knot vector (u)
vector <float> v; //knot vector (v)
int resolution; //rendering resolution

//declaring working variables
vector <vector <point2>> d2; //list of homogenized control points (d2)
vector <vector <point>> s; //list of graphed points (s)
vector <vector <point2>> s2; //list of homogenized graphed points (s2)

//declaring window size variables
int wX = 500;
int wY = 500;

//declaring display functions for each window
void displayXY();
void displayXZ();
void displayYZ();

//declaring function for reading input file data
void readFile();

//declaring basic drawing functions
void draw();
void drawline(float x0, float y0, float xE, float yE, float* buffer, float r, float g, float b);
void fillPixel(int x, int y, float r, float g, float b, float* buffer);

//declaring computation functions
void homogenizeD();
float BSBF(int i, int j, vector <float> kv, float value);
point2 solveS2(float uv, float vv);
void computeS2();
void inhomogenizeS2();

//declaring info printing function
void printInfo();

//declaring menu function
void menu();

/*--------------------IMPLEMENTATION--------------------*/

//main function
int main(int argc, char* argv[]) {
	//declaring size of buffers
	PixelBufferXY = new float[wX * wY * 3];
	PixelBufferXZ = new float[wX * wY * 3];
	PixelBufferYZ = new float[wX * wY * 3];

	//initiating windows for displaying orthgonoal projection
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE);

	glutInitWindowSize(wX, wY);
	glutInitWindowPosition(25, 25);
	int XY = glutCreateWindow("XY");
	glClearColor(0, 0, 0, 0);
	glutDisplayFunc(displayXY);

	glutInitWindowSize(wX, wY);
	glutInitWindowPosition(525, 25);
	int XZ = glutCreateWindow("XZ");
	glClearColor(0, 0, 0, 0);
	glutDisplayFunc(displayXZ);

	glutInitWindowSize(wX, wY);
	glutInitWindowPosition(1025, 25);
	int YZ = glutCreateWindow("YZ");
	glClearColor(0, 0, 0, 0);
	glutDisplayFunc(displayYZ);

	readFile();
	printInfo();
	draw();

	//menu and drawing loop
	glutMainLoop();

	return 0;
}

//default glut display for orthogonal projection
void displayXY() {
	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//draws pixel on screen, width and height must match pixel buffer dimension
	glDrawPixels(wX, wY, GL_RGB, GL_FLOAT, PixelBufferXY);

	//window refresh
	glFlush();
	glutPostRedisplay();
}
void displayXZ() {
	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//draws pixel on screen, width and height must match pixel buffer dimension
	glDrawPixels(wX, wY, GL_RGB, GL_FLOAT, PixelBufferXZ);

	//window refresh
	glFlush();
	glutPostRedisplay();
}
void displayYZ() {
	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//draws pixel on screen, width and height must match pixel buffer dimension
	glDrawPixels(wX, wY, GL_RGB, GL_FLOAT, PixelBufferYZ);

	//window refresh
	glFlush();
	glutPostRedisplay();

	//displaying menu
	menu();
}

//function for reading input file data
void readFile() {
	//reading text file
	ifstream inFile;
	inFile.open(filename);

	//file name validity check
	while (!inFile) {
		cout << "Unable to open file." << endl;
		cout << "Enter a valid input file name or \"exit\" to exit: ";
		cin >> filename;
		if (filename == "exit") {
			exit(1);
		}
		else {
			inFile.open(filename);
		}
		cout << endl;
	}

	//clearing storage vectors
	d.clear();
	w.clear();
	u.clear();
	v.clear();

	//starting input process
	string line; //declaring temporary transfer string variable
	int i, j; //declaring temporary iterator variables

	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line

	inFile >> line; //setting m
	m = stoi(line);
	inFile >> line; //setting n
	n = stoi(line);

	//setting sizes for points and weight vectors
	d.resize(m + 1);
	w.resize(m + 1);
	for (int a = 0; a <= m; a++) {
		d[a].resize(n + 1);
		w[a].resize(n + 1);
	}

	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line

	inFile >> line; //setting k
	k = stoi(line);
	inFile >> line; //setting l
	l = stoi(line);

	//checking if order is within bounds
	if (m + 1 <= k - 1 || n + 1 <= l - 1) {
		cout << "Program exited due to invalid order value." << endl;
		exit(EXIT_FAILURE);

	}

	//setting sizes of knot vectors
	u.resize(m + k + 1);
	v.resize(n + l + 1);

	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line

	//setting control points and weight
	for (int a = 0; a < (m + 1) * (n + 1); a++) {
		inFile >> line; //setting i
		i = stoi(line);
		inFile >> line; //setting j
		j = stoi(line);
		inFile >> line; //setting x
		d[i][j].x = stof(line);
		inFile >> line; //setting y
		d[i][j].y = stof(line);
		inFile >> line; //setting z
		d[i][j].z = stof(line);
		inFile >> line; //setting w
		w[i][j] = stof(line);
	}

	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line

	//setting knot vector u
	for (int a = 0; a <= m + k; a++) {
		inFile >> line;
		u[a] = stof(line);
		getline(inFile, line);
	}

	getline(inFile, line); //skipping text line

	//setting knot vector v
	for (int a = 0; a <= n + l; a++) {
		inFile >> line;
		v[a] = stof(line);
		getline(inFile, line);
	}

	getline(inFile, line); //skipping text line
	getline(inFile, line); //skipping text line

	//setting rendering resolution
	inFile >> line;
	resolution = stoi(line);

	//closing file
	inFile.close();

	return;
}

//draw function
void draw() {
	//clearing and resetting buffers
	delete[] PixelBufferXY;
	delete[] PixelBufferXZ;
	delete[] PixelBufferYZ;
	PixelBufferXY = new float[wX * wY * 3];
	PixelBufferXZ = new float[wX * wY * 3];
	PixelBufferYZ = new float[wX * wY * 3];

	//compute s
	homogenizeD();
	computeS2();
	inhomogenizeS2();


	//draw in lines between points in s
	for (int i = 0; i < resolution; i++) {
		for (int j = 0; j < resolution; j++) {
			if (i + 1 < resolution) {
				drawline(s[i][j].x, s[i][j].y, s[i + 1][j].x, s[i + 1][j].y, PixelBufferXY, 255, 255, 255); //drawing for XY projection
				drawline(s[i][j].x, s[i][j].z, s[i + 1][j].x, s[i + 1][j].z, PixelBufferXZ, 255, 255, 255); //drawing for XZ projection
				drawline(s[i][j].y, s[i][j].z, s[i + 1][j].y, s[i + 1][j].z, PixelBufferYZ, 255, 255, 255); //drawing for YZ projection
			}
			if (j + 1 < resolution) {
				drawline(s[i][j].x, s[i][j].y, s[i][j + 1].x, s[i][j + 1].y, PixelBufferXY, 255, 255, 255); //drawing for XY projection
				drawline(s[i][j].x, s[i][j].z, s[i][j + 1].x, s[i][j + 1].z, PixelBufferXZ, 255, 255, 255); //drawing for XZ projection
				drawline(s[i][j].y, s[i][j].z, s[i][j + 1].y, s[i][j + 1].z, PixelBufferYZ, 255, 255, 255); //drawing for YZ projection
			}
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
	if (x >= 0 && x < wX && y >= 0 && y < wY) {
		buffer[(y * 500 * 3) + (x * 3)] = r;
		buffer[(y * 500 * 3) + (x * 3) + 1] = g;
		buffer[(y * 500 * 3) + (x * 3) + 2] = b;
	}
	return;
}

//function to homogenize control points
void homogenizeD() {
	//clearing d2
	d2.clear();

	//setting size of d2
	d2.resize(m + 1);
	for (int a = 0; a <= m; a++) {
		d2[a].resize(n + 1);
	}

	//setting values for d2
	for (int i = 0; i <= m; i++) {
		for (int j = 0; j <= n; j++) {
			//setting values for d2 using specific i and j
			d2[i][j].x = w[i][j] * d[i][j].x; //setting x
			d2[i][j].y = w[i][j] * d[i][j].y; //setting y
			d2[i][j].z = w[i][j] * d[i][j].z; //setting z
			d2[i][j].w = w[i][j]; //setting w
		}
	}

	return;
}
//function to solve B-spline basis functions
float BSBF(int i, int j, vector <float> kv, float value) {
	//base case
	if (j == 1) {
		if (kv[i] <= value && value < kv[i + 1]) {
			return 1;
		}
		else {
			return 0;
		}
	}

	//recursive case
	else {
		//declaring temporary variable
		float temp;

		//solving for final value
		temp = ((value - kv[i]) / (kv[i + j - 1] - kv[i])) * BSBF(i, j - 1, kv, value);
		temp += ((kv[i + j] - value) / (kv[i + j] - kv[i + 1])) * BSBF(i + 1, j - 1, kv, value);

		//return temp
		return temp;
	}
}
//solves for a single s* value at specified u and v value
point2 solveS2(float uv, float vv) {
	//declaring temp variable and final point variable
	float temp;
	point2 fp = { 0, 0, 0, 0 };

	//solving for S2 with respect to given u and v values
	for (int i = 0; i <= m; i++) {
		for (int j = 0; j <= n; j++) {
			temp = BSBF(i, k, u, uv) * BSBF(j, l, v, vv);
			fp.x += d2[i][j].x * temp;
			fp.y += d2[i][j].y * temp;
			fp.z += d2[i][j].z * temp;
			fp.w += d2[i][j].w * temp;
		}
	}

	//return fp
	return fp;
}
//solves for all s* values by relaying above function for various u and v values
void computeS2() {
	//clearing s2
	s2.clear();

	//setting size of s2 with respect to rendering resolution value
	s2.resize(resolution);
	for (int a = 0; a < resolution; a++) {
		s2[a].resize(resolution);
	}

	//declaring increment and iterator variables for knot vectors u and v
	float utemp = u[k - 1]; //setting utemp to uMin
	float vtemp = v[l - 1]; //setting utemp to vMin
	float ustep = (u[m + 1] - u[k - 1]) / (resolution - 1); //setting ustep
	float vstep = (v[n + 1] - v[l - 1]) / (resolution - 1); //setting vstep


	//solving for s2 values
	for (int i = 0; i < resolution; i++) {
		for (int j = 0; j < resolution; j++) {
			s2[i][j] = solveS2(utemp, vtemp);
			vtemp += vstep;
		}
		vtemp = v[l - 1];
		utemp += ustep;
	}

	return;
}
//inhomogenizes points in s* to get final values for s
void inhomogenizeS2() {
	//clearing s
	s.clear();

	//setting size of s with respect to rendering resolution value
	s.resize(resolution);
	for (int a = 0; a < resolution; a++) {
		s[a].resize(resolution);
	}

	//declaring temporary iterator variables
	int i = 0, j = 0;

	//setting values for s
	for (int i = 0; i < resolution; i++) {
		for (int j = 0; j < resolution; j++) {
			//setting values for s by inhomogenizing s2
			s[i][j].x = s2[i][j].x / s2[i][j].w; //setting x
			s[i][j].y = s2[i][j].y / s2[i][j].w; //setting y
			s[i][j].z = s2[i][j].z / s2[i][j].w; //setting z
		}
	}

	return;
}

//function to print information about input data
void printInfo() {
	//printing control point set size values
	cout << "CONTROL POINT SET SIZES" << endl;
	cout << "Max Value of i (m): " << m << endl;
	cout << "Max Value of j (n): " << n << endl;
	cout << endl;

	//printing order values
	cout << "ORDERS" << endl;
	cout << "Order (k): " << k << endl;
	cout << "Order (l): " << l << endl;
	cout << endl;

	//printing control points and weights
	cout << "CONTROL POINTS AND WEIGHTS" << endl;
	int i = 0, j = 0; //declaring temporary iterator variables
	for (int a = 0; a < (m + 1) * (n + 1); a++) {
		//setting i and j
		if (j > n) {
			i++;
			j = 0;
		}

		//printing corresponding point data and weight for current i and j
		cout << "Point(" << i << ", " << j << "): (" << d[i][j].x << ", " << d[i][j].y << ", " << d[i][j].z << ") | Weight: " << w[i][j] << endl;

		//increment j after each loop
		j++;
	}
	cout << endl;

	//printing knot vectors
	cout << "KNOT VECTORS" << endl;
	cout << "Knot Vector (u): {";
	for (unsigned int a = 0; a < u.size() - 1; a++) {
		cout << u[a] << ", ";
	}
	cout << u.back() << "}" << endl;
	cout << "Knot Vector (v): {";
	for (unsigned int a = 0; a < v.size() - 1; a++) {
		cout << v[a] << ", ";
	}
	cout << v.back() << "}" << endl;
	cout << endl;

	//printing rendering resolution
	cout << "RENDERING RESOLUTION" << endl;
	cout << "Resolution: " << resolution << endl;
	cout << endl;

	return;
}

//function to display menu and take inputs
void menu() {
	//declaring temporary input variable
	int result;

	//displaying menu prompt
	cout << "Menu" << endl;
	cout << "The current input file is: " << filename << "." << endl;
	cout << "1. Reload current chosen input file." << endl;
	cout << "2. Choose a new \".txt\" file to take input from." << endl;
	cout << "3. Reload default input file." << endl;
	cout << "Enter the number of the choice: ";

	//taking user input
	cin >> result;
	while (result < 1 || result > 3) {
		cout << "Enter a valid input: ";
		cin >> result;
	}
	cout << endl;

	//taking corresponding action
	switch (result) {
	case 1:
		readFile();
		printInfo();
		draw();
		break;
	case 2:
		cout << "Enter the name of the new file: ";
		cin >> filename;
		readFile();
		printInfo();
		draw();
		break;
	case 3:
		filename = "DefaultInputFile.txt";
		readFile();
		printInfo();
		draw();
		break;
	}
}
