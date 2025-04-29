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
 * @struct Character
 * @brief Structure to hold font character information.
 * 
 * Stores OpenGL texture ID, glyph size, bearing, and advance distance
 * for each character in the font.
 */
struct Character {
    GLuint textureID;     ///< ID handle of the glyph texture
    glm::ivec2 size;       ///< Size of glyph
    glm::ivec2 bearing;    ///< Offset from baseline to left/top of glyph
    unsigned int advance;  ///< Offset to advance to next glyph
};

/**
 * @class TextRenderer
 * @brief Class for text rendering using FreeType and OpenGL.
 * 
 * Initializes font loading, glyph texture preparation, and text rendering.
 */
class TextRenderer {
public:
    /**
     * @brief Constructor to initialize the renderer.
     * @param width Width of screen or framebuffer.
     * @param height Height of screen or framebuffer.
     */
    TextRenderer(unsigned int width, unsigned int height);

    /**
     * @brief Loads font from file and prepares glyph textures.
     * @param fontPath Path to .ttf font file.
     * @param fontSize Size of glyphs in pixels.
     */
    void Load(const std::string& fontPath, unsigned int fontSize);

    /**
     * @brief Renders given text string to screen at specified position.
     * @param text The string to render.
     * @param x X position in pixels.
     * @param y Y position in pixels.
     * @param scale Size multiplier.
     * @param color RGB color of text.
     */
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);

    /**
     * @brief Calculates the width of rendered text.
     * @param text The string to measure.
     * @param scale Size multiplier.
     * @return Width of the text in pixels.
     */
    float CalculateTextWidth(const std::string& text, float scale);

    /**
     * @brief Getter for internal shader program ID.
     * @return GLuint shader program ID.
     */
    GLuint GetShaderID() const { return shaderID; }

private:
    GLuint VAO, VBO;                      ///< OpenGL vertex array and buffer objects
    GLuint shaderID;                      ///< OpenGL shader program ID
    std::map<char, Character> Characters; ///< Map storing loaded glyphs.
};

#endif // TEXT_RENDERER_H

