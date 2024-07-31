#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <random>

const float DEG2RAD = 3.14159 / 180;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

float GetRandomColor() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return dis(gen);
}

int GetRandomDirection() {
	int randomDirection[3] = { 1,5,6 };
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 2);
	return randomDirection[(int)dis(gen)];
}

int SetMovementDirection(int direction)
{
	// 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	switch (direction) {
	case 1:
	case 2:
		// up -> down, down -> up
		direction += 2;
		//direction = (rand() % 8) + 2;
		break;
	case 3:
	case 4:
		// left -> right, right -> left
		direction -= 2;
		//direction = (rand() % 8) - 2;
		break;
	case 5:
		// up right -> down left
		direction = 8;
		break;
	case 6:
		// up left -> down right
		direction = 7;
		break;
	case 7:
		// down right -> up left
		direction = 6;
		break;
	case 8:
		// down left -> up right
		direction = 5;
		break;
	default:
		// Go total random - Allows me to force random to similuate Kinetic Molecular Theory
		direction = (rand() % 8) + 1;
	}
	return direction;
}

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.02;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, float yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = GetRandomColor();
		green = GetRandomColor();
		blue = GetRandomColor();
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				// Slow the ball due to the mass of the ball being less than the brick, transferred to brick
				speed -= .001;
				// Lower bounds so they don't come to a halt
				if (speed == 0.01) {
					speed = 0.01;
				}

				// Once hit, the brick is tainted and is now destructable.
				brk->brick_type = DESTRUCTABLE;

				// Ball direction will go into the opposite direction it hit
				direction = SetMovementDirection(direction);
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				// Speed up the ball due to kinetic transfer
				speed += .001;
				// Upper bounds so they don't go so fast they appear to only be blinking rapidly
				if (speed > 0.09) {
					speed = 0.09;
				}

				// Once hit, the brick will shrink by half. Once it is smaller than 0.01 it will be destroyed
				brk->width /= 2;
				if (brk->width < .01) {
					brk->onoff = OFF;
				}

				// Change to random color on hit if destructive, indicating that it is volatile
				brk->red = GetRandomColor();
				brk->blue = GetRandomColor();
				brk->green = GetRandomColor();

				// Same for circle
				red = GetRandomColor();
				green = GetRandomColor();
				blue = GetRandomColor();
			}
		}
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = SetMovementDirection(-1);
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = SetMovementDirection(-1);
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = SetMovementDirection(-1);
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = SetMovementDirection(-1);
			}
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			direction = SetMovementDirection(-1);
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

std::vector<Circle> world;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		::exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Coding Collisions by Ryan Sherer", NULL, NULL);
	if (!window) {
		glfwTerminate();
		::exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Bottom row - 5 bricks
	Brick brick0(REFLECTIVE,    -0.500, -0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick1(DESTRUCTABLE,  -0.250, -0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick2(DESTRUCTABLE,   0.000, -0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick3(REFLECTIVE,     0.250, -0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick4(REFLECTIVE,     0.500, -0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());

	// 4 bricks
	Brick brick5(DESTRUCTABLE,  -0.375,  0.000, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick6(DESTRUCTABLE,  -0.125,  0.000, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick7(REFLECTIVE,     0.125,  0.000, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick8(REFLECTIVE,     0.375,  0.000, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());

	// 3 bricks
	Brick brick9(DESTRUCTABLE,  -0.250,  0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick10(DESTRUCTABLE,  0.000,  0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick11(DESTRUCTABLE,  0.250,  0.250, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());

	// 2 bricks
	Brick brick12(DESTRUCTABLE, -0.125,  0.500, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());
	Brick brick13(DESTRUCTABLE,  0.125,  0.500, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());

	// Top row - 1 brick
	Brick brick14(DESTRUCTABLE,  0.000,  0.750, 0.2, GetRandomColor(), GetRandomColor(), GetRandomColor());


	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			world[i].CheckCollision(&brick0);
			world[i].CheckCollision(&brick1);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&brick6);
			world[i].CheckCollision(&brick7);
			world[i].CheckCollision(&brick8);
			world[i].CheckCollision(&brick9);
			world[i].CheckCollision(&brick10);
			world[i].CheckCollision(&brick11);
			world[i].CheckCollision(&brick12);
			world[i].CheckCollision(&brick13);
			world[i].CheckCollision(&brick14);
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		brick0.drawBrick();
		brick1.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		brick6.drawBrick();
		brick7.drawBrick();
		brick8.drawBrick();
		brick9.drawBrick();
		brick10.drawBrick();
		brick11.drawBrick();
		brick12.drawBrick();
		brick13.drawBrick();
		brick14.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	::exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b, radius;
		float x, y;
		r = GetRandomColor();
		g = GetRandomColor();
		b = GetRandomColor();
		radius = 0.2;
		x = 0.8;
		y = -0.8;
		// Since we're simulating Kinetic Molecular Theory, direction must be random
		Circle B(x, y, radius, SetMovementDirection(-1), 0.05, r, g, b);
		world.push_back(B);
	}
}
