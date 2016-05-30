#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <gl/glut.h>
#include <algorithm>
#include <array>
#include <vector>
#include <sstream>
#include <string>

#include "utils.h"
#include "Bezier.h"
#include "Point.h"

//Parameters:
#define DELTA_T_BEZIER 0.001
#define WIDTH 800
#define HEIGHT 600
#define RADIUS_SELECT_POINT 8

#define ITERATIVE_BEZIER_VELOCITY_INIT 3
#define ITERATIVE_BEZIER_VELOCITY_MIN 1
#define ITERATIVE_BEZIER_VELOCITY_MAX 500

//Parameters of the colors and size of points and lines
const color_t background_help_color = color_uchar(0x1B,0x58,0x7B);
const color_t text_help_color = color_uchar(0xEC, 0xD0, 0x78);
const color_t bezier_line_color = { 1.0f, 0.0f, 0.0f };
const float bezier_line_width = 3.0f;
const color_t control_points_color = {0.0f,0.3f,0.3f};
const float control_points_size = 8.0f;
const color_t lines_between_control_points_color = { 0.0f, 0.45f, 0.45f };
const float lines_between_control_points_width = 2.0f;
std::vector<color_t> intermediate_lines_colors;
const float intermediate_lines_width = 1.0f;
std::vector<color_t>& intermediate_points_colors = intermediate_lines_colors;
const float intermediate_points_width = 3.0f;

//List of the colors used in iterative mode:
void init_intermediate_lines_colors() {
	intermediate_lines_colors.push_back(color_uchar(0, 169, 235));//Bright blue 
	intermediate_lines_colors.push_back(color_uchar(255, 233, 0));//Serdar Yellow
	intermediate_lines_colors.push_back(color_uchar(149, 119, 0));//Orange Crush
	intermediate_lines_colors.push_back(color_uchar(2,134,152));//Silent deadly blue
	intermediate_lines_colors.push_back(color_uchar(0, 190, 33));//Zinger Green
	intermediate_lines_colors.push_back(color_uchar(17, 34, 51));////Coldfusion
	intermediate_lines_colors.push_back(color_uchar(80, 0, 255));//Blue Violet
	intermediate_lines_colors.push_back(color_uchar(114, 102, 89));//Ia Mujer Diosa
	intermediate_lines_colors.push_back(color_uchar(251, 176, 52));//Orange
}

//Global control variables
Bezier<Point2d> bezier;
int point_selected = -1;
std::vector<Point2d> actual_bezier_points;
bool show_help = true;

//iterative bezier:
bool iterative_bezier = true;
double iterative_bezier_t = 0.0;
bool iterative_bezier_paused = false;
unsigned iterative_bezier_velocity = ITERATIVE_BEZIER_VELOCITY_INIT;

//undo
std::vector<Point2d> undo_points;
//screen resolution
int screen_width = WIDTH;
int screen_height = HEIGHT;

void reset_iterative_bezier() {
	iterative_bezier_t = 0.0;
	iterative_bezier_paused = false;
	actual_bezier_points.clear();
}

void window_resize(GLsizei w, GLsizei h) {
	screen_width = std::max(100, w);
	screen_height = std::max(100, h);

	glViewport(0, 0, screen_width, screen_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.f, screen_width, screen_height, 0.f);
}

