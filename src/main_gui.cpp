/*
 * @mainpage 3DCalculator
 * @project 3DCalculator
 * @author Pavol Mihalik (xmihalp01 VUT FIT)
 * @file main.cpp
 * @date 2025-04-03
 * @brief core opengl rendering logic and event handling for 3d scene
 *
 * this file contains the main rendering pipeline implementation,
 * including context setup, shader loading, draw calls, and frame updates.
 * it initializes the rendering context, loads models and textures,
 * handles user input, and updates the scene.
 * 
 * dependencies:
 * - glad: loads opengl functions
 * - glfw: handles window creation and input
 * - glm: provides matrix math for 3d transforms
 * - stb_image: loads textures into memory
 * - tinyobjloader: imports .obj model geometry
 * - textRenderer: renders 2d on-screen text (optional)
 *
 */


/**
 * @brief opengl function loader (glad)
 *
 * loads all modern opengl functions dynamically at runtime.
 */
#include <glad/glad.h>

/**
 * @brief window and input handling (glfw)
 *
 * handles window creation, opengl context setup, and keyboard/mouse input.
 */
#include <GLFW/glfw3.h>

/**
 * @brief math library (glm)
 *
 * used for 3d vectors, matrices, and transformations.
 */
#include <glm/glm.hpp>                        // base types like vec3, mat4
#include <glm/gtc/matrix_transform.hpp>       // functions like translate, rotate, scale
#include <glm/gtc/type_ptr.hpp>               // convert glm types to float pointers

/**
 * @brief standard c++ libraries
 *
 * used for i/o, containers, math, strings, threading, etc.
 */
#include <iostream>       // prints to terminal
#include <fstream>        // reads shader files
#include <vector>         // dynamic arrays
#include <sstream>        // stringstream for reading text
#include <string>         // string manipulation
#include <cmath>          // math functions like sin, cos, pow
#include <thread>         // runs texture loader in background
#include <future>         // optional for async behavior
#include <unordered_set>  // stores allowed input values for filtering
#include <stack>          // Needed for dummy calculate
#include <stdexcept>      // For throw runtime_error

/**
 * @brief image loader (stb_image)
 *
 * loads .jpg/.png images into memory for use as textures.
 */
#define STB_IMAGE_IMPLEMENTATION              // enables stb image function definitions
#include "stb/stb_image.h"                    // single-header image library

/**
 * @brief obj file model loader (tinyobjloader)
 *
 * reads .obj mesh files and loads vertices, textures, and materials.
 */
#include "tiny_obj_loader.h"                  // loads 3d models into mesh structs

/**
 * @brief 2d text rendering module (optional)
 *
 * renders on-screen text using freetype and opengl.
 */
#include "TextRenderer.h"                   // disabled for now, can be enabled for HUD text

// camera distance variables
// radius controls current zoom, target_radius smooths the zoom animation
static float radius = 5.0f;
float target_radius = 5.0f;
//double calculate(const std::string& expr);


// stores the current input string from user (e.g. "6^2+3")
//std::string current_input;
double stored_value = 0; // value we already have (left side)
std::string current_input; // what the user is currently typing (string)
std::string pending_operator; // "+", "-", "*", or "/"
bool just_evaluated = false; // did we just finish "="?
std::string full_expression; // the full typed equation
std::string current_value;   // the evaluated result


/**
 * @brief calculator input state for root logic
 *
 * used to track multi-step input for custom root operations.
 */
enum input_state {
    NORMAL,                          // default state, handles regular input
    AWAITING_ROOT_DEGREE_FROM_NUMBER, // waiting for root degree (e.g. 2 in root(2,9))
    AWAITING_ROOT_DEGREE_BEFORE_VALUE // waiting for value after root degree
};

// current active input state
input_state input_state = NORMAL;

// stores one part of root expression temporarily
std::string root_value;

/**
 * @struct SubMesh
 * @brief Represents a single drawable part of a 3D mesh with a material.
 *
 * Contains the geometry (vertices and indices) and OpenGL buffer handles.
 */
struct SubMesh {
    GLuint vao; //!< Vertex Array Object ID.
    GLuint vbo; //!< Vertex Buffer Object ID.
    GLuint ebo; //!< Element Buffer Object ID.
    std::vector<float> vertices; //!< Flattened list of vertex attributes (position, texcoord, normal).
    std::vector<unsigned int> indices; //!< Index buffer for drawing.
    int material_ID; //!< Index of the material associated with this submesh.
};


/**
 * @brief full mesh data
 *
 * contains multiple submeshes and related material info.
 */
struct Mesh {
    std::vector<SubMesh> submeshes;              // collection of mesh parts
    std::vector<tinyobj::material_t> materials;  // material info loaded from obj file
};

/**
 * @brief calculator button definition
 *
 * defines position, size and label of a button in 3d space.
 */
struct Button {
    glm::vec3 position;   // center of the button in world space
    float size = 0.1f;    // size used for hit detection (radius-ish)
    std::string label;    // input value this button represents
};

// global list of buttons in scene
std::vector<Button> buttons;

/**
 * @brief global flags for cubemap loading
 *
 * used by loading thread to signal main thread when cubemap is ready to upload.
 */
std::atomic<bool> cubemap_ready = false;  // true when image data is loaded in memory
std::atomic<bool> cubemap_loaded = false; // true after OpenGL texture is created
GLuint cubemap_texture = 0;               // OpenGL texture id for skybox cubemap
bool show_loading = true;
std::atomic<int> cubemap_loaded_faces = 0;

/**
 * @brief global storage for cubemap textures
 *
 * stores raw image data and info for each face of the cubemap.
 * faces are in this order: right, left, top, bottom, front, back.
 */
std::vector<unsigned char*> images(6);      // raw pixel data for 6 images
std::vector<int> widths(6), heights(6);     // image sizes per face
std::vector<int> channels(6);               // number of channels (e.g. RGB/RGBA)

