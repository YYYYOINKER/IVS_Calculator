
#pragma once
#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>

/**
 * @brief Structure to hold font character information
 */
struct Character {
    GLuint textureID;     // ID handle of the glyph texture
    glm::ivec2 size;      // Size of glyph
    glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
    unsigned int advance; // Offset to advance to next glyph
};

/**
 * @brief Text rendering class using FreeType and OpenGL
 */
class TextRenderer {
public:
    /**
     * @brief Constructor to initialize the renderer
     * @param width Width of screen or framebuffer
     * @param height Height of screen or framebuffer
     */
    TextRenderer(unsigned int width, unsigned int height);

    /**
     * @brief Loads font from file and prepares glyphs
     * @param fontPath Path to .ttf font file
     * @param fontSize Size in pixels
     */
    void Load(const std::string& fontPath, unsigned int fontSize);

    /**
     * @brief Renders given text to screen at position
     * @param text The string to render
     * @param x X position (pixels)
     * @param y Y position (pixels)
     * @param scale Size multiplier
     * @param color RGB color of text
     */
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);

    float CalculateTextWidth(const std::string& text, float scale);

    /**
     * @brief Getter for internal shader program ID
     * @return GLuint shader program ID
     */
    GLuint GetShaderID() const { return shaderID; }

private:
    GLuint VAO, VBO;                      // OpenGL vertex array + buffer
    GLuint shaderID;                      // OpenGL shader program ID
    std::map<char, Character> Characters; // Loaded characters
};

#endif // TEXT_RENDERER_H

