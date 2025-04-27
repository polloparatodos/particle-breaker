#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <GLFW/glfw3.h>

// Mathematical Constants
const float DEG2RAD = 3.14159 / 180;

// Speed and Movement Constants
const float MIN_SPEED = 0.01;
const float MAX_SPEED = 0.09;
const float SPEED_DELTA = 0.001;
const int RANDOM_DIRECTION_PROBABILITY = 10;

// Circle Properties
const float INITIAL_RADIUS = 0.02;
const int CIRCLE_SEGMENTS = 360;
const float CIRCLE_START_X = 0.8f;
const float CIRCLE_START_Y = -0.8f;

// Brick Properties
const float BRICK_WIDTH = 0.2f;
const float MIN_BRICK_WIDTH = 0.01f;

// Boundary Constants
const float BOUNDARY_LEFT = -1.0f;
const float BOUNDARY_RIGHT = 1.0f;
const float BOUNDARY_TOP = -1.0f;
const float BOUNDARY_BOTTOM = 1.0f;

// Window Properties
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

void processInput(GLFWwindow* window);

template <typename T>
T GetRandomValue() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<T> dis(0.0f, 1.0f);
	return dis(gen);
}

float GetRandomValue(){
	return GetRandomValue<float>();
}

int SetMovementDirection(int direction) {
    static const std::unordered_map<int, int> directionMap = {
        {1, 3}, {2, 4}, {3, 1}, {4, 2},
        {5, 8}, {6, 7}, {7, 6}, {8, 5}
    };

    auto it = directionMap.find(direction);
    if (it != directionMap.end()) {
        return it->second;
    }

    // Default to a random direction if invalid
    return (rand() % 8) + 1;
}

class Brick {
	private:
		float red, green, blue;
		float x, y, width;
		BRICKTYPE brick_type;
		ONOFF onoff;
	
	public:
		Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
			: brick_type(bt), x(xx), y(yy), width(ww), red(rr), green(gg), blue(bb), onoff(ON) {}
	
		void drawBrick() const {
			if (onoff == ON) {
				float halfside = width / 2;
	
				glColor3d(red, green, blue);
				glBegin(GL_POLYGON);
				glVertex2d(x + halfside, y + halfside);
				glVertex2d(x + halfside, y - halfside);
				glVertex2d(x - halfside, y - halfside);
				glVertex2d(x - halfside, y + halfside);
				glEnd();
			}
		}
	
		void hit() {
			if (brick_type == DESTRUCTABLE) {
				width /= 2;
				if (width < MIN_BRICK_WIDTH) {
					onoff = OFF;
				}
				setColor(GetRandomValue(), GetRandomValue(), GetRandomValue());
			}
		}
	
		bool isColliding(float ballX, float ballY, float ballRadius) const {
			float halfside = width / 2;
			return (ballX + ballRadius > x - halfside && ballX - ballRadius < x + halfside &&
					ballY + ballRadius > y - halfside && ballY - ballRadius < y + halfside);
		}
	
		void setColor(float r, float g, float b) {
			red = r;
			green = g;
			blue = b;
		}
	
		BRICKTYPE getBrickType() const { return brick_type; }
		ONOFF setOnOff() const { return onoff; }
		void setBrickType(BRICKTYPE type) { brick_type = type; }

