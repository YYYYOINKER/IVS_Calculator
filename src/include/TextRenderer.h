#pragma once

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/gl.h>

struct Character {
    GLuint textureID;   // ID handle of the glyph texture
    glm::ivec2 size;     // Size of glyph
    glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
    GLuint advance;      // Offset to advance to next glyph
};

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height);
    void Load(const std::string& font, unsigned int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);

private:
    std::map<char, Character> Characters;
    GLuint VAO, VBO;
    GLuint shaderID;
};

