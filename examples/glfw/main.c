#include <stdlib.h>
#include <stdio.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define PEANUT_GB_HEADER_ONLY
#include "gb.h"

// Vertex shader
static const GLchar *vertexShaderSrc = "#version 440\n"
	"layout (location = 0) in vec4 vertexPosition;\n"
	"layout (location = 1) in vec2 vertexUV;\n"
	"out vec2 TexCoord0;\n"
	"void main() {\n"
	"  gl_Position = vertexPosition;\n"
	"  TexCoord0 = vertexUV;\n"
	"}\0";

// Fragment shader
static const GLchar *fragmentShaderSrc = "#version 440\n"
	"precision mediump float;\n"
	"in vec2 TexCoord0;\n"
	"out vec4 FragColor;\n"
	"uniform sampler1D ColorTable;\n"
	"uniform sampler2D MyIndexTexture;\n"
	"void main() {\n"
	"  lowp float index = texture(MyIndexTexture, TexCoord0).r;\n"
	"  FragColor = texture(ColorTable, index);\n"
	"}\0";

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar *message,
		const void *userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
}

GLuint compile_shader(GLenum type, const char *shaderSource)
{
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

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
	GLFWwindow *window;
	struct priv gb_ctx;

	if(argc != 2)
	{
		fprintf(stderr, "%s ROM\n", argv[0]);
		return -1;
	}

	{
		char *rom_file_name;
		rom_file_name = argv[1];
		if(gb_init_file(&gb_ctx, rom_file_name) != 0) {
			fprintf(stderr, "Error initalising GB context.\n");
			return -1;
		}
	}

	glfwSetErrorCallback(glfw_error_callback);

	// Initialize GLFW
	if (!glfwInit()) return -1;

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(LCD_WIDTH, LCD_HEIGHT, "OpenGL GLFW3 Example", NULL, NULL);
	if (!window) {
		fprintf(stderr, "Window could not be created.\n");
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// Make the window's context current
	glfwMakeContextCurrent(window);

	{
		int version = gladLoadGL(glfwGetProcAddress);
		printf("GL %d.%d\n",
		       GLAD_VERSION_MAJOR(version),
		       GLAD_VERSION_MINOR(version));
		printf("Ver: %s\n"
			"Shader: %s\n"
			"%s %s\n",
			glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION),
			glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	/* Enable Vsync. */
	glfwSwapInterval(1);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// Define palette (256 RGB values)
	unsigned char palette[256 * 3]; // Replace with your palette data
	palette[0] = palette[1] = palette[2] = 0xFF;
	palette[3] = 0xFF;
	palette[4] = 0x84;
	palette[5] = 0x84;
	palette[6] = 0x94;
	palette[7] = 0x3A;
	palette[8] = 0x3A;
	palette[9] = palette[10] = palette[11] = 0x00;

	palette[12] = palette[13] = palette[14] = 0xFF;
	palette[15] = 0x00;
	palette[16] = 0xFF;
	palette[17] = 0x00;
	palette[18] = 0x31;
	palette[19] = 0x84;
	palette[20] = 0x00;
	palette[21] = 0x00;
	palette[22] = 0x4A;
	palette[23] = 0x00;

	palette[24] = palette[25] = palette[26] = 0xFF;
	palette[27] = 0x63;
	palette[28] = 0xA5;
	palette[29] = 0xFF;
	palette[30] = 0x00;
	palette[31] = 0x00;
	palette[32] = 0xFF;
	palette[33] = palette[34] = palette[35] = 0x00;

	// Define pixel data (8-bit values)
	unsigned char pixels[LCD_WIDTH * LCD_HEIGHT]; // Replace with your pixel data
	for(unsigned i = 0; i < sizeof(pixels); i++) {
		pixels[i] = i & 0xFF;
		//pixels[i] = i & 0b11;
	}

	// Create texture
	GLuint paletteTexture;
	glGenTextures(1, &paletteTexture);
	glBindTexture(GL_TEXTURE_1D, paletteTexture);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLuint indexTexture;
	glGenTextures(1, &indexTexture);
	glBindTexture(GL_TEXTURE_2D, indexTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, LCD_WIDTH, LCD_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load and compile shaders
	GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexShaderSrc);
	GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

	// Create and link program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glUseProgram(program);

	// Define vertices for a fullscreen quad
	const GLfloat vertices[] = {
		// positions    // texture Coords (flipped on y-axis)
		-1.0f, -1.0f, 0.0f, 1.0f, // Bottom-left corner of the screen, top-left of the texture
		1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right corner of the screen, top-right of the texture
		-1.0f, 1.0f, 0.0f, 0.0f, // Top-left corner of the screen, bottom-left of the texture
		1.0f, 1.0f, 1.0f, 0.0f // Top-right corner of the screen, bottom-right of the texture
	};

	// Generate VAO and VBO with the respective buffers
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object
	glBindVertexArray(VAO);

	// Bind and set vertex buffer(s)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Configure vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Unbind VBO (the VAO will keep the VBOs bound)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind VAO
	glBindVertexArray(0);

	GLint ct_loc = glGetUniformLocation(program, "ColorTable");
	glUniform1i(ct_loc, 0);
	GLint it_loc = glGetUniformLocation(program, "MyIndexTexture");
	glUniform1i(it_loc, 1);
	
	if(it_loc == -1) {
		fprintf(stderr, "MyIndexTexture location not found.\n");
		glfwTerminate();
		return -1;
	}

	glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, paletteTexture);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		gb_run_frame(&gb_ctx.gb);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, indexTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			LCD_WIDTH, LCD_HEIGHT, GL_RED, GL_UNSIGNED_BYTE,
			gb_ctx.fb);

		// In your render loop, bind the VAO and draw the quad
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Draw 4 vertices as a triangle strip
		glBindVertexArray(0);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
