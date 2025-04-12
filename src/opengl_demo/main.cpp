extern "C" {
#include <glad/glad.h>
}
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

// Shader loading utility
std::string loadShaderSource(const char* path) {

    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Compile and link shader program
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {

    std::string vCode = loadShaderSource(vertexPath);
    std::string fCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}


float vertices[] = {

    // positions          // normals
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

    0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
    0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
    0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
    0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  0.0f,
    0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  0.0f
};


int main() {

    std::cout << "OpenGL Scene starting..." << std::endl;

    if (!glfwInit()) {
        std::cerr << "GLFW init failed!" << std::endl;
        return -1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Calculator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint shader = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    GLuint vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float) , (void*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // setup camera variables
    float radius = 5.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float target_yaw = 0.0f;
    float target_pitch = 0.0f;

    double last_x = 0.0; double last_y = 0.0;
    bool dragging = false;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //float last_time = glfwGetTime();

    // MAIN LOOP
    // ==========

    while (!glfwWindowShouldClose(window)) {

        //float current_time = glfwGetTime();
        //float delta_time = current_time - last_time;
        //last_time = current_time;

        // Input

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        
            if (!dragging) {
                glfwGetCursorPos(window, &last_x, &last_y);
                dragging = true;
            }else {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                
                double dx = xpos - last_x;
                double dy = ypos - last_y;

                // follow the mouse kinda rotation
                target_yaw -= dx * 0.3f;
                target_pitch += dy * 0.3f;

                // clamp

                if (target_yaw > 89.0f) target_yaw = 89.0f;
                if (target_pitch < -89.0f) target_pitch = -89.0f;

                last_x = xpos;
                last_y = ypos;
            }
        }else {
            dragging = false;
            target_yaw *= 0.75f;
            target_pitch *= 0.75f;
        }
        yaw += (target_yaw - yaw) * 0.1f;
        pitch += (target_pitch - pitch) * 0.1f;


        // camera position
        glm::vec3 direction;
        direction.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));

        glm::vec3 camera_pos = direction * radius;

        glm::mat4 view = glm::lookAt(camera_pos , glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        // rendering

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // lighting
    
        glUniform3f(glGetUniformLocation(shader, "lightDir"), -0.5f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.7f, 1.0f);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
}







