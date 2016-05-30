#include "utils.h"

void glPrintText(int x, int y, color_t color, void * font, const char *string)
{
	glColor_t(color);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, string[i]);
	}
}