		float getWidth() const { return width; }
		float getX() const { return x; }
		float getY() const { return y; }
	};
	
	class Circle {
	private:
		float red, green, blue;
		float radius;
		float x, y;
		float speed = MIN_SPEED;
		int direction;

		// Move logic for a specific direction
		void handleDirection(int direction, float& x, float& y, float radius, float speed) {
			auto move = [&](float& coord, float limit, float delta) {
				if ((delta > 0 && coord < limit) || (delta < 0 && coord > limit)) {
					coord += delta;
				} else {
					this->direction = SetMovementDirection(-1);
				}
			};
	
			if (direction == 1 || direction == 5 || direction == 6) {
				move(y, BOUNDARY_TOP + radius, -speed); // Up
			}
			if (direction == 2 || direction == 5 || direction == 7) {
				move(x, BOUNDARY_RIGHT - radius, speed); // Right
			}
			if (direction == 3 || direction == 7 || direction == 8) {
				move(y, BOUNDARY_BOTTOM - radius, speed); // Down
			}
			if (direction == 4 || direction == 6 || direction == 8) {
				move(x, BOUNDARY_LEFT + radius, -speed); // Left
			}
		}
	
	public:
		Circle(double xx, float yy, double rr, int dir, float rad, float r, float g, float b)
			: x(xx), y(yy), radius(rad), direction(dir), red(r), green(g), blue(b) {}
	
		void changeColor() {
			red = GetRandomValue();
			green = GetRandomValue();
			blue = GetRandomValue();
		}
	
		void CheckCollision(Brick* brick) {
			if (!brick || brick->getWidth() <= 0) {
				return; // Invalid brick, skip collision
			}
	
			if (brick->isColliding(x, y, radius)) {
				if (brick->getBrickType() == REFLECTIVE) {
					speed -= SPEED_DELTA;
					if (speed < MIN_SPEED) {
						speed = MIN_SPEED;
					}
					brick->setBrickType(DESTRUCTABLE);
					direction = SetMovementDirection(direction);
				} else if (brick->getBrickType() == DESTRUCTABLE) {
					speed += SPEED_DELTA;
					if (speed > MAX_SPEED) {
						speed = MAX_SPEED;
					}
					brick->hit();
					changeColor();
				}
			}
		}
	
		void MoveOneStep() {
			// Introduce a "buzzing" effect by randomly changing the direction slightly
			if (rand() % RANDOM_DIRECTION_PROBABILITY == 0) { // 10% chance to change direction on each step
				direction = SetMovementDirection(-1);
			}
		
			auto move = [&](float& coord, float limit, float delta) {
				if ((delta > 0 && coord < limit) || (delta < 0 && coord > limit)) {
					coord += delta;
				} else {
					direction = SetMovementDirection(-1);
				}
			};

			handleDirection(direction, x, y, radius, speed);
		}
	
		void DrawCircle() const {
			glColor3f(red, green, blue);
			glBegin(GL_POLYGON);
			for (int i = 0; i < CIRCLE_SEGMENTS; i++) {
				float degInRad = i * DEG2RAD;
			    // This produces the "buzzing" effect of the ball, simulating the kinetic energy transfer
			    // to the ball as it bounces off the bricks. The faster it goes, the more it "buzzes"
			    SetMovementDirection(-1);
				glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
			}
			glEnd();
		}
	};

std::vector<Circle> world;

Brick CreateBrick(BRICKTYPE type, float x, float y, float width){
	return Brick(type, x, y, width, GetRandomValue(), GetRandomValue(), GetRandomValue());
}

void InitializeBricks(std::vector<Brick>& bricks) {
    float yPositions[] = {-0.250, 0.000, 0.250, 0.500, 0.750}; // Y positions for each row
    int rows = 5; // Number of rows

    for (int row = 0; row < rows; row++) {
        int bricksInRow = rows - row; // Number of bricks in the current row
        float startX = -0.500 + (row * 0.125); // Starting X position for the row

        for (int i = 0; i < bricksInRow; i++) {
            float x = startX + i * 0.250;
            BRICKTYPE type = (row == 0 && i % 2 == 0) ? REFLECTIVE : DESTRUCTABLE; // Alternate types for the bottom row
            bricks.push_back(CreateBrick(type, x, yPositions[row], BRICK_WIDTH));
        }
    }
}

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		::exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pseudo Kinetic Molecular Theory Simulator", NULL, NULL);
	if (!window) {
		glfwTerminate();
		::exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	std::vector<Brick> bricks;
	InitializeBricks(bricks);
	
	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		// Explicitly set the background color to black
		// This is the color of the window, not the background of the scene
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// Movement and collision detection
		for (int i = 0; i < world.size(); i++) {
			for (auto& brick : bricks) {
				world[i].CheckCollision(&brick);
			}
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		// Remove destroyed bricks from the vector
		bricks.erase(std::remove_if(bricks.begin(), bricks.end(), [](const Brick& brick) {
			return brick.setOnOff() == OFF;
		}), bricks.end());

		for (auto& brick : bricks) {
			brick.drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	::exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		float r = GetRandomValue(), g = GetRandomValue(), b = GetRandomValue();

		// Since we're simulating Kinetic Molecular Theory, direction must be random
		// I mean, it's not really KMT, but it is a fun way to think about it
		world.emplace_back(CIRCLE_START_X, CIRCLE_START_Y, INITIAL_RADIUS, SetMovementDirection(-1), 0.05, r, g, b);
	}
}