/**
 * @brief loads shader source from file
 *
 * opens a file, reads its contents and returns as a single string.
 *
 * @param path path to the shader source file
 * @return std::string content of the shader file
 */
std::string load_shader_source(const char* path) {
    std::ifstream file(path);        // open file for reading
    std::stringstream ss;            // stringstream to collect lines
    ss << file.rdbuf();              // read entire file into the stream
    return ss.str();                 // return combined string
}

/**
 * @brief compiles and links vertex + fragment shaders
 *
 * loads shader source from files, compiles them,
 * links them into a shader program and returns the program ID.
 *
 * @param vertex_path path to vertex shader source
 * @param fragment_path path to fragment shader source
 * @return GLuint compiled and linked OpenGL shader program
 */
GLuint createShaderProgram(const char* vertex_path, const char* fragment_path) {

    // load source code of vertex and fragment shaders
    std::string v_code = load_shader_source(vertex_path);  // read vertex shader code from file
    std::string f_code = load_shader_source(fragment_path); // read fragment shader code from file
    const char* v_shader_code = v_code.c_str();           // convert to C-string for OpenGL
    const char* f_shader_code = f_code.c_str();

    // create vertex shader and compile it
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);      // create vertex shader object
    glShaderSource(vertex_shader, 1, &v_shader_code, nullptr);      // attach source
    glCompileShader(vertex_shader);                               // compile shader

    // create fragment shader and compile it
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);  // create fragment shader object
    glShaderSource(fragment_shader, 1, &f_shader_code, nullptr);    // attach source
    glCompileShader(fragment_shader);                             // compile shader

    // link shaders into a single shader program
    GLuint shader_program = glCreateProgram();                    // create shader program
    glAttachShader(shader_program, vertex_shader);                 // attach vertex shader
    glAttachShader(shader_program, fragment_shader);               // attach fragment shader
    glLinkProgram(shader_program);                                // link program

    // cleanup shaders after linking
    glDeleteShader(vertex_shader);  // no longer needed
    glDeleteShader(fragment_shader);

    return shader_program;  // return the program ID
}

/**
 * @brief loads a .obj model from disk and prepares it for rendering
 *
 * loads geometry and materials, generates opengl buffers (vao/vbo/ebo),
 * and loads any textures found in the materials.
 *
 * @param obj_path path to the .obj model file
 * @param base_path base folder for textures/materials
 * @return Mesh a mesh object containing geometry, buffers and textures
 */
Mesh load_obj_model(const std::string& obj_path, const std::string& base_path) {
    tinyobj::attrib_t attrib;                       // stores all vertex data
    std::vector<tinyobj::shape_t> shapes;           // stores individual mesh parts
    std::vector<tinyobj::material_t> materials;     // stores materials and texture info
    std::string warn, err;                          // capture warnings and errors

    // load the obj model
    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path.c_str(), base_path.c_str(), true);
    if (!ok) throw std::runtime_error("Failed to load OBJ: " + err);  // crash if load failed

    // store OpenGL texture ids for each material
    std::vector<GLuint> texture_IDs(materials.size());
    for (size_t i = 0; i < materials.size(); ++i) {
        std::string tex_path = base_path + "/" + materials[i].diffuse_texname;  // full path to texture
        int w, h, ch;
        unsigned char* data = stbi_load(tex_path.c_str(), &w, &h, &ch, 0);      // load image
        if (data) {
            glGenTextures(1, &texture_IDs[i]);                          // create texture
            glBindTexture(GL_TEXTURE_2D, texture_IDs[i]);               // bind it
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,           // send image to GPU
                    (ch == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // smooth filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);                                     // free image data
        } else {
            std::cerr << "Failed to load: " << tex_path << std::endl;   // log failure
            texture_IDs[i] = 0;                                         // set to 0 if failed
        }
    }

    Mesh model;  // main mesh we return

    // go over each shape (sub-mesh)
    for (const auto& shape : shapes) {
        SubMesh sub;  // temporary submesh
        sub.material_ID = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];  // which material to use

        // go over all indices in this shape
        for (const auto& idx : shape.mesh.indices) {
            float vx = attrib.vertices[3 * idx.vertex_index + 0];  // x
            float vy = attrib.vertices[3 * idx.vertex_index + 1];  // y
            float vz = attrib.vertices[3 * idx.vertex_index + 2];  // z

            float tx = 0.0f, ty = 0.0f;
            if (!attrib.texcoords.empty() && idx.texcoord_index >= 0) {
                tx = attrib.texcoords[2 * idx.texcoord_index + 0];         // u
                ty = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]; // v (flipped)
            }

            float nx = 0.0f, ny = 0.0f, nz = 0.0f;
            if (!attrib.normals.empty() && idx.normal_index >= 0) {
                nx = attrib.normals[3 * idx.normal_index + 0];  // normal x
                ny = attrib.normals[3 * idx.normal_index + 1];  // normal y
                nz = attrib.normals[3 * idx.normal_index + 2];  // normal z
            }

            // push full vertex into buffer
            sub.vertices.insert(sub.vertices.end(), { vx, vy, vz, tx, ty, nx, ny, nz });

            // push index
            sub.indices.push_back(sub.indices.size());
        }

        // generate OpenGL buffers for this submesh
        glGenVertexArrays(1, &sub.vao);                       // create vertex array
        glGenBuffers(1, &sub.vbo);                            // create vertex buffer
        glGenBuffers(1, &sub.ebo);                            // create element/index buffer

        glBindVertexArray(sub.vao);                           // bind array
        glBindBuffer(GL_ARRAY_BUFFER, sub.vbo);               // bind vertex buffer
        glBufferData(GL_ARRAY_BUFFER,                        // send vertex data to GPU
                sub.vertices.size() * sizeof(float),
                sub.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub.ebo);       // bind index buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,                // send index data to GPU
                sub.indices.size() * sizeof(unsigned int),
                sub.indices.data(), GL_STATIC_DRAW);

        // enable vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);             // pos
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // texcoord
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // normal
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);                                 // unbind
        model.submeshes.push_back(sub);                       // add to model
    }

    // store texture ID inside material structure (uses unused 'illum' field)
    for (size_t i = 0; i < texture_IDs.size(); ++i) {
        materials[i].illum = texture_IDs[i];  // reuse field to store texture handle
    }
    model.materials = materials;  // assign materials

    return model;  // return the complete mesh
}


