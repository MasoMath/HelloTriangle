/**
Gianluca Gisolo

Demo for C++, headers, and OpenGL
Creates a spinning rectangle on the bottom
right of a window that runs on the GPU.
Texture is a stretch out container.
9/10/22
*/

#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "./Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

constexpr unsigned int SCR_WIDTH{ 800 };
constexpr unsigned int SCR_HEIGHT{ 600 };

const char* vertexShaderPath
{
	"./vertexShader.glsl"
};
const char* fragmentShaderPath
{
	"./fragmentShader.glsl"
};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Initializes GLFW and creates canvas
GLFWwindow* initlializeGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creation of canvas
	GLFWwindow* window{ glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr) };
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	// loading GLAD and setting canvas as current context
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	return window;
}

// Main render loop
void renderLoop(GLFWwindow*& window, Shader& shader, unsigned int& VAO, unsigned int* textures)
{
	shader.use();
	shader.setInt("texture1", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glBindVertexArray(VAO);

	// matrix's uniform location
	unsigned int transformLoc = glGetUniformLocation(shader.getID(), "transform");
	glm::mat4 transform{ glm::mat4(1.0f) };
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		transform = glm::mat4(1.0f); // initialize matrix to identity matrix first
		transform = glm::translate(transform, glm::vec3{ 0.5f, -0.5f, 0.0f });
		transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3{ 0.0f, 0.0f, 1.0f });

		// set matrix before drawing the element
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}


// generate a texture from preexisting uint and pregiven location
void generateTexture(unsigned int& texture, const char* textureLoc, int textureNum = 0, bool hasAlpha = false)
{
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + textureNum);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// metadata
	int width, height, nrChannels;

	if (hasAlpha)
		stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(textureLoc, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (hasAlpha)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// deallocate memory used for the texture
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{
	GLFWwindow* window{ initlializeGLFW() };

	// creation of shaderProgram
	Shader shader{ vertexShaderPath, fragmentShaderPath };

	// Vertices and how to interpret them
	constexpr float vertices[]
	{
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
	};

	constexpr int indices[]
	{
		0, 1, 3,	// first triangle
		1, 2, 3		// second triangle
	};

	unsigned int texture1;
	generateTexture(texture1, "./notAbee.jpg");
	unsigned int textures[]{ texture1 };

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// =================== Vertex Attributes ======================
	// vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbinding VBO, VAO, and texture
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	renderLoop(window, shader, VAO, textures);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();

	return 0;
}