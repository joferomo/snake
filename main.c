#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef float coords[2];

enum DIRECTION {
	STATIONARY,
	UP,
	DOWN,
	LEFT,
	RIGHT
} snakeDirection, prevSnakeDirection;

coords* snakePos;
coords fruitPos = { 0.0f, 0.0f };
int snakeLength = 1;

float moveInterval = 0.1f;
float timeElapsed = 0.0f;
float t = 0.0f;
float t0 = 0.0f;

int keysPressed[] = { 0, 0, 0, 0, 0 };

int gameOver = 0;

void loadShader(unsigned int id, const char* file)
{
	FILE* shaderFile = fopen(file, "r");
	if(shaderFile == NULL)
	{
		fprintf(stderr, "Failed to open file %s\n", file);
		return;
	}
	char buffer[128];
	char* source = malloc(sizeof (char));
	*source = '\0';
	int n = 0;
	while(fgets(buffer, 128, shaderFile))
	{
		source = realloc(source, sizeof (char) * 128 * (++n));
		strncat(source, buffer, 128);
	}
	fclose(shaderFile);
	const char* sourcePtr = source;
	glShaderSource(id, 1, &sourcePtr, NULL);
	free(source);

	glCompileShader(id);
	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		char infoLog[512];
		glGetShaderInfoLog(id, 512, 0, infoLog);
		fprintf(stderr, "Failed to load shader %d:\n%s\n", id, infoLog);
	}
}

void restartGame()
{
	snakePos = realloc(snakePos, sizeof (coords));
	snakePos[0][0] = 0.0f;
	snakePos[0][1] = -4.0f / 21.0f;
	snakeLength = 1;

	fruitPos[0] = 0.0f;
	fruitPos[1] = 0.0f;

	snakeDirection = STATIONARY;
	prevSnakeDirection = STATIONARY;

	t = 0.0f;
	t0 = 0.0f;
	timeElapsed = 0.0f;

	gameOver = 0;
}