/**
 * @brief Very basic calculator parser.
 *
 * Evaluates a simple math expression (only +, -, *, /, no parentheses).
 * Warning: Division by zero throws runtime_error. 
 *
 * @param expr String containing math expression (e.g., "3+5*2").
 * @return Evaluated result as double.
 */
double calculate(const std::string& expr) {
    // ⚠ WARNING: Only handles simple cases, no parentheses
    if (expr.empty())
        return 0.0;

    double result = 0.0;
    double current = 0.0;
    char operation = '+';

    for (size_t i = 0; i <= expr.size(); ++i) {
        if (i < expr.size() && (isdigit(expr[i]) || expr[i] == '.')) {
            current = current * 10 + (expr[i] - '0');
        } else {
            if (operation == '+') result += current;
            else if (operation == '-') result -= current;
            else if (operation == '*') result *= current;
            else if (operation == '/') {
                if (current == 0) throw std::runtime_error("Divide by zero");
                result /= current;
            }
            if (i < expr.size())
                operation = expr[i];
            current = 0;
        }
    }

    return result;
}

/**
 * @brief loads a cubemap from 6 image files into opengl texture
 *
 * takes file paths to the 6 faces of a cubemap, loads each with stb_image,
 * and creates an opengl cubemap texture usable in shaders.
 *
 * @param faces vector of 6 file paths, ordered as: right, left, top, bottom, front, back
 * @return textureID opengl handle to the cubemap texture
 */
GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);                     // generate a new texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);    // bind as cubemap

    int width, height, nrChannels;

    // go over each face of the cubemap
    for (unsigned int i = 0; i < faces.size(); i++) {
        // load image from disk
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            // upload image to correct face of the cubemap
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                    width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data); // free memory after upload
        } else {
            // if failed, report and still free
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    return textureID; // return texture handle
}


/**
 * @brief SkyboxVertices data.
 *
 * Array containing vertex information for the cubemap.
 */
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

/**
 * @brief handles mouse scroll to zoom camera
 *
 * scroll up = zoom in, scroll down = zoom out
 * clamps zoom range between 2.0 and 30.0
 *
 * @param window pointer to glfw window
 * @param xoffset scroll on x (unused)
 * @param yoffset scroll on y (used for zoom)
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    target_radius -= yoffset * 0.5f; // zoom by changing target camera radius

    // clamp to min and max zoom
    if (target_radius < 2.0f) target_radius = 2.0f;
    if (target_radius > 30.0f) target_radius = 30.0f;
}


/**
 * @brief Processes a single calculator input (from mouse or keyboard).
 *
 * Handles regular digits, operations (+, -, *, /), constants (pi, e),
 * and special functions (sqrt, a^n, factorial, clear).
 * Manages the calculator state machine to properly update expressions and results.
 *
 * @param inputLabel The button label or character pressed (e.g., "7", "+", "pi").
 */