void draw_text() {
	if (show_help) {
		auto font = GLUT_BITMAP_8_BY_13;
		int char_height = 13;
		int char_width = 8;
		int margin = 4;

		int lines = 13;
		int coluns = 40;

		int text_height = lines * char_height;
		int text_width = coluns * char_width;

		int x = screen_width / 2 - text_width / 2;
		int y = screen_height / 2 - text_height / 2;


		auto pos_inc_y = [&](){
			auto aux = y;
			y += char_height;
			return aux;
		};

		//Draw a square in the end
		glColor_t(background_help_color);
		glBegin(GL_QUADS); {
			int x_begin = x - margin;
			int x_end = x + text_width + margin;
			int y_begin = y - char_height - margin;
			int y_end = y - char_height + text_height + margin;

			glVertex2i(x_begin, y_begin);
			glVertex2i(x_end, y_begin);
			glVertex2i(x_end, y_end);
			glVertex2i(x_begin, y_end);
		}

		glEnd();

		//Desenha o texto
		glPrintText(x + text_width / 2 - (char_width*4)/2, pos_inc_y(), text_help_color, font, "HELP");
		pos_inc_y();
		glPrintText(x, pos_inc_y(), text_help_color, font, "H:          Help");
		glPrintText(x, pos_inc_y(), text_help_color, font, "Click:      Add or move a point");
		glPrintText(x, pos_inc_y(), text_help_color, font, "Backspace:  Remove last added point");
		glPrintText(x, pos_inc_y(), text_help_color, font, "C:          Remove all points");
		glPrintText(x, pos_inc_y(), text_help_color, font, "CTRL+Z:     Undo a remove");
		glPrintText(x, pos_inc_y(), text_help_color, font, "I:          Toggle iterative mode");
		glPrintText(x, pos_inc_y(), text_help_color, font, "+:          Speed up the iterative mode");
		glPrintText(x, pos_inc_y(), text_help_color, font, "-:          Slow down the iterative mode");
		glPrintText(x, pos_inc_y(), text_help_color, font, "Spacebar:   Pause iterative mode");
		glPrintText(x, pos_inc_y(), text_help_color, font, "R:          Randomly add a point");
		glPrintText(x, pos_inc_y(), text_help_color, font, "Enter:      Close this window");

	}

	//Print general data
	{
		std::stringstream ss;
		int x = screen_width - 110;
		int y = 20;
		int char_height = 13;

		auto pos_inc_y = [&](){
			auto aux = y;
			y += char_height;
			return aux;
		};


		ss << "Points: " << bezier.control_points().size();
		glPrintText(x, pos_inc_y(), color(0, 0, 0), GLUT_BITMAP_8_BY_13, ss.str().c_str());
		ss.str(std::string());

		if (iterative_bezier) {
			ss << "Velocity: " << iterative_bezier_velocity;
			glPrintText(x, pos_inc_y(), color(0, 0, 0), GLUT_BITMAP_8_BY_13, ss.str().c_str());
			ss.str(std::string());
		}

		if (iterative_bezier_paused) {
			glPrintText(x, pos_inc_y(), color(0, 0, 0), GLUT_BITMAP_8_BY_13, "Paused");
		}
	}

#if _DEBUG
	{
		int x = screen_width / 2 - (20*18)/2;
		int y = 20;
		glPrintText(x, y, color(255, 0, 0), GLUT_BITMAP_HELVETICA_18, "WARNING: COMPILED IN DEBUG MODE!!!!");
		glPrintText(x, y+18, color(255, 0, 0), GLUT_BITMAP_HELVETICA_18, "PLEASE REBUILD IN RELEASE MODE!!!!");
	}
#endif
	
}

void draw() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);

	auto draw_strip_line = [](const std::vector<Point2d>& points) {
		glBegin(GL_LINE_STRIP);
		for (const auto& i : points) {
			glVertex2d(i.x, i.y);
		}
		glEnd();
	};

	auto draw_points = [](const std::vector<Point2d>& points) {
		glBegin(GL_POINTS);
		for (const auto& i : points) {
			glVertex2d(i.x, i.y);
		}
		glEnd();
	};

	if (bezier.valid()) {
		//Compute the bezier points (all of them if not in iterative mode)
		if (iterative_bezier) {
			if (!iterative_bezier_paused) {
				for (unsigned i = 0; i < iterative_bezier_velocity; i++)  {
					actual_bezier_points.emplace_back(bezier.point(std::min(iterative_bezier_t, 1.0)));
					iterative_bezier_t += DELTA_T_BEZIER;
				}
			}
		}
		else {
			for (double t = 0; t < 1.0 + DELTA_T_BEZIER; t += DELTA_T_BEZIER) {
				actual_bezier_points.emplace_back(bezier.point(std::min(t, 1.0)));
			}
		}

		if (iterative_bezier) {
			//Draw the auxiliary lines (just iterative mode)
			size_t color_level = 0;
			size_t color_level_limit = intermediate_lines_colors.size() - 1;
			glLineWidth(intermediate_lines_width);
			for (const auto& l : bezier.auxiliar_points()) {
				glColor_t(intermediate_lines_colors[color_level]);
				draw_strip_line(l);
				color_level = std::min(color_level + 1, color_level_limit);
			}


			//Draw the meeting point of the auxiliary lines of the bezier
			color_level = 0;
			color_level_limit = intermediate_lines_colors.size() - 1;
			glPointSize(intermediate_points_width);
			for (const auto& l : bezier.auxiliar_points()) {
				glColor_t(intermediate_points_colors[color_level]);
				draw_points(l);
				color_level = std::min(color_level + 1, color_level_limit);
			}
		}
	}

	if (bezier.control_points().size() > 0) {
		//Lines between control points
		glColor_t(lines_between_control_points_color);
		glLineWidth(lines_between_control_points_width);
		draw_strip_line(bezier.control_points());

		//Control points
		glColor_t(control_points_color);
		glPointSize(control_points_size);
		draw_points(bezier.control_points());
	}

	if (bezier.valid()) {
		//Draw a bezier (complet or not)
		glColor_t(bezier_line_color);
		glLineWidth(bezier_line_width);
		draw_strip_line(actual_bezier_points);

		//Checking for the next draw
		if (iterative_bezier) {
			if (iterative_bezier_t >= 1.0) {
				reset_iterative_bezier();
			}
		}
		else {
			actual_bezier_points.clear();
		}
	}

	draw_text();

	glutSwapBuffers();
	if (iterative_bezier)
		glutPostRedisplay();
}

