#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <SDL.h>
#undef main // "Hack" because else we get an error using SDL
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void checkShaderCompilationStatus(unsigned int shader);
void checkShaderProgramLinkingStatus(unsigned int shaderProgram);
// --------------------------------------------------------------------------------------------------------
// Shader sources
// --------------------------------------------------------------------------------------------------------
const char* vertexShaderSource = R"(
	#version 330 core
	
	layout (location = 0) in vec3 position;

	uniform mat4 model;

	void main() {
		gl_Position = model * vec4(position, 1.0);
	}
)";

const char* fragmentShaderSource = R"(
	#version 330 core

	out vec4 color;

	uniform vec4 ourColor;

	void main() {
		color = ourColor;
	}
)";

// --------------------------------------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------------------------------------
SDL_Window* window = NULL;
SDL_GLContext glContext;

// Window size
unsigned int windowHeight = 800;
unsigned int windowWidth = 1000;

// Initial window position
unsigned int initialX = 200;
unsigned int initialY = 200;

// Window title
char windowTitle[] = "SDL2Cube";

// Buffer objects
GLuint VAO;
GLuint VBO;
GLuint EBO;

// Is the render loop still running
bool running = true;

// SDL event structure
SDL_Event event;


// --------------------------------------------------------------------------------------------------------
// Data to draw
// --------------------------------------------------------------------------------------------------------
float vertices[] = {
		0.5f,  0.5f, 0.5f,		// 0 Top right, front
		0.5f, -0.5f, 0.5f,		// 1 Bottom right, front
		-0.5f, -0.5f, 0.5f,		// 2 Bottom left, front
		-0.5f,  0.5f, 0.5f,		// 3 Top left, front
		0.5f,  0.5f, -0.5f,		// 4 Top right, back
		0.5f, -0.5f, -0.5f,		// 5 Bottom right, back
		-0.5f, -0.5f, -0.5f,	// 6 Bottom left, back
		-0.5f,  0.5f, -0.5f,	// 7 Top left, back
};

GLuint indices[] = {
	// Front
	0, 1, 2,
	0, 2, 3,

	// Left
	2, 3, 6,
	3, 6, 7,

	// Back
	5, 6, 7,
	4, 5, 7,

	// Right
	0, 1, 5,
	0, 4, 5,

	// Top
	0, 3, 7,
	0, 4, 7,

	// Bottom
	1, 2, 6,
	1, 5, 6
};

GLuint vertexAttributes = 3;
GLuint triangleCount = 12;
GLuint indexCount = triangleCount * 3;

int main() {
	// --------------------------------------------------------------------------------------------------------
	// Initialize SDL, window and context
	// --------------------------------------------------------------------------------------------------------

	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Failed to initialize SDL" << std::endl;
		return 1;
	}

	// Use hardware acceleration
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// Gonna use OpenGL 3.3 for this
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Use the doublebuffer (swapchain) of OpenGL
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


	// Create the window
	window = SDL_CreateWindow(windowTitle, initialX, initialY, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		return 1;
	}

	// Create OpenGL context to use
	glContext = SDL_GL_CreateContext(window);
	if (glContext == NULL) {
		std::cout << "Failed to create OpenGL context" << std::endl;
		return 1;
	}

	// Load OpenGL functions via Glad / SDL
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	// Enable V-Sync
	SDL_GL_SetSwapInterval(1);

	// --------------------------------------------------------------------------------------------------------
	// Initialize OpenGL
	// --------------------------------------------------------------------------------------------------------
	GLint viewWidth;
	GLint viewHeight;
	
	// Get size of the drawable area of the window
	SDL_GL_GetDrawableSize(window, &viewWidth, &viewHeight);

	// Set OpenGL viewport size
	glViewport(0, 0, viewWidth, viewHeight);

	// Enable z-buffer
	glEnable(GL_DEPTH_TEST);

	// --------------------------------------------------------------------------------------------------------
	// Shaders
	// --------------------------------------------------------------------------------------------------------
	// Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	checkShaderCompilationStatus(vertexShader);

	// Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	checkShaderCompilationStatus(fragmentShader);

	// Create shader program and link shaders
	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	checkShaderProgramLinkingStatus(shaderProgram);

	// Tidy up shaders as they won't be used anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// --------------------------------------------------------------------------------------------------------
	// Setup Buffers
	// --------------------------------------------------------------------------------------------------------
	
	// Generate a vertex array object (VAO)
	glGenVertexArrays(1, &VAO);
	// Generate a vertex buffer object(VBO)
	glGenBuffers(1, &VBO);
	// Generate an element buffer object(EBO)
	glGenBuffers(1, &EBO);

	// Bind VAO, then bind and set VBO and EBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Copy vertices into VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Copy indices into EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set vertex attribute pointers in VAO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Unbind for now, will use later in render loop
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// --------------------------------------------------------------------------------------------------------
	// Render loop
	// --------------------------------------------------------------------------------------------------------
	while (running) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float time = (float)SDL_GetTicks() / 1000.0;

		{
			float greenValue = sin(time) / 2.0f + 0.5f;
			int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
			glUniform4f(vertexColorLocation, 1.0f, greenValue, 0.0f, 1.0f);
		}

		{
			// Model matrix
			glm::mat4 model = glm::mat4(1.0f);
			// Rotate plane a bit about x axis based on time
			model = glm::rotate(model, (float)time * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

			int modelLoc = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		}

		// Drawing
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		SDL_GL_SwapWindow(window); // swap buffers

		// Handle events. As SDL_PollEvent removes the next event from the event queue and returns 0 if there
		// are no events on the stack, we use a while loop to check for appropriate events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						running = false;
						break;
					case SDLK_1:
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						break;
					case SDLK_2:
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						break;
					}
			}
			else if (event.type == SDL_QUIT) {
				running = false;
			}
		}
	}


	// --------------------------------------------------------------------------------------------------------
	// Tidy everything up
	// --------------------------------------------------------------------------------------------------------
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


void checkShaderCompilationStatus(unsigned int shader) {
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void checkShaderProgramLinkingStatus(unsigned int shaderProgram) {
	int success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
}
