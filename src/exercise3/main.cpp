#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Callback prototypes
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Mouse movement variables
bool firstMouse = true;
float lastX = 400, lastY = 300;  // Initial mouse position
float yaw = -90.0f, pitch = 0.0f; // Initial camera angles
float sensitivity = 0.05f; // Mouse sensitivity
bool isMousePressed = false;

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Shaders (vertex and fragment)
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fragColor = color;
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(fragColor, 1.0f);
}
)";

// Vertex data for the floating island, trees, and clouds (same as before but with colors)
GLfloat islandVertices[] = {
    -0.6f,  0.1f, 0.0f,  0.5f, 0.35f, 0.05f,  // Top left (brown)
     0.6f,  0.1f, 0.0f,  0.5f, 0.35f, 0.05f,  // Top right (brown)
     0.6f, -0.3f, 0.0f,  0.5f, 0.35f, 0.05f,  // Bottom right (brown)
    -0.6f, -0.3f, 0.0f,  0.5f, 0.35f, 0.05f   // Bottom left (brown)
};

GLuint islandIndices[] = {
    0, 1, 2, 2, 3, 0  // Main rectangle
};

// Tree trunk and leaves data (simple)
GLfloat treeVertices[] = {
    // Trunk 1
    -0.05f, 0.2f, 0.0f,  0.4f, 0.25f, 0.1f, // Bottom left of trunk
     0.05f, 0.2f, 0.0f,  0.4f, 0.25f, 0.1f, // Bottom right of trunk
     0.05f, 0.4f, 0.0f,  0.4f, 0.25f, 0.1f, // Top right of trunk
    -0.05f, 0.4f, 0.0f,  0.4f, 0.25f, 0.1f, // Top left of trunk

    // Leaves (simple triangular shape, centered above the trunk)
     0.0f, 0.2f, 0.0f,  0.0f, 0.4f, 0.2f,   // Top center
    -0.2f, 0.0f, 0.0f,  0.0f, 0.4f, 0.2f,   // Left
     0.2f, 0.0f, 0.0f,  0.0f, 0.4f, 0.2f    // Right
};

GLuint treeIndices[] = {
    // Trunk (two triangles)
    0, 1, 2,  0, 2, 3, 
    // Leaves (triangle)
    4, 5, 6
};


// Cloud vertices (4 clouds)
GLfloat cloudVertices[] = {
    // Cloud 1 (top-left)
    -0.8f,  0.8f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 1 (cloud 1)
    -0.6f,  0.8f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 2 (cloud 1)
    -0.7f,  0.9f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 3 (cloud 1)

    // Cloud 2 (top-right)
     0.7f,  0.8f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 1 (cloud 2)
     0.9f,  0.8f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 2 (cloud 2)
     0.8f,  0.9f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 3 (cloud 2)

    // Cloud 3 (upper center-left)
    -0.4f,  0.7f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 1 (cloud 3)
    -0.2f,  0.7f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 2 (cloud 3)
    -0.3f,  0.8f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 3 (cloud 3)

    // Cloud 4 (upper center-right)
     0.7f,  0.6f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 1 (cloud 4)
     0.5f,  0.6f, 0.0f,  1.0f, 1.0f, 1.0f, // Vertex 2 (cloud 4)
     0.6f,  0.7f, 0.0f,  1.0f, 1.0f, 1.0f  // Vertex 3 (cloud 4)
};

GLuint cloudIndices[] = {
    // Cloud 1
    0, 1, 2,  // Triangle 1 for Cloud 1
    
    // Cloud 2
    3, 4, 5,  // Triangle 1 for Cloud 2
    
    // Cloud 3
    6, 7, 8,  // Triangle 1 for Cloud 3
    
    // Cloud 4
    9, 10, 11  // Triangle 1 for Cloud 4
};

// VAO, VBO, EBO
unsigned int islandVAO, islandVBO, islandEBO;
unsigned int treeVAO, treeVBO, treeEBO;
unsigned int cloudVAO, cloudVBO, cloudEBO;

GLuint shaderProgram;

// Function to compile shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Error: " << infoLog << std::endl;
    }
    return shader;
}

// Function to link shaders into a program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Error: " << infoLog << std::endl;
    }
    return shaderProgram;
}

// Mouse callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!isMousePressed)
        return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed because y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Mouse button callback function
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        isMousePressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isMousePressed = false;
    }
}

// Function to initialize OpenGL objects (VAOs, VBOs, EBOs)
void initOpenGL() {
    // Initialize shaders
    shaderProgram = createShaderProgram();

    // Floating Island Setup
    glGenVertexArrays(1, &islandVAO);
    glGenBuffers(1, &islandVBO);
    glGenBuffers(1, &islandEBO);

    glBindVertexArray(islandVAO);
    glBindBuffer(GL_ARRAY_BUFFER, islandVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(islandVertices), islandVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, islandEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(islandIndices), islandIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Tree Setup
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glGenBuffers(1, &treeEBO);

    glBindVertexArray(treeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertices), treeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeIndices), treeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Cloud Setup
    glGenVertexArrays(1, &cloudVAO);
    glGenBuffers(1, &cloudVBO);
    glGenBuffers(1, &cloudEBO);

    glBindVertexArray(cloudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cloudVertices), cloudVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cloudIndices), cloudIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void renderIsland(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(islandVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Function to render trees
void renderTree(glm::mat4 view, glm::mat4 projection, glm::vec3 position) {
    glUseProgram(shaderProgram);
    glBindVertexArray(treeVAO);

    // Scaling factor for leaves
    glm::vec3 leavesScaleFactor(0.75f, 0.75f, 0.75f);

    // Render Leaves First
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);              // Translate to the tree position
    model = glm::translate(model, glm::vec3(0.0f, 0.3f, 0.0f)); // Move leaves above the trunk
    model = glm::scale(model, leavesScaleFactor);         // Scale down the leaves
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw leaves (second part of tree)
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint))); // Last 3 indices for the leaves

    // Render Trunk Second
    model = glm::mat4(1.0f); // Reset model matrix
    model = glm::translate(model, position);              // Translate to the tree position
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // Draw trunk (first part of tree)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // First 6 indices for the trunk
}


void renderClouds(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(cloudVAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Floating Island", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Initialize OpenGL objects
    initOpenGL();

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Light blue sky
        // Set the view and projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene
        renderIsland(view, projection);
        renderTree(view, projection, glm::vec3(-0.2f, -0.1f, 0.0f)); // First tree
        renderTree(view, projection, glm::vec3(0.4f, -0.1f, 0.0f));  // Second tree

        renderClouds(view, projection);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Function to process input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Callback for resizing window
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
