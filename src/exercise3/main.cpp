#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>

// Callback prototypes
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

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

const char* motionBlurVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* motionBlurFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D currentFrame;
uniform sampler2D previousFrame;

void main() {
    vec4 currentColor = texture(currentFrame, TexCoords);
    vec4 previousColor = texture(previousFrame, TexCoords);
    FragColor = mix(currentColor, previousColor, 0.7); // Increase the blend factor for a stronger effect
}
)";

const char* colorCorrectionFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec3 colorAdjust; // RGB adjustment values

void main() {
    vec4 color = texture(screenTexture, TexCoords);
    color.rgb += colorAdjust; // Adjust colors
    FragColor = color;
}
)";

const char* vignettingFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec4 color = texture(screenTexture, TexCoords);
    float dist = distance(TexCoords, vec2(0.5, 0.5));
    color.rgb *= smoothstep(0.8, 0.5, dist); // Adjust these values for stronger/weaker vignetting
    FragColor = color;
}
)";

const char* filmGrainFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D noiseTexture;
uniform float grainAmount; // Amount of grain

void main() {
    vec4 color = texture(screenTexture, TexCoords);
    vec4 noise = texture(noiseTexture, TexCoords);
    color.rgb += noise.rgb * grainAmount; // Adjust grain amount
    FragColor = color;
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
unsigned int framebuffer1, framebuffer2;
unsigned int textureColorbuffer1, textureColorbuffer2;
unsigned int rbo1, rbo2;
unsigned int motionBlurShaderProgram, quadVAO, quadVBO;
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
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

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
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
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
void setupFramebuffers() {
    // Framebuffer 1
    glGenFramebuffers(1, &framebuffer1);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);

    glGenTextures(1, &textureColorbuffer1);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer1, 0);

    glGenRenderbuffers(1, &rbo1);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo1);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer 1 is not complete!" << std::endl;

    // Framebuffer 2
    glGenFramebuffers(1, &framebuffer2);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);

    glGenTextures(1, &textureColorbuffer2);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer2, 0);

    glGenRenderbuffers(1, &rbo2);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo2);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer 2 is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Function to initialize OpenGL objects (VAOs, VBOs, EBOs)
void initOpenGL() {
    // Initialize shaders
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    motionBlurShaderProgram = createShaderProgram(motionBlurVertexShaderSource, motionBlurFragmentShaderSource);

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

    // Framebuffer setup
    setupFramebuffers();

    // Quad setup for motion blur
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
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

GLuint createNoiseTexture(int width, int height) {
    std::vector<GLfloat> noiseData(width * height * 3);
    for (int i = 0; i < width * height * 3; ++i) {
        noiseData[i] = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
    }

    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, noiseData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return noiseTexture;
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

    // Setup framebuffers
    setupFramebuffers();

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize shader programs
    GLuint motionBlurShaderProgram = createShaderProgram(motionBlurVertexShaderSource, motionBlurFragmentShaderSource);
    GLuint colorCorrectionShaderProgram = createShaderProgram(vertexShaderSource, colorCorrectionFragmentShaderSource);
    GLuint vignettingShaderProgram = createShaderProgram(vertexShaderSource, vignettingFragmentShaderSource);
    GLuint filmGrainShaderProgram = createShaderProgram(vertexShaderSource, filmGrainFragmentShaderSource);

    // Create noise texture
    GLuint noiseTexture = createNoiseTexture(128, 128); // Adjust the size as needed

    // Main rendering loop
    static bool showMotionBlur = true;
    static bool showColorCorrection = false;
    static bool showVignetting = false;
    static bool showFilmGrain = false;
    static float colorAdjust[3] = {0.0f, 0.0f, 0.0f};
    static float grainAmount = 0.1f;
    static float fps = 0.0f;
    static float lastTime = glfwGetTime();
    static int frameCount = 0;
    bool useFramebuffer1 = true;

    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);

        // Calculate FPS
        frameCount++;
        float currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0f) {
            fps = frameCount / (currentTime - lastTime);
            frameCount = 0;
            lastTime = currentTime;
        }

        // Render to framebuffer
        if (useFramebuffer1) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
        }
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Light blue sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the view and projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Render scene
        renderIsland(view, projection);
        renderTree(view, projection, glm::vec3(-0.2f, -0.1f, 0.0f)); // First tree
        renderTree(view, projection, glm::vec3(0.4f, -0.1f, 0.0f));  // Second tree
        renderClouds(view, projection);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Apply post-processing effects
        if (showMotionBlur) {
            glUseProgram(motionBlurShaderProgram);
            glBindVertexArray(quadVAO);
            glDisable(GL_DEPTH_TEST);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, useFramebuffer1 ? textureColorbuffer1 : textureColorbuffer2);
            glUniform1i(glGetUniformLocation(motionBlurShaderProgram, "currentFrame"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, useFramebuffer1 ? textureColorbuffer2 : textureColorbuffer1);
            glUniform1i(glGetUniformLocation(motionBlurShaderProgram, "previousFrame"), 1);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }

        if (showColorCorrection) {
            glUseProgram(colorCorrectionShaderProgram);
            glBindVertexArray(quadVAO);
            glDisable(GL_DEPTH_TEST);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, useFramebuffer1 ? textureColorbuffer1 : textureColorbuffer2);
            glUniform1i(glGetUniformLocation(colorCorrectionShaderProgram, "screenTexture"), 0);
            glUniform3fv(glGetUniformLocation(colorCorrectionShaderProgram, "colorAdjust"), 1, colorAdjust);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }

        if (showVignetting) {
            glUseProgram(vignettingShaderProgram);
            glBindVertexArray(quadVAO);
            glDisable(GL_DEPTH_TEST);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, useFramebuffer1 ? textureColorbuffer1 : textureColorbuffer2);
            glUniform1i(glGetUniformLocation(vignettingShaderProgram, "screenTexture"), 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }

        if (showFilmGrain) {
            glUseProgram(filmGrainShaderProgram);
            glBindVertexArray(quadVAO);
            glDisable(GL_DEPTH_TEST);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, useFramebuffer1 ? textureColorbuffer1 : textureColorbuffer2);
            glUniform1i(glGetUniformLocation(filmGrainShaderProgram, "screenTexture"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, noiseTexture); // Assuming noiseTexture is already created
            glUniform1i(glGetUniformLocation(filmGrainShaderProgram, "noiseTexture"), 1);
            glUniform1f(glGetUniformLocation(filmGrainShaderProgram, "grainAmount"), grainAmount);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create ImGui window
        ImGui::Begin("Settings");
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Checkbox("Motion Blur", &showMotionBlur);
        ImGui::Checkbox("Color Correction", &showColorCorrection);
        ImGui::SliderFloat3("Color Adjust", colorAdjust, -1.0f, 1.0f);
        ImGui::Checkbox("Vignetting", &showVignetting);
        ImGui::Checkbox("Film Grain", &showFilmGrain);
        ImGui::SliderFloat("Grain Amount", &grainAmount, 0.0f, 1.0f);
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Swap framebuffers
        useFramebuffer1 = !useFramebuffer1;
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Callback for resizing window
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}