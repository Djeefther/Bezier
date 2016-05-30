#ifndef UTILS_H_
#define UTILS_H_

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <queue>
#include <deque>
#include <stack>
#include <exception>
#include <stdexcept>
#include <iterator>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <array>
#include <gl/glut.h>

#define PRINT_VALUE(a) std::cout << #a << " = " << (a) << " "
#define PRINTLN_VALUE(a) std::cout << #a << " = " << (a) << "\n"
#define FILE_PRINT_VALUE(file,a) (file) << #a << " = " << (a) << " "
#define FILE_PRINTLN_VALUE(file,a) (file) << #a << " = " << (a) << "\n"

typedef std::array<float, 3> color_t;
typedef unsigned char uchar;
inline color_t color(float red, float green, float blue);
inline color_t color_uchar(uchar red, uchar green, uchar blue);
inline void glColor_t(const color_t& color);


void glPrintText(int x, int y, color_t color, void * font, const char *string);

/////////////////////////////////////////////// 

inline color_t color(float red, float green, float blue) {
	color_t c = { red, green, blue };
	return c;
}

inline color_t color_uchar(uchar red, uchar green, uchar blue) {
	color_t c = { float(red) / 255.0f, float(green) / 255.0f, float(blue) / 255.0f };
	return c;
}


inline void glColor_t(const color_t& color) {
	glColor3f(color[0], color[1], color[2]);
}



#endif