int clicked_point(const std::vector<Point2d>& points, const Point2d& click) {
	int i = 0;
	for (const auto& p : points) {
		if (abs(p - click) <= RADIUS_SELECT_POINT) {
			return i;
		}
		i++;
	}
	return -1;
}

void movement_mouse(int x, int y) {
	if (point_selected >= 0) {
		bezier.control_points()[point_selected] = Point2d(x, y);

		undo_points.clear();
		reset_iterative_bezier();
		glutPostRedisplay();
	}

}

void click_mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			Point2d p(x, y);
			auto actual_point_selected = clicked_point(bezier.control_points(), p);

			if (actual_point_selected >= 0) {
				if (actual_point_selected == point_selected) {
					bezier.control_points().erase(bezier.control_points().begin() + point_selected);
					point_selected = -1;
				}
				else {
					point_selected = actual_point_selected;
				}
			}
			else {
				bezier.control_points().emplace_back(p);

				point_selected = actual_point_selected;//Remove selection if exists
				undo_points.clear();
				reset_iterative_bezier();

				glutPostRedisplay();
			}
		}
		else if (state == GLUT_UP) {
			point_selected = -1;
		}
	}
}

void keybord(unsigned char c, int x, int y) {
	switch (tolower(c))
	{
	case 'c': {
		for (auto it = bezier.control_points().rbegin(); it != bezier.control_points().rend(); it++) {
			undo_points.push_back(*it);
		}
		bezier.control_points().clear();

		point_selected = -1;
		reset_iterative_bezier();
		glutPostRedisplay();
		break;
	}
	case 8: {//backspace
		if (bezier.control_points().size() > 0) {
			undo_points.push_back(bezier.control_points().back());
			bezier.control_points().pop_back();

			point_selected = -1;
			reset_iterative_bezier();
			glutPostRedisplay();
		}
		break;
	}
	case 26: {//ctrl+z
		if (undo_points.size() > 0) {
			bezier.control_points().push_back(undo_points.back());
			undo_points.pop_back();
			
			point_selected = -1;
			reset_iterative_bezier();
			glutPostRedisplay();
		}
		break;
	}
	case 'h': {
		show_help = !show_help;
		glutPostRedisplay();
		break;
	}
	case '\r': { //Enter
		show_help = false;		
		glutPostRedisplay();
		break;

	}
	case ' ' : {
		iterative_bezier_paused = !iterative_bezier_paused;
		break;
	}
	case 'i': {
		iterative_bezier = !iterative_bezier;
		reset_iterative_bezier();
		glutPostRedisplay();
		break;
	}
	case '=':
	case '+': {
		if (iterative_bezier) {
			iterative_bezier_velocity = std::min(iterative_bezier_velocity + 1, (unsigned)ITERATIVE_BEZIER_VELOCITY_MAX);
		}
		break;
	}
	case '-':
	case '_': {
		if (iterative_bezier) {
			iterative_bezier_velocity = std::max(iterative_bezier_velocity - 1, (unsigned)ITERATIVE_BEZIER_VELOCITY_MIN);
		}
		break;
	}
	case 'r': {//random put
		int x = rand() % screen_width;
		int y = rand() % screen_height;

		bezier.control_points().emplace_back(x,y);

		point_selected = -1;//Remove the select if exists
		undo_points.clear();
		reset_iterative_bezier();

		glutPostRedisplay();
		break;
	}
	default:
		break;
	}
}



void init_gl() {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(	(glutGet(GLUT_SCREEN_WIDTH)	- WIDTH)/2,
							(glutGet(GLUT_SCREEN_HEIGHT) - HEIGHT)/2);

	glutCreateWindow("Curvas Bezier - dsa2");

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutReshapeFunc(window_resize);
	glutDisplayFunc(draw);
	glutMouseFunc(click_mouse);
	glutKeyboardFunc(keybord);
	glutMotionFunc(movement_mouse);


	glClearColor(1.f, 1.f, 1.f, 1.0f);

	init_intermediate_lines_colors();
}

int main() {
	init_gl();
	glutMainLoop();
}