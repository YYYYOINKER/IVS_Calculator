extern "C" {
#include <glad/glad.h>
}
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>

#include <thread>
#include <future>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

std::atomic<bool> cubemapReady = false;
std::atomic<bool> cubemapLoaded = false;
GLuint cubemapTexture = 0;

std::vector<unsigned char*> images(6);
std::vector<int> widths(6), heights(6), channels(6);


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

GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                    width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    return textureID;
}

// Vertices for cube placeholder and skybox cube
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

float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

// ========================================================================================
int main() {

    std::cout << "OpenGL Scene starting..." << std::endl;

    // failed to initialize
    if (!glfwInit()) {
        std::cerr << "GLFW init failed!" << std::endl;
        return -1;
    }

    // setup glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Calculator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set as main context 
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint shader = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint skyboxShader = createShaderProgram("shaders/skybox.vert", "shaders/skybox.frag");

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


    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces = {
        "textures/skybox/px.jpg",
        "textures/skybox/nx.jpg",
        "textures/skybox/py.jpg",
        "textures/skybox/ny.jpg",
        "textures/skybox/pz.jpg",
        "textures/skybox/nz.jpg"
    };

    // Spawn a thread to load the cubemap of skybox
    std::thread loaderThread([faces]() {
            for (int i = 0; i < 6; ++i) {
            images[i] = stbi_load(faces[i].c_str(), &widths[i], &heights[i], &channels[i], 0);
            if (!images[i]) {
            std::cerr << "Failed to load: " << faces[i] << std::endl;
            }
            }
            cubemapReady = true;
            });

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
        //
        // =================
        //       Input
        // =================

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

        // update viewport in case window was resized or WM is weird
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // =======================
        //        rendering
        // =======================
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

        // render skybox
        // Upload cubemap if loaded by thread
        static bool uploadedCubemap = false;
        if (cubemapReady && !uploadedCubemap) {
            glGenTextures(1, &cubemapTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

            for (int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                        widths[i], heights[i], 0, GL_RGB, GL_UNSIGNED_BYTE, images[i]);
                stbi_image_free(images[i]);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            uploadedCubemap = true;
            cubemapLoaded = true;
        }

        // render skybox
        glDepthFunc(GL_LEQUAL);
        if (cubemapLoaded) {
            glUseProgram(skyboxShader);
            glm::mat4 viewSky = glm::mat4(glm::mat3(view));
            glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(viewSky));
            glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glBindVertexArray(skyboxVAO);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        } else {
            // Just render a dummy placeholder cube or text while loading
            glUseProgram(shader);
            glm::mat4 loadingModel = glm::scale(model, glm::vec3(0.5f));
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(loadingModel));
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    loaderThread.join();

    glfwTerminate();
    return 0;
}


