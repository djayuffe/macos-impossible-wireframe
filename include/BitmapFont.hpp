#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace bitmapfont {
struct Glyph { int x=0,y=0,width=0,height=0,advance=0; };
struct Atlas {
 int width=3328,height=672;
 std::array<Glyph,36> glyphs{};
 std::vector<unsigned char> pixels;
};
// Crop the supplied irregular A-Z/0-9 sheet into individually padded cells.
// The original artwork is not modified. Pixels are copied at native resolution.
Atlas crop(const std::vector<unsigned char>& rgba,int width,int height);
int index(char c);
}
