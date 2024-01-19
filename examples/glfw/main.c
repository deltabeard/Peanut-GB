#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

// Vertex shader
const GLchar* vertexShaderSrc = "#version 440\n"
	"attribute vec4 vertexPosition;\n"
	"attribute vec2 vertexUV;\n"
	"varying vec2 TexCoord0;\n"
	"void main() {\n"
	"  gl_Position = vertexPosition;\n"
	"  TexCoord0 = vertexUV;\n"
	"}\0";

// Fragment shader
const GLchar* fragmentShaderSrc = "#version 440\n"
	"varying vec2 TexCoord0;\n"
	"uniform sampler2D ColorTable;\n"
	"uniform sampler2D MyIndexTexture;\n"
	"void main() {\n"
	"  vec4 myindex = texture2D(MyIndexTexture, TexCoord0);\n"
	"  vec4 texel = texture2D(ColorTable, myindex.xy);\n"
	"  gl_FragColor = texel;\n"
	"}\0";

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                GLsizei length, const GLchar* message,const void* userParam)
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


int main(void)
{
	GLFWwindow *window;

	glfwSetErrorCallback(glfw_error_callback);

	// Initialize GLFW
	if (!glfwInit()) return -1;

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "OpenGL GLFW3 Example", NULL, NULL);
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
			"%s %s\n"
			//"Ext: %s\n"
			,
			glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION),
			glGetString(GL_VENDOR), glGetString(GL_RENDERER)
			//,glGetString(GL_EXTENSIONS)
			);
	}

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	/* Enable Vsync. */
	glfwSwapInterval(1);

	// Define palette (256 RGB values)
	unsigned char palette[256 * 3]; // Replace with your palette data
	palette[0] = 0x00;
	palette[1] = 0x00;
	palette[2] = 0x00;

	palette[3] = 0xFF;
	palette[4] = 0x00;
	palette[5] = 0x00;

	palette[6] = 0x00;
	palette[7] = 0x00;
	palette[8] = 0xFF;

	palette[9] = 0xFF;
	palette[10] = 0xFF;
	palette[11] = 0xFF;

	// Define pixel data (8-bit values)
	const int width = 640, height = 480;
	unsigned char pixels[width * height]; // Replace with your pixel data
	for(unsigned i = 0; i < sizeof(pixels); i++) {
		pixels[i] = i & 0b11;
	}

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

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
	// Positions   // Texture Coords
	-1.0f,  1.0f,  0.0f, 1.0f, // Top Left
	-1.0f, -1.0f,  0.0f, 0.0f, // Bottom Left
	1.0f, -1.0f,  1.0f, 0.0f, // Bottom Right
	1.0f,  1.0f,  1.0f, 1.0f  // Top Right
	};

	// Define indices for the quad
	const GLuint indices[] = {
		0, 1, 2, // First Triangle
		0, 2, 3  // Second Triangle
	};

	// Generate VAO and VBO with the respective buffers
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object
	glBindVertexArray(VAO);

	// Bind and set vertex buffer(s)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind and set index buffer(s)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Configure vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Unbind VBO (the VAO will keep the VBOs bound)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind VAO
	glBindVertexArray(0);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, paletteTexture);
		glUniform1i(glGetUniformLocation(program, "ColorTable"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, indexTexture);
		glUniform1i(glGetUniformLocation(program, "MyIndexTexture"), 1);

		// In your render loop, bind the VAO and draw the quad
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
