#include <stdlib.h>
#include <stdio.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define PEANUT_GB_12_COLOUR 0
#define PEANUT_GB_USE_NIBBLE_FOR_PALETTE 1
#include "../../peanut_gb.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Framebuffer. */
	uint8_t fb[LCD_HEIGHT][LCD_WIDTH];
};

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		       const uint8_t val)
{
	const struct priv_t * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
}


/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
static uint8_t *alloc_file(const char *file_name, size_t *file_len)
{
	FILE *rom_file = fopen(file_name, "rb");
	size_t rom_size;
	uint8_t *rom = NULL;

	if(rom_file == NULL)
		return NULL;

	fseek(rom_file, 0, SEEK_END);
	rom_size = ftell(rom_file);
	rewind(rom_file);
	rom = malloc(rom_size);

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
	{
		free(rom);
		fclose(rom_file);
		return NULL;
	}

	if(file_len != NULL)
		*file_len = rom_file;

	fclose(rom_file);
	return rom;
}

static uint8_t *read_cart_ram_file(const char *save_file_name, size_t len)
{
	uint8_t *ret;
	size_t read_size;

	ret = alloc_file(save_file_name, &read_size);

	if(len != read_size)
	{
		free(ret);
		return NULL;
	}
	
	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(ret == NULL)
		ret = calloc(len, 1);

	return ret;
}

void write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			 const size_t len)
{
	FILE *f;

	if(len == 0 || *dest == NULL)
		return;

	if((f = fopen(save_file_name, "wb")) == NULL)
	{
		return;
	}

	/* Record save file. */
	fwrite(f, *dest, sizeof(uint8_t), len);
	fclose(f);

	return;
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
	exit(EXIT_FAILURE);
}

#if ENABLE_LCD
/**
 * Draws scanline into framebuffer.
 */
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		   const uint_least8_t line)
{
	struct priv_t *priv = gb->direct.priv;
	memcpy(&priv->fb[line][0], pixels, 160);
}
#endif

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
	"in vec2 TexCoord0;\n"
	"out vec4 FragColor;\n"
	"uniform sampler2D ColorTable;\n"
	"uniform sampler2D MyIndexTexture;\n"
	"void main() {\n"
	"  float index = texture(MyIndexTexture, TexCoord0).r * 255.0;\n"
	"  vec2 paletteCoord = vec2(index / 255.0, 0.5);\n" // Assuming the palette is a 256x1 texture
	"  FragColor = texture(ColorTable, paletteCoord);\n"
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
	char *rom_file_name;
	struct gb_s gb;
	struct priv_t priv;

	if(argc != 2)
	{
		fprintf(stderr, "%s ROM\n", argv[0]);
		return -1;
	}
	else
	{
		rom_file_name = argv[1];
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = alloc_file(rom_file_name, NULL)) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Initialise context. */
	int ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &priv);
	if(ret != GB_INIT_NO_ERROR)
	{
		fprintf(stderr, "Peanut-GB failed to initialise: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	priv.cart_ram = malloc(gb_get_save_size(&gb));

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
	// gb.direct.interlace = 1;
#endif

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
	unsigned char palette[256 * 3] = { 0xFF }; // Replace with your palette data
#if 0
	for(unsigned i = 0; i < sizeof(palette);) {
		palette[i] = (i & 0xFF);
		i++;
		palette[i] = 255u - (i & 0xFF);
		i++;
		palette[i] = 128u - (i & 0xFF);
		i++;
	}
#else
	palette[0] = palette[1] = palette[2] = 0x00;

	palette[3] = 0xFF;
	palette[4] = 0x00;
	palette[5] = 0x00;

	palette[6] = 0x00;
	palette[7] = 0xFF;
	palette[8] = 0x00;

	palette[9]  = 0x00;
	palette[10] = 0x00;
	palette[11] = 0xFF;
#endif

	// Define pixel data (8-bit values)
	unsigned char pixels[LCD_WIDTH * LCD_HEIGHT]; // Replace with your pixel data
	for(unsigned i = 0; i < sizeof(pixels); i++) {
		pixels[i] = i & 0xFF;
		//pixels[i] = i & 0b11;
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
    -1.0f, -1.0f,  0.0f, 1.0f, // Bottom-left corner of the screen, top-left of the texture
     1.0f, -1.0f,  1.0f, 1.0f, // Bottom-right corner of the screen, top-right of the texture
    -1.0f,  1.0f,  0.0f, 0.0f, // Top-left corner of the screen, bottom-left of the texture
     1.0f,  1.0f,  1.0f, 0.0f  // Top-right corner of the screen, bottom-right of the texture
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

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		gb_run_frame(&gb);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, paletteTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, indexTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			LCD_WIDTH, LCD_HEIGHT, GL_RED, GL_UNSIGNED_BYTE,
			priv.fb);

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
