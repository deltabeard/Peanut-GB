
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if 0
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define MIN_OPENGL_MAJOR_VER 2
#define MIN_OPENGL_MINOR_VER 0

// Shader sources
const GLchar *vertexShaderSource = "#version 330\n"
	"layout (location = 0) in vec2 aPos;\n"
	"layout (location = 1) in vec2 texCoord;\n"
	"out vec2 TexCoord;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
	"	TexCoord = texCoord;\n"
	"}\0";

const GLchar *fragmentShaderSource = "#version 330\n"
	"in vec2 TexCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D bitmapTexture;\n"
	"uniform sampler1D paletteTexture;\n"
	"void main()\n"
	"{\n"
	"	float index = texture(bitmapTexture, TexCoord).r;\n"
	"	color = texture(paletteTexture, index);\n"
	"}\0";

static void error_callback(int error, const char *desc)
{
	fprintf(stderr, "Error %d: %s\n", error, desc);
}

static void key_callback(GLFWwindow* window, int key,
		int scancode, int action, int mode)
{
	if(action != GLFW_PRESS)
		return;

	if(key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Dummy function to simulate bitmap data updating
void updateBitmapData(unsigned char *bitmapData, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        bitmapData[i] = rand() % 256; // Random 8-bit value
    }
}

GLuint compileShader(const char* shaderSource, GLenum type) {
	// Create a shader object
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	// Check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
	}

	return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
	// Compile vertex and fragment shaders
	GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

	// Create shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	int success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}

	// Delete the shaders as they're linked into our program now and no longer needed
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int main(void)
{
	GLFWwindow *win;
	GLuint VBO, VAO;
	GLuint bitmapTex, paletteTex;
	unsigned char bitmapData[160][144] = { 0 };
	unsigned char palette[256][3];

	memset(&palette[0][0], 0x44, sizeof(palette));

	if(!glfwInit()) return EXIT_FAILURE;

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MIN_OPENGL_MAJOR_VER);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MIN_OPENGL_MINOR_VER);
	win = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	if(!win) return EXIT_FAILURE;

	glfwMakeContextCurrent(win);
	glfwSetKeyCallback(win, key_callback);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);

	{
		int version = gladLoadGL(glfwGetProcAddress);
		printf("GL %d.%d\n",
			GLAD_VERSION_MAJOR(version),
			GLAD_VERSION_MINOR(version));
	}

	glViewport(0, 0, 640, 480);

	/* Enable Vsync. */
	glfwSwapInterval(1);

	{
		const float vertices[] = {
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f,  0.5f, 0.0f
		};
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
			     GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
		glUseProgram(shaderProgram);
	}

#if 0
	// Generate and bind the bitmap texture
	glGenTextures(1, &bitmapTex);
	glBindTexture(GL_TEXTURE_2D, bitmapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 160, 144,
		0, GL_RED, GL_UNSIGNED_BYTE, bitmapData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	palette[1][0] = palette[1][1] = palette[1][2] = 0xFF;
	glGenTextures(1, &paletteTex);
	glBindTexture(GL_TEXTURE_1D, paletteTex);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256,
		0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

	while (!glfwWindowShouldClose(win))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
#if 0
		// Update bitmap data
		updateBitmapData(&bitmapData[0][0], 160, 144);


		// Update texture
		glBindTexture(GL_TEXTURE_2D, bitmapTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			160, 144,
			GL_RED, GL_UNSIGNED_BYTE, bitmapData);

		glUseProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bitmapTex);
		glUniform1i(glGetUniformLocation(shaderProgram, "bitmapTexture"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, paletteTex);
		glUniform1i(glGetUniformLocation(shaderProgram, "paletteTexture"), 1);

		// Rendering
		glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	//glDeleteProgram(shaderProgram);
	glfwDestroyWindow(win);
	glfwTerminate();
	return EXIT_SUCCESS;
}
#endif

#if 0
#include <glad/gles2.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

// Vertex shader
const char* vertex_shader_src = "attribute vec2 position; void main() { gl_Position = vec4(position, 0.0, 1.0); }";

// Fragment shader
const char* fragment_shader_src = "precision mediump float; uniform sampler2D texture; void main() { gl_FragColor = texture2D(texture, gl_PointCoord); }";