void process_input(const std::string& inputLabel) {

    static double stored_value = 0;
    static std::string current_input = "";
    static std::string pending_operator = "";
    static bool just_evaluated = false;

    static const std::unordered_set<std::string> validInputs = {
        "0","1","2","3","4","5","6","7","8","9",
        "+","-","*","/","%","=",".","pi","e","sqrt","a^n","!","C","CE","^"
    };

    if (validInputs.count(inputLabel) == 0) {
        std::cerr << "Ignored invalid input: " << inputLabel << std::endl;
        return;
    }

    if (inputLabel == "C") { // Full reset
        stored_value = 0;
        current_input.clear();
        pending_operator.clear();
        full_expression = "0";  // Set to 0, not empty
        current_value = "0";    // Set to 0, not empty
        just_evaluated = false;
        return;
    }

    if (inputLabel == "CE") {
        if (current_value.empty()) current_value = "0"; // just to be extra safe
        full_expression = current_value; // Keep current answer
        current_input.clear();
        pending_operator.clear();
        just_evaluated = true;
        return;
    }

    if (inputLabel == "pi") {
        if (just_evaluated) {
            stored_value = 0;
            pending_operator.clear();
            current_input.clear();
            full_expression.clear();
            current_value.clear();
            just_evaluated = false;
        }
        if (full_expression == "0") {
            full_expression.clear(); // 🛠️ fix for "0pi" bug
        }
        current_input += "pi"; 
        full_expression += "pi";
        current_value = current_input;
        return;
    }

    if (inputLabel == "e") {
        if (just_evaluated) {
            stored_value = 0;
            pending_operator.clear();
            current_input.clear();
            full_expression.clear();
            current_value.clear();
            just_evaluated = false;
        }
        if (full_expression == "0") {
            full_expression.clear();  // ← fix for "0e" bug
        }
        current_input += "e"; 
        full_expression += "e";
        current_value = current_input;
        return;
    }



    // Allow negative number at start or after operator
    // Allow starting a negative number at the beginning or after an operator
    if (inputLabel == "-") {
        if (current_input.empty()) {
            if (full_expression.empty() || full_expression == "0" || !pending_operator.empty()) {
                // Starting a new negative number
                if (full_expression == "0") {
                    full_expression.clear(); // Remove dummy zero
                }
                current_input += "-";
                full_expression += "-";
                current_value = current_input;
                just_evaluated = false;
                return;
            }
        }
    }




    if (inputLabel == "=") {
        if (!pending_operator.empty() && !current_input.empty()) {
            try {

                std::string evaluated_input = current_input;
                std::string real_expression;
                for (size_t i = 0; i < evaluated_input.size(); ++i) {
                    if (evaluated_input.substr(i,2) == "pi") {
                        real_expression += "3.1415926535";
                        i += 1; // skip extra character because "pi" is two letters
                    }
                    else if (evaluated_input[i] == 'e') {
                        real_expression += "2.7182818284";
                    }
                    else {
                        real_expression += evaluated_input[i];
                    }
                }
                double right_opperand = std::stod(real_expression);
                if (pending_operator == "+") stored_value += right_opperand;
                else if (pending_operator == "-") stored_value -= right_opperand;
                else if (pending_operator == "*") stored_value *= right_opperand;
                else if (pending_operator == "/") {
                    if (right_opperand == 0) {
                        current_value = "ERR";
                        return;
                    }
                    stored_value /= right_opperand;
                }
            } catch (const std::invalid_argument& e) {
                // if user pressed weird junk, safely ignore
                current_value = "ERR";
                return;
            }
            pending_operator.clear();
            current_input.clear();

            // Force -0 to 0
            if (std::abs(stored_value) < 1e-8) stored_value = 0.0;
            current_value = std::to_string(stored_value);


            current_value = std::to_string(stored_value);
            if (current_value.find('.') != std::string::npos) {
                current_value.erase(current_value.find_last_not_of('0') + 1, std::string::npos);
                if (current_value.back() == '.') {
                    current_value.pop_back();
                }
            }

            full_expression = current_value; // 🛠️ final result replaces history
            just_evaluated = true;
        }
        return;
    }

    // Handle operator input
    if (inputLabel == "+" || inputLabel == "-" || inputLabel == "*" || inputLabel == "/") {
        if (!current_input.empty()) {
            try {
                std::string evaluated_input = current_input;
                std::string real_expression;
                for (size_t i = 0; i < evaluated_input.size(); ++i) {
                    if (evaluated_input.substr(i,2) == "pi") {
                        real_expression += "3.1415926535";
                        i += 1; // skip the next character of "pi"
                    }
                    else if (evaluated_input[i] == 'e') {
                        real_expression += "2.7182818284";
                    }
                    else {
                        real_expression += evaluated_input[i];
                    }
                }
                double right_opperand = std::stod(real_expression);

                if (pending_operator.empty()) {
                    stored_value = right_opperand;
                } else {
                    if (pending_operator == "+") stored_value += right_opperand;
                    else if (pending_operator == "-") stored_value -= right_opperand;
                    else if (pending_operator == "*") stored_value *= right_opperand;
                    else if (pending_operator == "/") {
                        if (right_opperand == 0) {
                            current_value = "ERR";
                            return;
                        }
                        stored_value /= right_opperand;
                    }
                }
                current_value = std::to_string(stored_value);
                if (current_value.find('.') != std::string::npos) {
                    current_value.erase(current_value.find_last_not_of('0') + 1, std::string::npos);
                    if (current_value.back() == '.') {
                        current_value.pop_back();
                    }
                }
            } catch (const std::invalid_argument& e) {
                // safe ignore
            }
            current_input.clear();
        }

        // ignore if last char was operator already (block ++++)
        if (!full_expression.empty()) {
            char last = full_expression.back();
            if (last == '+' || last == '-' || last == '*' || last == '/')
                return;
        }

        pending_operator = inputLabel;
        full_expression += inputLabel;
        just_evaluated = false;
        return;
    }

    // Handle number input (normal numbers and dot)
    if (std::isdigit(inputLabel[0]) || inputLabel == ".") {
        if (just_evaluated) {
            stored_value = 0;
            pending_operator.clear();
            current_input.clear();
            full_expression.clear();
            current_value.clear();
            just_evaluated = false;
        }

        // 🛠️ Fix: If full_expression is "0" or "-0", replace it nicely
        if (inputLabel == "." && (full_expression.empty() || full_expression == "0" || full_expression == "-0")) {
            if (!current_input.empty() && current_input[0] == '-') {
                full_expression = "-0.";
            } else {
                full_expression = "0.";
            }
        } else if (full_expression == "0" || full_expression == "-0") {
            if (!current_input.empty() && current_input[0] == '-') {
                full_expression = "-" + inputLabel;
            } else {
                full_expression = inputLabel;
            }
        } else {
            full_expression += inputLabel;
        }


        current_input += inputLabel;

        // Correctly set current_value, including any negative sign
        try {
            std::string evaluated_input = current_input;
            std::string real_expression;
            for (size_t i = 0; i < evaluated_input.size(); ++i) {
                if (evaluated_input.substr(i,2) == "pi") {
                    real_expression += "3.1415926535";
                    i += 1; // skip "pi"
                }
                else if (evaluated_input[i] == 'e') {
                    real_expression += "2.7182818284";
                }
                else {
                    real_expression += evaluated_input[i];
                }
            }
            double val = std::stod(real_expression);
            current_value = std::to_string(val);

            // trim trailing 0's nicely
            if (current_value.find('.') != std::string::npos) {
                current_value.erase(current_value.find_last_not_of('0') + 1, std::string::npos);
                if (current_value.back() == '.') {
                    current_value.pop_back();
                }
            }
        } catch (...) {
            // fallback if something explodes
            current_value = current_input;
        }

    }

}


/**
 * @brief glfw callback for special key input
 *
 * handles non-character keys like escape, enter, or R
 * sends mapped string to process_input
 * leaves regular characters for character_callback
 *
 * @param window glfw window context
 * @param key key code pressed
 * @param scancode hardware key code (unused)
 * @param action key press type (press/repeat/release)
 * @param mods modifier keys (shift/ctrl etc.)
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // handle key only if pressed or held down
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_BACKSPACE: process_input("CE"); break;  // backspace = CE
            case GLFW_KEY_DELETE:    process_input("C");  break;  // delete = C
            case GLFW_KEY_ENTER:     process_input("=");  break;
            case GLFW_KEY_R:         process_input("sqrt"); break;
            case GLFW_KEY_LEFT_BRACKET: process_input("a^n"); break;
            case GLFW_KEY_RIGHT_BRACKET: process_input("!"); break;
            case GLFW_KEY_P:         process_input("pi"); break;
            case GLFW_KEY_E:         process_input("e"); break;
            default: break;
        }
    }

}

/**
 * @brief glfw callback for character input
 *
 * handles typed characters like numbers and operators
 * filters to allowed symbols and sends as string to process_input
 *
 * @param window glfw window context
 * @param codepoint unicode value of typed character
 */