void getPlayerInput(GLFWwindow* window)
{
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && keysPressed[0] == 0)
	{
		snakeDirection = UP;
		keysPressed[0] = 1;
	}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && keysPressed[1] == 0)
	{
		snakeDirection = DOWN;
		keysPressed[1] = 1;
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && keysPressed[2] == 0)
	{
		snakeDirection = LEFT;
		keysPressed[2] = 1;
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && keysPressed[3] == 0)
	{
		snakeDirection = RIGHT;
		keysPressed[3] = 1;
	}
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && keysPressed[4] == 0)
	{
		restartGame();
		keysPressed[4] = 1;
	}
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
	{
		keysPressed[0] = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
	{
		keysPressed[1] = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
	{
		keysPressed[2] = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
	{
		keysPressed[3] = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
	{
		keysPressed[4] = 0;
	}
}

int main()
{
	srand(time(NULL));

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(16*21, 16*21, "Snake", NULL, NULL);
	glfwMakeContextCurrent(window);

	const GLFWvidmode* screen = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	glfwSetWindowPos(window, (screen->width - windowWidth) / 2, (screen->height - windowHeight) / 2);

	gladLoaderLoadGL();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	loadShader(vertexShader, "vertexShader.glsl");

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	loadShader(fragmentShader, "fragmentShader.glsl");

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float size = 1.0f / 21.0f;

	float vertices[] = {
		-size,  size,
		 size,  size,
		 size, -size,
		-size, -size
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 2
	};

	unsigned int vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (void*) 0);
	glEnableVertexAttribArray(0);

	float color[4];

	snakePos = malloc(sizeof (coords));
	snakePos[0][0] = 0.0f;
	snakePos[0][1] = -4.0f / 21.0f;

	glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
	do
	{
		t = glfwGetTime();
		timeElapsed += t - t0;

		glClear(GL_COLOR_BUFFER_BIT);

		getPlayerInput(window);
		if(gameOver == 0)
		{
			// Game logic
			if(timeElapsed >= moveInterval)
			{
				// Head movement
				timeElapsed = 0.0f;
				coords prevSnakePos = { snakePos[0][0], snakePos[0][1] };
				switch(snakeDirection)
				{
					case UP:
						if(prevSnakeDirection == DOWN)
						{
							snakeDirection = prevSnakeDirection;
						}
						break;
					case DOWN:
						if(prevSnakeDirection == UP)
						{
							snakeDirection = prevSnakeDirection;
						}
						break;
					case LEFT:
						if(prevSnakeDirection == RIGHT)
						{
							snakeDirection = prevSnakeDirection;
						}
						break;
					case RIGHT:
						if(prevSnakeDirection == LEFT)
						{
							snakeDirection = prevSnakeDirection;
						}
						break;
				}
				prevSnakeDirection = snakeDirection;
				switch(snakeDirection)
				{
					case UP:
						snakePos[0][1] += 2.0f / 21.0f;
						break;
					case DOWN:
						snakePos[0][1] -= 2.0f / 21.0f;
						break;
					case LEFT:
						snakePos[0][0] -= 2.0f / 21.0f;
						break;
					case RIGHT:
						snakePos[0][0] += 2.0f / 21.0f;
						break;
				}

				// Game over conditions
				if(snakePos[0][0] > 20.0f / 21.0f || snakePos[0][0] < -20.0f / 21.0f || snakePos[0][1] > 20.0f / 21.0f || snakePos[0][1] < -20.0f / 21.0f)
				{
					snakePos[0][0] = prevSnakePos[0];
					snakePos[0][1] = prevSnakePos[1];
					gameOver = 1;
				}
				else
				{
					int headPosX = snakePos[0][0] * 100;
					int headPosY = snakePos[0][1] * 100;
					for(int i = snakeLength-1; i > 0; i--)
					{
						if(headPosX == (int) (snakePos[i][0] * 100) && headPosY == (int) (snakePos[i][1] * 100))
						{
							snakePos[0][0] = prevSnakePos[0];
							snakePos[0][1] = prevSnakePos[1];
							gameOver = 1;
						}
					}
				}

				// Fruit collision
				if(gameOver == 0)
				{
					if((int) (snakePos[0][0] * 100) == (int) (fruitPos[0] * 100) && (int) (snakePos[0][1] * 100) == (int) (fruitPos[1] * 100))
					{
						coords prevFruitPos;
						prevFruitPos[0] = fruitPos[0];
						prevFruitPos[1] = fruitPos[1];
						do
						{
							fruitPos[0] = (-10 + rand() % 20) * 2.0f / 21.0f;
							fruitPos[1] = (-10 + rand() % 20) * 2.0f / 21.0f;
						} while((int) (fruitPos[0] * 100) == (int) (prevFruitPos[0] * 100) && (int) (fruitPos[1] * 100) == (int) (prevFruitPos[1] * 100));
						snakePos = realloc(snakePos, sizeof (coords) * (++snakeLength));
					}
				}

				// Segment movement
				if(gameOver == 0)
				{
					for(int i = snakeLength-1; i > 1; i--)
					{
						snakePos[i][0] = snakePos[i-1][0];
						snakePos[i][1] = snakePos[i-1][1];
					}
					snakePos[1][0] = prevSnakePos[0];
					snakePos[1][1] = prevSnakePos[1];
				}
			}
		}

		// Draw snake
		color[0] = 0.0f;
		color[1] = 1.0f;
		color[2] = 0.0f;
		color[3] = 1.0f;
		glUniform4fv(glGetUniformLocation(shaderProgram, "color"), 1, color);

		for(int i = 0; i < snakeLength; i++)
		{
			float snakeTransform[16] = {
				1.0f,			0.0f,			0.0f,	0.0f,
				0.0f,			1.0f,			0.0f,	0.0f,
				0.0f,			0.0f,			1.0f,	0.0f,
				snakePos[i][0],	snakePos[i][1],	0.0f,	1.0f,
			};
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, snakeTransform);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		if(gameOver == 0)
		{
			// Draw fruit
			color[0] = 1.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			color[3] = 1.0f;
			glUniform4fv(glGetUniformLocation(shaderProgram, "color"), 1, color);

			float fruitTransform[16] = {
				0.8f,			0.0f,			0.0f,	0.0f,
				0.0f,			0.8f,			0.0f,	0.0f,
				0.0f,			0.0f,			0.8f,	0.0f,
				fruitPos[0],	fruitPos[1],	0.0f,	1.0f,
			};
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, fruitTransform);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);

		glfwPollEvents();

		t0 = t;
	} while(glfwWindowShouldClose(window) == GLFW_FALSE);

	free(snakePos);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}