// Compile shader
GLuint compile_shader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	// Check for linking errors
	int success;
	char infoLog[512];
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}

	return shader;
}

int main(void) {
	GLFWwindow* window;

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	window = glfwCreateWindow(640, 480, "8-bit Palette Example", NULL, NULL);
	if (!window) {
		glfwTerminate();
		fprintf(stderr, "Failed to create GLFW window\n");
		return -1;
	}

	glfwMakeContextCurrent(window);

	{
		int version = gladLoadGLES2(glfwGetProcAddress);
		printf("GL %d.%d\n",
		       GLAD_VERSION_MAJOR(version),
		       GLAD_VERSION_MINOR(version));
	}

	/* Enable Vsync. */
	glfwSwapInterval(1);

	// Compile shaders
	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

	// Create program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	// Use program
	glUseProgram(program);

	// Define 8-bit palette and texture
	GLubyte palette[256 * 3]; // RGB for each color
	palette[0] = 0x00;
	palette[1] = 0xFF;
	palette[2] = 0x00;

	palette[3] = 0xFF;
	palette[4] = 0x00;
	palette[5] = 0x00;


	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Define vertex data
	GLfloat vertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set up vertex array
	GLint posAttrib = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
#endif

#include <glad/gles2.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

// Vertex shader
const GLchar* vertexShaderSrc =
	"attribute vec2 position;"
	"attribute vec2 texcoord;"
	"varying vec2 Texcoord;"
	"void main() {"
	"  Texcoord = texcoord;"
	"  gl_Position = vec4(position, 0.0, 1.0);"
	"}";

// Fragment shader
const GLchar* fragmentShaderSrc =
	"precision mediump float;"
	"varying vec2 Texcoord;"
	"uniform sampler2D paletteTexture;"
	"uniform sampler2D indexTexture;"
	"void main() {"
	"  float index = texture2D(indexTexture, Texcoord).r * 255.0;"
	"  vec2 paletteTexCoord = vec2((index + 0.5) / 256.0, 0.5);"
	"  gl_FragColor = texture2D(paletteTexture, paletteTexCoord);"
	"}";

int main(void) {
	GLFWwindow* window;

	// Initialize GLFW
	if (!glfwInit()) return -1;

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "OpenGL ES 2 GLFW3 Example", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	{
		int version = gladLoadGLES2(glfwGetProcAddress);
		printf("GL %d.%d\n",
		       GLAD_VERSION_MAJOR(version),
		       GLAD_VERSION_MINOR(version));
		printf("%s\n", glGetString(GL_VERSION));
	}

	/* Enable Vsync. */
	glfwSwapInterval(1);

	// Define palette (256 RGB values)
	unsigned char palette[256 * 3]; // Replace with your palette data
	palette[0] = 0x00;
	palette[1] = 0xFF;
	palette[2] = 0x00;

	palette[3] = 0xFF;
	palette[4] = 0x00;
	palette[5] = 0x00;

	// Define pixel data (8-bit values)
	const int width = 640, height = 480;
	unsigned char pixels[width * height]; // Replace with your pixel data
	memset(pixels, 1, sizeof(pixels));
	pixels[10] = 0;

	// Create texture
	GLuint paletteTexture;
	glGenTextures(1, &paletteTexture);
	glBindTexture(GL_TEXTURE_2D, paletteTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLuint indexTexture;
	glGenTextures(1, &indexTexture);
	glBindTexture(GL_TEXTURE_2D, indexTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);


	// Load and compile shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
	glCompileShader(fragmentShader);

	// Create and link program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glUseProgram(program);

	const GLfloat vertices[] = {
		// Positions  // TexCoords
		-1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
		1.0f, -1.0f,  1.0f, 0.0f, // Bottom-right
		1.0f,  1.0f,  1.0f, 1.0f, // Top-right
		-1.0f,  1.0f,  0.0f, 1.0f  // Top-left
	};


	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		// Update texture with pixel data
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, paletteTexture);
		glUniform1i(glGetUniformLocation(program, "paletteTexture"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, indexTexture);
		glUniform1i(glGetUniformLocation(program, "indexTexture"), 1);

		// Draw
		glDrawArrays(GL_POINTS, 0, width * height);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