void character_callback(GLFWwindow* window, unsigned int codepoint) {
    char c = static_cast<char>(codepoint); // convert to ascii char

    // only allow valid typed characters
    if (std::isdigit(c) || c == '.' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
        process_input(std::string(1, c)); // send one-char string to input
    }
}




/**
 * @brief Entry point of the application.
 *
 * Initialize the window, OpenGL context, loads resources
 * and runs the main render loop + logic.
 *
 * @return Exit code (0 == success).
 */
int main() {

    // print to confirm launch
    std::cout << "OpenGL Scene starting..." << std::endl;

    // try to initialize glfw
    if (!glfwInit()) {
        std::cerr << "GLFW init failed!" << std::endl;
        return -1; // fail early if glfw doesn't start
    }

    // set window version to opengl 3.3 (compatibility profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // major version 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // minor version 3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // for legacy compatibility

    // create a window sized 800x600 with title "Calculator"
    GLFWwindow* window = glfwCreateWindow(800, 600, "Calculator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open window!" << std::endl;
        glfwTerminate(); // safely shut down glfw
        return -1;       // exit with error
    }

    // activate the window's OpenGL context
    glfwMakeContextCurrent(window);

    // load OpenGL functions using glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1; // exit if GLAD fails
    }

    GLuint screen_FBO, screen_Texture; // framebuffer + texture for screen

    // create framebuffer object
    glGenFramebuffers(1, &screen_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, screen_FBO);

    // create 2D texture to draw into
    glGenTextures(1, &screen_Texture);
    glBindTexture(GL_TEXTURE_2D, screen_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // 512x256 resolution
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_Texture, 0);

    // check that framebuffer is valid
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "failed to create screen framebuffer!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind


    // set callbacks for input
    glfwSetKeyCallback(window, key_callback);         // for special key presses (enter, esc, etc.)
    glfwSetCharCallback(window, character_callback);  // for character input (numbers, operators, etc.)

    // get the size of the framebuffer (needed for viewport later)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);


    // load texture of font
    TextRenderer textRenderer(width, height);
    textRenderer.Load("fonts/LiberationSans-Bold.ttf", 48);

    //std::string full_expression = "12 + 3 *";     // top small line
    //std::string current_value    = "36";          // bottom large line


    // enable depth testing so objects render correctly in 3d
    glEnable(GL_DEPTH_TEST);

    // create shader program for the main scene
    GLuint shader = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // create shader program for the skybox
    GLuint skybox_shader = createShaderProgram("shaders/skybox.vert", "shaders/skybox.frag");

    // create vao and vbo for any debug or fallback geometry
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao); // bind vao

    // setup position attribute (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // setup normal attribute (nx, ny, nz)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // setup skybox geometry
    GLuint skybox_VAO, skybox_VBO;
    glGenVertexArrays(1, &skybox_VAO);
    glGenBuffers(1, &skybox_VBO);
    glBindVertexArray(skybox_VAO);                      // bind skybox vao
    glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);          // bind vbo to it
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW); // upload vertices
    glEnableVertexAttribArray(0);                      // enable attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // set up position only

    // load paths to all 6 skybox faces
    std::vector<std::string> faces = {
        "textures/skybox/px.jpg", // right
        "textures/skybox/nx.jpg", // left
        "textures/skybox/py.jpg", // top
        "textures/skybox/ny.jpg", // bottom
        "textures/skybox/pz.jpg", // front
        "textures/skybox/nz.jpg"  // back
    };

    // launch background thread to load cubemap images
    std::thread loaderThread([faces]() {
            for (int i = 0; i < 6; ++i) {
            images[i] = stbi_load(faces[i].c_str(), &widths[i], &heights[i], &channels[i], 0);
            if (!images[i]) {
            std::cerr << "Failed to load: " << faces[i] << std::endl;
            }
            cubemap_loaded_faces++; // increment on each successful load
            }
            cubemap_ready = true;
            });


    // camera rotation variables
    float yaw = 0.0f;               // current horizontal angle
    float pitch = 0.0f;             // current vertical angle
    float target_yaw = 0.0f;        // desired horizontal angle
    float target_pitch = 0.0f;      // desired vertical angle

    double last_x = 0.0, last_y = 0.0; // last mouse position
    bool dragging = false;            // whether the mouse is dragging

    // configure glfw input mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // show mouse
    glfwSetScrollCallback(window, scroll_callback);             // zoom with scroll

    // create texture handle for calculator texture (used in shader)
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // set texture filtering (smooth)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load png image from disk
    int w, h, ch;
    unsigned char* data = stbi_load("textures/others/Baker.png", &w, &h, &ch, 0);
    if (data) {
        // send image to gpu
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (ch == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
    }
    stbi_image_free(data); // clean up image from RAM

    // load calculator 3d model (.obj) and its materials
    Mesh calculator = load_obj_model("objects/calc.obj", "objects/");

    // define clickable buttons in 3d space (position, size, label)
    std::vector<Button> buttons = {
        { glm::vec3(-0.068487f, 0.020152f, 0.009860f), 0.01f, "C" },
        { glm::vec3(-0.035860f, 0.020152f, 0.009860f), 0.01f, "CE" },
        { glm::vec3(0.062499f, -0.015371f, 0.009860f), 0.01f, "/" },
        { glm::vec3(-0.001081f, -0.118039f, 0.009860f), 0.01f, "." },
        { glm::vec3(-0.001081f, -0.025285f, 0.009860f), 0.01f, "8" },
        { glm::vec3(0.030847f, -0.118039f, 0.009860f), 0.01f, "=" },
        { glm::vec3(-0.001081f, -0.056747f, 0.009860f), 0.01f, "5" },
        { glm::vec3(-0.033242f, -0.056747f, 0.009860f), 0.01f, "4" },
        { glm::vec3(0.033742f, 0.020152f, 0.009860f), 0.01f, "e" },
        { glm::vec3(0.062499f, -0.072665f, 0.009860f), 0.01f, "-" },
        { glm::vec3(-0.064406f, -0.015371f, 0.009860f), 0.01f, "%" },
        { glm::vec3(0.030847f, -0.025285f, 0.009860f), 0.01f, "9" },
        { glm::vec3(-0.033242f, -0.087510f, 0.009860f), 0.01f, "1" },
        { glm::vec3(0.062499f, -0.100775f, 0.009860f), 0.01f, "+" },
        { glm::vec3(0.065115f, 0.020152f, 0.009860f), 0.01f, "pi" },
        { glm::vec3(-0.033242f, -0.025285f, 0.009860f), 0.01f, "7" },
        { glm::vec3(0.030847f, -0.056747f, 0.009860f), 0.01f, "6" },
        { glm::vec3(0.030847f, -0.087510f, 0.009860f), 0.01f, "3" },
        { glm::vec3(-0.001081f, -0.087510f, 0.009860f), 0.01f, "2" },
        { glm::vec3(-0.033242f, -0.118039f, 0.009860f), 0.01f, "0" },
        { glm::vec3(-0.064406f, -0.046000f, 0.009860f), 0.01f, "sqrt" },
        { glm::vec3(-0.064406f, -0.075000f, 0.009860f), 0.01f, "a^n" },
        { glm::vec3(-0.064406f, -0.104000f, 0.009860f), 0.01f, "!" },
        { glm::vec3(0.062499f, -0.043000f, 0.009860f), 0.01f, "*" }
    };


    /*
     *  ___             _     _                  
     * | __|_ _____ _ _| |_  | |   ___  ___ _ __ 
     * | _|\ V / -_) ' \  _| | |__/ _ \/ _ \ '_ \
     * |___|\_/\___|_||_\__| |____\___/\___/ .__/
     *                                     |_|   
     *
     *  main loop — handles all input, updates, and rendering
     *  runs until user closes the window
     */

    std::unordered_set<std::string> printed;
    for (const auto& mat : calculator.materials) {
        if (!printed.count(mat.name)) {
            std::cout << "Material: " << mat.name << std::endl;
            printed.insert(mat.name);
        }
    }


    current_value = "0";
    full_expression = "";
    while (!glfwWindowShouldClose(window)) {

        if (show_loading && !cubemap_ready) {
            glClearColor(0.0f, 0.0f, 0.1f, 1.0f);  
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Correct projection for upright text (Y grows up)
            glm::mat4 uprightProj = glm::ortho(0.0f, static_cast<float>(width),
                    0.0f, static_cast<float>(height));

            glUseProgram(textRenderer.GetShaderID());
            glUniformMatrix4fv(glGetUniformLocation(textRenderer.GetShaderID(), "projection"),
                    1, GL_FALSE, glm::value_ptr(uprightProj));

            float scale = 1.5f;
            float centerX = width / 2.0f - 120.0f;  // Adjust this value to move the text further left
            float centerY = height / 1.2f;

            // Move "Loading" a bit higher, adjust "..." and "1/6" further below "Loading"
            textRenderer.RenderText("Loading", centerX, centerY , scale, glm::vec3(1.0f));  // Moved up
            std::string dots(std::min(6, cubemap_loaded_faces.load()), '.');
            textRenderer.RenderText(dots, centerX + 20.0f, centerY - 60, 3.2f, glm::vec3(1.0f));  // Adjusted positioning for dots
            std::string countStr = std::to_string(cubemap_loaded_faces.load()) + "/6";
            textRenderer.RenderText(countStr, centerX + 100.0f, centerY - 120, 1.2f, glm::vec3(1.0f));  // Adjusted position for count

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }


        // =================
        //       input
        // =================

        // check if middle mouse button is held
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {

            // if this is the first frame of dragging, store cursor position
            if (!dragging) {
                glfwGetCursorPos(window, &last_x, &last_y);
                dragging = true;
            } else {
                // get current cursor position
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                // calculate cursor movement delta
                double dx = xpos - last_x;
                double dy = ypos - last_y;

                // rotate view based on cursor movement
                target_yaw   -= dx * 0.3f;
                target_pitch += dy * 0.3f;

                // clamp pitch and yaw
                if (target_yaw > 89.0f) target_yaw = 89.0f;
                if (target_pitch < -89.0f) target_pitch = -89.0f;

                last_x = xpos;
                last_y = ypos;
            }

        } else {
            // if not dragging, smooth out motion
            dragging = false;
            target_yaw   *= 0.75f;
            target_pitch *= 0.75f;
        }

        // smooth interpolation of camera rotation
        yaw   += (target_yaw - yaw) * 0.1f;
        pitch += (target_pitch - pitch) * 0.1f;

        // =================
        //    camera setup
        // =================

        // create camera direction vector based on yaw and pitch
        glm::vec3 direction;
        direction.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));

        // set camera position by offsetting from origin
        glm::vec3 camera_pos = direction * radius;

        // create view and projection matrices
        glm::mat4 view = glm::lookAt(camera_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f); // base model transform
        model = glm::scale(model, glm::vec3(10.0f)); // scale calculator model

        // update viewport to current window size (handle resizes or wm oddities)
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // =======================
        //     rendering the calc
        // =======================

        // smooth zooming (camera distance) using interpolation
        radius += (target_radius - radius) * 0.1f;

        // bind offscreen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, screen_FBO);

        // set viewport to texture resolution
        glViewport(0, 0, 512, 256);

        // clear with transparent black (or solid black)
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // disable depth for 2d drawing
        glDisable(GL_DEPTH_TEST);

        // enable blending for alpha text
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float screen_WIDTH = 756.0f;
        float screen_HEIGHT = 756.0f;

        float margin_RIGHT = 20.0f; // pixels from right edge
        float margin_TOP = 20.0f;   // pixels from top edge

        // ---- full_expression (history) ----
        float expr_SCALE = 2.0f;
        float max_Expr_width = screen_WIDTH - 2 * margin_RIGHT;

        std::string line1 = "";
        std::string line2 = "";
        std::string line3 = "";

        for (char c : full_expression) {
            if (textRenderer.CalculateTextWidth(line1 + c, expr_SCALE) <= max_Expr_width) {
                line1 += c;
            }
            else if (textRenderer.CalculateTextWidth(line2 + c, expr_SCALE) <= max_Expr_width) {
                line2 += c;
            }
            else if (textRenderer.CalculateTextWidth(line3 + c, expr_SCALE) <= max_Expr_width) {
                line3 += c;
            }
            else {
                if (line3.length() >= 3) {
                    line3.erase(line3.size() - 3);
                    line3 += "...";
                } else {
                    line3 = "...";
                }
                break; // no need to continue
            }
        }

        // Now render line1, line2, line3 separately

        float exprY = screen_HEIGHT - margin_TOP + expr_SCALE * 30.0f; // top line

        if (!line1.empty()) {
            float exprWidth1 = textRenderer.CalculateTextWidth(line1, expr_SCALE);
            float exprX1 = screen_WIDTH - exprWidth1 - margin_RIGHT;
            textRenderer.RenderText(line1, exprX1, exprY, expr_SCALE, glm::vec3(0.7f));
        }

        if (!line2.empty()) {
            float exprWidth2 = textRenderer.CalculateTextWidth(line2, expr_SCALE);
            float exprX2 = screen_WIDTH - exprWidth2 - margin_RIGHT;
            textRenderer.RenderText(line2, exprX2, exprY - 75.0f, expr_SCALE, glm::vec3(0.7f)); // +50 for second line
        }

        if (!line3.empty()) {
            float exprWidth3 = textRenderer.CalculateTextWidth(line3, expr_SCALE);
            float max_Expr_width = screen_WIDTH - 2 * margin_RIGHT;

            if (exprWidth3 > max_Expr_width) {
                while (textRenderer.CalculateTextWidth("..." + line3, expr_SCALE) > max_Expr_width && line3.length() > 1) {
                    line3.erase(0, 1); // Remove characters from the start
                }
                line3 = "..." + line3; // Prepend "..." AFTER trimming
                exprWidth3 = textRenderer.CalculateTextWidth(line3, expr_SCALE); // recalculate
            }

            float exprX3 = screen_WIDTH - exprWidth3 - margin_RIGHT;
            textRenderer.RenderText(line3, exprX3, exprY - 150.0f, expr_SCALE, glm::vec3(0.7f));
        }


        // --- draw current_input during typing ---

        if (!current_input.empty() && !just_evaluated) {
            float inputScale = 2.8f; // start big
            float minInputScale = 1.2f; // minimum shrink
            float maxInputWidth = screen_WIDTH - 2 * margin_RIGHT;

            float inputWidth = textRenderer.CalculateTextWidth(current_input, inputScale);

            if (inputWidth > maxInputWidth) {
                inputScale *= maxInputWidth / inputWidth;
                if (inputScale < minInputScale) inputScale = minInputScale;
            }

            float inputWidthFinal = textRenderer.CalculateTextWidth(current_input, inputScale);
            float inputX = screen_WIDTH - inputWidthFinal - margin_RIGHT;
            float inputY = exprY - 650.0f; // same Y position as answer

            textRenderer.RenderText(current_input, inputX, inputY, inputScale, glm::vec3(1.0f));
        }
        else {
            // --- draw current_value when not typing ---
            float base_scale = 2.8f;
            float min_scale = 1.6f;
            float target_scale = base_scale;

            std::string display_value = current_value;

            // Special handling for negatives
            if (!display_value.empty() && display_value[0] == '-') {
                // Keep the minus, cap rest to 15
                if (display_value.length() > 16) {
                    display_value = "-" + display_value.substr(1, 15);
                }
            } else {
                // Normal positive numbers
                if (display_value.length() > 16) {
                    display_value = display_value.substr(0, 16);
                }
            }

            size_t char_count = 0;
            for (char c : display_value) {
                if (c != '.' && c != '-') {
                    ++char_count;
                }
            }

            if (char_count > 9) {
                target_scale = base_scale * (9.0f / static_cast<float>(char_count));
                if (target_scale < min_scale) {
                    target_scale = min_scale;
                }
            }

            float value_width_final = textRenderer.CalculateTextWidth(display_value, target_scale);
            float value_x = screen_WIDTH - value_width_final - margin_RIGHT;
            float value_y = exprY - 650.0f;

            textRenderer.RenderText(display_value, value_x, value_y, target_scale, glm::vec3(1.0f));
        }



        // cleanup state
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // return to normal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // activate texture unit 0
        glActiveTexture(GL_TEXTURE0);

        // bind calculator texture
        glBindTexture(GL_TEXTURE_2D, tex);

        // send texture to shader as "tex" uniform
        glUniform1i(glGetUniformLocation(shader, "tex"), 0);

        // set background color (dark grey) and clear buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use main shader program
        glUseProgram(shader);

        // send camera position as light position and view position
        glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(camera_pos));
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(camera_pos));

        // override light position to a fixed point in front of the scene
        glm::vec3 light_position = glm::vec3(0.0f, 0.0f, 5.0f);
        glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(light_position));

        // send model/view/projection matrices to the shader
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // setup basic directional light (used in fragment shader)
        glUniform3f(glGetUniformLocation(shader, "lightDir"), -0.5f, -1.0f, -0.3f);
        glm::vec3 dimLight = glm::vec3(0.3f); // soft light
        glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(dimLight));

        // set base color of object (light blue)
        glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.7f, 1.0f);
        // loop through all submeshes of the calculator model
        for (const auto& sub : calculator.submeshes) {
            GLuint texID = 0;          // default texture id
            std::string matName;       // name of the material

            // if the submesh has a valid material, get its info
            if (sub.material_ID >= 0 && static_cast<size_t>(sub.material_ID) < calculator.materials.size()) {
                texID = calculator.materials[sub.material_ID].illum; // we store texture id in illum (repurposed)
                matName = calculator.materials[sub.material_ID].name;
            }

            // activate shader and texture
            glUseProgram(shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texID);
            glUniform1i(glGetUniformLocation(shader, "tex"), 0);

            // update matrix uniforms
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            // setup light direction and color
            glUniform3f(glGetUniformLocation(shader, "lightDir"), -0.5f, -1.0f, -0.3f);
            glUniform3f(glGetUniformLocation(shader, "lightColor"), 0.7f, 0.7f, 0.7f);

            // 🌿 set green if it's the screen material, else default to white
            // if this is the calculator screen, use dynamic texture
            if (matName == "Material.027") {
                glBindTexture(GL_TEXTURE_2D, screen_Texture); // use rendered screen
            } else {
                glBindTexture(GL_TEXTURE_2D, texID); // normal material texture
            }

            // draw the submesh
            glBindVertexArray(sub.vao);
            glDrawElements(GL_TRIANGLES, sub.indices.size(), GL_UNSIGNED_INT, 0);
        }

        // handle mouse clicks with debounce (one click per press)
        static bool was_pressed = false; // remember last press state

        // check if left mouse is pressed and wasn't pressed before
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !was_pressed) {
            was_pressed = true; // mark press to avoid spamming

            double mx, my;
            glfwGetCursorPos(window, &mx, &my); // get mouse screen coords

            // convert screen to normalized device coordinates (-1 to 1)
            float ndc_x = 2.0f * mx / width - 1.0f;
            float ndc_y = 1.0f - 2.0f * my / height;
            glm::vec4 ray_clip(ndc_x, ndc_y, -1.0f, 1.0f); // ray in clip space

            // convert ray to eye space
            glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f); // set direction

            // convert ray to world space
            glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
            ray_world = glm::normalize(ray_world); // make it unit vector

            // check ray collision with each button
            for (const auto& btn : buttons) {
                // direction to button from camera
                glm::vec3 toButton = (btn.position * 10.0f - camera_pos);

                // t = projected distance along ray
                float t = glm::dot(toButton, ray_world);
                if (t < 0.0f) continue; // ignore buttons behind camera

                // point on ray closest to button
                glm::vec3 closest = camera_pos + ray_world * t;

                // distance from that point to button center
                float distance = glm::length(closest - btn.position * 10.0f);

                // if within button radius, it's a hit
                if (distance < btn.size * 15.0f) {
                    process_input(btn.label); // process the button label
                }
            }

        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            was_pressed = false; // reset press state on release
        }

        static bool uploadedCubemap = false; // one-time upload flag

        // if cubemap is ready and not yet uploaded
        if (cubemap_ready && !uploadedCubemap) {
            glGenTextures(1, &cubemap_texture); // generate texture id
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture); // bind cube map

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
            cubemap_loaded = true;
            show_loading = false;

            // ✅ Restore normal HUD Y projection (origin bottom-left)
            glm::mat4 normalProj = glm::ortho(0.0f, static_cast<float>(width),
                    static_cast<float>(height), 0.0f);
            glUseProgram(textRenderer.GetShaderID());
            glUniformMatrix4fv(glGetUniformLocation(textRenderer.GetShaderID(), "projection"),
                    1, GL_FALSE, &normalProj[0][0]);
        }


        // render skybox
        glDepthFunc(GL_LEQUAL); // allow skybox to draw behind everything

        if (cubemap_loaded) {
            glUseProgram(skybox_shader); // use skybox shader

            glm::mat4 viewSky = glm::mat4(glm::mat3(view)); // remove translation from view
            glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "view"), 1, GL_FALSE, glm::value_ptr(viewSky));
            glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glBindVertexArray(skybox_VAO);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
            glDrawArrays(GL_TRIANGLES, 0, 36); // draw cube
        } else {
            // draw placeholder cube while loading
            glUseProgram(shader);
            glm::mat4 loadingModel = glm::scale(model, glm::vec3(0.5f));
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(loadingModel));
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glm::mat4 flippedDefault = glm::ortho(0.0f, static_cast<float>(width),
                static_cast<float>(height), 0.0f); // flipped Y

        glUseProgram(textRenderer.GetShaderID());
        glUniformMatrix4fv(glGetUniformLocation(textRenderer.GetShaderID(), "projection"),
                1, GL_FALSE, glm::value_ptr(flippedDefault));


        glDepthFunc(GL_LESS); // restore default depth function

        // attempt to render input over screen (not yet finished)
        /*
           for (const auto& sub : calculator.submeshes) {
           if (sub.material_ID >= 0 && static_cast<size_t>(sub.material_ID) < calculator.materials.size()) {
           const auto& mat = calculator.materials[sub.material_ID];
           if (mat.name == "Material.027") {
           glm::vec4 screenWorldPos = model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
           glm::vec4 clipSpace = projection * view * screenWorldPos;

           if (clipSpace.w != 0.0f) {
           clipSpace /= clipSpace.w; // perspective divide

           float x = (clipSpace.x * 0.5f + 0.5f) * width;
           float y = (clipSpace.y * 0.5f + 0.5f) * height;

        //textRenderer.RenderText(current_input, x, y, 0.8f, glm::vec3(0.0f, 0.9f, 0.1f));
        }
        break;
        }
        }
        }
        */

        //textRenderer.RenderText(current_input, 100.0f, 60.0f, 1.0f, glm::vec3(0.0f)); // fallback text render

        glfwSwapBuffers(window); // swap front and back buffer
        glfwPollEvents();        // handle window + input events
    }

    loaderThread.join(); // wait for skybox thread to finish
    glfwTerminate();     // shutdown window + context
    return 0;            // exit successfully
}

/* end of file main.cpp */

