#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Vertex Shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 fragColor;

    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        fragColor = aColor;
    }
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;

    void main() {
        FragColor = vec4(fragColor, 1.0);
    }
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader Compilation Error: " << infoLog << std::endl;
    }

    return shader;
}

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "Shader Linking Error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Floating Island", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Shader program
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Vertex data for the scene
    float vertices[] = {
        // Sky (blue gradient rectangle)
        -1.0f,  1.0f, 0.5f, 0.8f, 1.0f,   // Top left
         1.0f,  1.0f, 0.5f, 0.8f, 1.0f,   // Top right
        -1.0f,  0.0f, 0.2f, 0.6f, 0.9f,   // Bottom left
         1.0f,  0.0f, 0.2f, 0.6f, 0.9f,   // Bottom right

        // Water (blue rectangle)
        -1.0f,  0.0f, 0.1f, 0.5f, 0.7f,   // Top left
         1.0f,  0.0f, 0.1f, 0.5f, 0.7f,   // Top right
        -1.0f, -1.0f, 0.0f, 0.3f, 0.5f,   // Bottom left
         1.0f, -1.0f, 0.0f, 0.3f, 0.5f,   // Bottom right

        // Island Base (brown triangular shape with sharp peaks)
        -0.8f, -0.4f, 0.5f, 0.3f, 0.1f,   // Far left
        0.8f, -0.4f, 0.5f, 0.3f, 0.1f,   // Far right
        -0.4f, -0.1f, 0.6f, 0.4f, 0.2f,   // Middle left
        0.4f, -0.1f, 0.6f, 0.4f, 0.2f,   // Middle right
        0.0f, -0.7f, 0.5f, 0.2f, 0.1f    // Sharp bottom peak

        // Grass (green polygon)
        -0.7f, -0.1f, 0.1f, 0.8f, 0.1f,   // Left edge
        0.7f, -0.1f, 0.1f, 0.8f, 0.1f,   // Right edge
        -0.6f,  0.3f, 0.2f, 0.9f, 0.2f,   // Left top
        0.6f,  0.3f, 0.2f, 0.9f, 0.2f    // Right top


         // Tree Trunk (brown rectangle)
        -0.05f,  0.1f, 0.5f, 0.3f, 0.1f,  // Bottom left
         0.05f,  0.1f, 0.5f, 0.3f, 0.1f,  // Bottom right
        -0.05f,  0.3f, 0.5f, 0.3f, 0.1f,  // Top left
         0.05f,  0.3f, 0.5f, 0.3f, 0.1f,  // Top right

        // Tree Foliage (green triangle)
        -0.15f,  0.3f, 0.1f, 0.8f, 0.1f,  // Bottom left
         0.15f,  0.3f, 0.1f, 0.8f, 0.1f,  // Bottom right
         0.0f,   0.5f, 0.1f, 0.9f, 0.1f   // Top
    };

    unsigned int indices[] = {
        // Sky
        0, 1, 2,
        1, 2, 3,

        // Water
        4, 5, 6,
        5, 6, 7,

        // Island Base
        0, 1, 4,
        0, 2, 4,
        1, 3, 4,

        // Grass
        5, 6, 7,
        6, 7, 8,

        // Tree Trunk
        16, 17, 18,
        17, 18, 19,

        // Tree Foliage
        20, 21, 22
    };

    // VAO, VBO, and EBO setup
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.5f, 0.8f, 0.95f, 1.0f);  // Background color (sky blue)
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
