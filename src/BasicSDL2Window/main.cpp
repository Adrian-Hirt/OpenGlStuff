#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <SDL.h>
#undef main // "Hack" because else we get an error using SDL
#include <glad/glad.h>


void checkShaderCompilationStatus(unsigned int shader);
void checkShaderProgramLinkingStatus(unsigned int shaderProgram);
// --------------------------------------------------------------------------------------------------------
// Shader sources
// --------------------------------------------------------------------------------------------------------
const char* vertexShaderSource = R"(
	#version 330 core
	
	layout (location = 0) in vec3 position;

	void main() {
		gl_Position = vec4(position, 1.0);
	}
)";

const char* fragmentShaderSource = R"(
	#version 330 core

	out vec4 color;

	void main() {
		color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	}
)";

// --------------------------------------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------------------------------------
SDL_Window* window = NULL;
SDL_GLContext glContext;

// Window size
unsigned int windowHeight = 600;
unsigned int windowWidth = 800;

// Initial window position
unsigned int initialX = 200;
unsigned int initialY = 200;

// Window title
char windowTitle[] = "BasicSDL2Window";

// Buffer objects
GLuint VAO;
GLuint VBO;

// Is the render loop still running
bool running = true;

// SDL event structure
SDL_Event event;


// --------------------------------------------------------------------------------------------------------
// Data to draw
// --------------------------------------------------------------------------------------------------------
float vertices[] = {
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f

};
GLuint vertexCount = 6;

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

	// Bind VAO, then bind and set VBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Copy vertices into VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

		// Drawing
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
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
