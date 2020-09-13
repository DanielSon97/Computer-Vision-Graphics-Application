#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>

using namespace std;

//declaring input file name (default: "Test.txt")
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
vector <vector <point>> p; //list of data points
vector <vector <point>> drows; //list of first set of row control points
vector <vector <point>> dcols; //list of first set of column control points
vector <vector <point>> dcols2; //list of first set of column control points
vector <vector <point>> d; //list of control points
int m; //max value of i (m)
int n; //max value of j (n)
int k = 4; //order (k)
int l = 4; //order (l)
vector <vector <float>> w; //list of weights (w)
vector <float> u; //knot vector (u)
vector <float> v; //knot vector (v)
int resolution = 50; //rendering resolution

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

//delcaring computation functions
void solveRows();
vector <point> rowCP(vector <point> pin);
void convertRC();
void solveCP();
void homogenizeD();
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
	//printInfo();
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
	p.clear();
	d.clear();
	drows.clear();
	dcols.clear();
	dcols2.clear();
	w.clear();
	u.clear();
	v.clear();
	d2.clear();
	s.clear();
	s2.clear();

	//starting input process
	string line; //declaring temporary transfer string variable
	int i, j; //declaring temporary iterator variables
	float x, y, z; //declaring temporary point variables

	getline(inFile, line); //skipping text line

	//setting control points and weight
	while (!inFile.eof()) {
		inFile >> i;
		inFile >> j;
		inFile >> x;
		inFile >> y;
		inFile >> z;
		getline(inFile, line);

		if (j == 0) {
			p.resize(p.size() + 1); //increase p size
		}

		p[i].push_back(makePoint(x, y, z));
	}

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

	solveRows();
	convertRC();
	solveCP();
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

void solveRows() {
	for (unsigned int i = 0; i < p.size(); i++) {
		drows.push_back(rowCP(p[i]));
	}
}
//function to calculate control points for a row
vector <point> rowCP(vector <point> pin) {
	vector <point> dout;
	int k = pin.size() - 1;
	int m = k + 2;
	float w;

	//initialzing all matrix simulation vectors
	vector <float> a;
	vector <float> b;
	vector <float> c;
	vector <point> dp;
	a.resize(m + 1, 1);
	a[0] = 0;
	a.back() = 0;
	b.resize(m + 1, 4);
	b[0] = 1;
	b.back() = 1;
	c.resize(m + 1, 1);
	c[0] = 0;
	c.back() = 0;
	dp.resize(m + 1);

	dp[0].x = pin[0].x;
	dp[0].y = pin[0].y;
	dp[0].z = pin[0].z;
	for (int i = 1; i <= m - 1; i++) {
		dp[i].x = 6 * pin[i - 1].x;
		dp[i].y = 6 * pin[i - 1].y;
		dp[i].z = 6 * pin[i - 1].z;
	}
	dp[m].x = pin.back().x;
	dp[m].y = pin.back().y;
	dp[m].z = pin.back().z;

	dout.resize(m + 1); //resizing return vector
	
	//solving tridiagonal matrix
	for (int i = 1; i <= m; i++) {
		w = a[i] / b[i - 1];
		b[i] = b[i] - (w * c[i - 1]);
		dp[i].x = dp[i].x - (w * dp[i - 1].x);
		dp[i].y = dp[i].y - (w * dp[i - 1].y);
		dp[i].z = dp[i].z - (w * dp[i - 1].z);
	}

	dout[m].x = dp[m].x / b[m];
	dout[m].y = dp[m].y / b[m];
	dout[m].z = dp[m].z / b[m];

	for (int i = m - 1; i >= 0; i--) {
		dout[i].x = (dp[i].x - (c[i] * dout[i + 1].x)) / b[i];
		dout[i].y = (dp[i].y - (c[i] * dout[i + 1].y)) / b[i];
		dout[i].z = (dp[i].z - (c[i] * dout[i + 1].z)) / b[i];
	}

	return dout;
}
void convertRC() {
	dcols.resize(drows[0].size());
	for (unsigned int i = 0; i < drows[0].size(); i++) {
		for (unsigned int j = 0; j < drows.size(); j++) {
			dcols[i].push_back(drows[j][i]);
		}
	}
}

void solveCP() {
	for (unsigned int i = 0; i < dcols.size(); i++) {
		dcols2.push_back(rowCP(dcols[i]));
	}

	d.resize(dcols2[0].size());
	for (unsigned int i = 0; i < dcols2[0].size(); i++) {
		for (unsigned int j = 0; j < dcols2.size(); j++) {
			d[i].push_back(dcols2[j][i]);
		}
	}

	m = p[0].size();
	n = p.size();
	w.resize(d.size());
	for (unsigned int i = 0; i < w.size(); i++) {
		w[i].resize(d[0].size(), 1);
	}
	for (int i = 0; i <= m + k; i++) {
		u.push_back(i);
	}
	for (int i = 0; i <= n + l; i++) {
		v.push_back(i);
	}
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
	//printing data points
	cout << "DATA POINTS" << endl;
	for (unsigned int i = 0; i < p.size(); i++) {
		for (unsigned int j = 0; j < p[i].size(); j++) {
			cout << "Point(" << i << ", " << j << "): (" << p[i][j].x << ", " << p[i][j].y << ", " << p[i][j].z << ")" << endl;
		}
	}

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
	}
}
