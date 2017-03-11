#pragma once
#include "../../Resource/Resource.h"
#include <map>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

class Font : public Resource {

public:

    struct Character {
      float ax; // advance.x
      float ay; // advance.y

      float bw; // bitmap.width;
      float bh; // bitmap.rows;

      float bl; // bitmap_left;
      float bt; // bitmap_top;

      float tx; // x offset of glyph in texture coordinates
    } character[128];

    Font();
    ~Font();
    void setSize(int size);
    int getSize() const { return size; }
    void load(const std::string& path) override;
    void renderText(const std::string& text, float x, float y, float sx, float sy);
    glm::vec2 getAtlasSize() { return glm::vec2(atlasWidth, atlasHeight); }

private:
    int size = 14;
    FT_Face face;
    int atlasWidth;
    int atlasHeight;
};
