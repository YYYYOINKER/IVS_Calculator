/**
 * @file TextRenderer.cpp
 * @brief Implementation of text rendering using FreeType and OpenGL.
 * 
 * Contains functions to load fonts, generate character textures, 
 * render strings at specified positions, and measure text widths.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "pather.h"


extern std::string loadShaderSource(const char* path);
extern GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);



TextRenderer::TextRenderer(unsigned int width, unsigned int height) {
    // Compile and setup the shader
    shaderID = createShaderProgram(pather("shaders/text.vert").c_str(), pather("shaders/text.frag").c_str());

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width),
                                  static_cast<float>(height), 0.0f); // flip Y

    glUseProgram(shaderID);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, &projection[0][0]);

    // Configure VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void TextRenderer::Load(const std::string& fontPath, unsigned int fontSize) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "FREETYPE: Failed to init FreeType" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "FREETYPE: Failed to load font " << fontPath << std::endl;
        return;
    }

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
    std::cerr << "FREETYPE: Failed to load font " << fontPath << std::endl;
    FT_Done_FreeType(ft);
    return;
}


    FT_Set_Pixel_Sizes(face, 0, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters[c] = character;

    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}



float TextRenderer::CalculateTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    for (char c : text) {
        if (Characters.count(c)) {
            width += (Characters[c].advance >> 6) * scale; // advance is in 1/64 pixels
        }
    }
    return width;
}

void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glUseProgram(shaderID);
    glUniform3f(glGetUniformLocation(shaderID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    for (auto c = text.begin(); c != text.end(); ++c) {
        if (Characters.find(*c) == Characters.end()) {
            std::cerr << "Missing glyph for character: " << *c << std::endl;
            continue;  // Correctly placed continue statement inside loop
        }

        Character ch = Characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

