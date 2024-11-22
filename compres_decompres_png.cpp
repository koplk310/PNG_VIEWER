
#include "MyPNG.h"
#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
GLuint WIDTH = 1920, HEIGHT = 1200;

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position , 1.0);\n"
"ourColor = color;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"in vec3 ourColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(ourColor, 1.0f);\n"
"}\n\0";

// The MAIN function, from here we start the application and run the game loop
int main(int argc,char*argv[])
{   
    while (true) {
        try {

            std::string s;
            std::cout << "PNG File to open:\n";
            std::cin >> s;

            MyPNG png(s.c_str());
            WIDTH = png.getwidth()  ;
            HEIGHT = png.getheight() ;

            GLfloat dh = (2.0f / png.getheight());
            GLfloat dw = (2.0f / png.getwidth());
            RGB** pixels = png.getpixels();

            GLfloat* vertices = new GLfloat[png.getheight() * png.getwidth() * 6];

            GLfloat x = -1.0f;
            GLfloat y = 1.0f;

            int pos = 0;

            for (int i = 0; i < png.getheight(); ++i, y -= dh) {
                x = -1.0f;
                for (int j = 0; j < png.getwidth(); ++j, x += dw) {
                    vertices[pos++] = x;
                    vertices[pos++] = y;
                    vertices[pos++] = 0.0f;
                    vertices[pos++] = ((GLfloat)pixels[i][j].Red / 255.0);
                    vertices[pos++] = ((GLfloat)pixels[i][j].Green / 255.0);
                    vertices[pos++] = ((GLfloat)pixels[i][j].Blue / 255.0);
                }

            }



            // Init GLFW
            glfwInit();
            // Set all the required options for GLFW
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

            // Create a GLFWwindow object that we can use for GLFW's functions
            GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "PNG_VIEWER", nullptr, nullptr);
            glfwMakeContextCurrent(window);

            // Set the required callback functions
            glfwSetKeyCallback(window, key_callback);

            // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
            glewExperimental = GL_TRUE;
            // Initialize GLEW to setup the OpenGL Function pointers
            glewInit();

            // Define the viewport dimensions
            glViewport(0, 0, WIDTH, HEIGHT);


            // Build and compile our shader program
            // Vertex shader
            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
            glCompileShader(vertexShader);
            // Check for compile time errors
            GLint success;
            GLchar infoLog[512];
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
            // Fragment shader
            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
            glCompileShader(fragmentShader);
            // Check for compile time errors
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
            // Link shaders
            GLuint shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);
            // Check for linking errors
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);




            glEnable(GL_PROGRAM_POINT_SIZE);
            //glPointSize(std::max(dh * HEIGHT , dw * WIDTH ));
            glPointSize(1.5f);




            GLuint VBO, VAO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * pos, vertices, GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            // Color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0); // Unbind VAO






            // Game loop
            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();

                // Render
                // Clear the colorbuffer
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Draw the triangle
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glDrawArrays(GL_POINTS, 0, png.getheight() * png.getwidth());
                glBindVertexArray(0);

                // Swap the screen buffers
                glfwSwapBuffers(window);
            }
            // Properly de-allocate all resources once they've outlived their purpose
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            // Terminate GLFW, clearing any resources allocated by GLFW.
            glfwTerminate();
        }
        catch (...) {
            std::cout << "unexpected error try again";
        }
    }
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}