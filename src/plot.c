#include "plot.h"

#include <corecrt_malloc.h>
#include <corecrt_math.h>
#include <corecrt_memcpy_s.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <corecrt_math_defines.h>

static plot_t plot = {0};

// Hash function from http://www.cse.yorku.ca/~oz/hash.html
static unsigned long hash(char* str) {
	unsigned long hash = 5381;
	int			  c;

	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

plot_t* plot_new(double width, double height) {
	memset(&plot, 0, sizeof(plot));

	plot.width	= width;
	plot.height = height;

	return &plot;
}

int plot_load_img(char* path) {
	return -1;
}

int plot_path_add(char* name) {

	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(plot.path_names[i] != NULL) {
			continue;
		}

		plot.path_names[i]	= name;
		plot.path_hashes[i] = hash(name);
		memset(&plot.paths[i], 0, sizeof(plot.paths[i]));

		plot.paths[i].size = 4;
		plot.paths[i].points =
			malloc(sizeof(plot_point_t) * plot.paths[i].size);
		plot.paths[i].radii = malloc(sizeof(plot_point_t) * plot.paths[i].size);

		return 0;
	}

	return -1;
}

int plot_path_del(char* name) {
	unsigned long h = hash(name);

	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(plot.path_hashes[i] == h) {
			plot.path_names[i]	= NULL;
			plot.path_hashes[i] = 0;

			memset(&plot.paths[i], 0, sizeof(plot.paths[i]));

			return 0;
		}
	}

	return -1;
}

int plot_path_sel(char* name) {
	unsigned long h = hash(name);

	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(plot.path_hashes[i] == h) {
			plot.sel = i;

			return 0;
		}
	}

	return -1;
}

static void* plot_buffer_resize(void* buf, int size, int new_size) {
	void* temp = malloc(size);

	// store buffer in temp buffer.
	memcpy(temp, buf, size);

	// resize the buffer.
	void* newbuf = realloc(buf, new_size);

	// repopulate buffer with previous values.
	memcpy(newbuf, temp, size);

	free(temp);

	return newbuf;
}

int plot_point_add(double x, double y) {

	// resize points buffer if it is too small to fit another point.
	if(plot.paths[plot.sel].point_count + 1 >= plot.paths[plot.sel].size) {
		int new_size = plot.paths[plot.sel].size * 2;

		// resize point buffer to fit more points.
		plot.paths[plot.sel].points =
			plot_buffer_resize(plot.paths[plot.sel].points,
							   plot.paths[plot.sel].size * sizeof(plot_point_t),
							   new_size * sizeof(plot_point_t));

		// now do the same but for the radii. note that radii has half the size
		// because there is only 1 radii point for every two points.
		plot.paths[plot.sel].radii =
			plot_buffer_resize(plot.paths[plot.sel].radii,
							   plot.paths[plot.sel].size * sizeof(plot_point_t),
							   new_size * sizeof(plot_point_t) / 2);

		plot.paths[plot.sel].size = new_size;
	}

	int index = plot.paths[plot.sel].point_count;

	plot.paths[plot.sel].points[index].x = x;
	plot.paths[plot.sel].points[index].y = y;

	// if index is even, we add a radii because a radii is needed for every two
	// points.
	if(index != 0 && index % 2 == 0) {
		// set the radii to the average of the new point and previous point.
		plot.paths[plot.sel].radii[index / 2].x =
			(plot.paths[plot.sel].points[index - 1].x + x) / 2;
		plot.paths[plot.sel].radii[index / 2].y =
			(plot.paths[plot.sel].points[index - 1].y + y) / 2;
	}

	plot.paths[plot.sel].point_count++;

	return 0;
}

int plot_point_del(int index) {
	return -1;
}

int plot_point_get_index(double x, double y, double radius) {
	for(int i = 0; i < plot.paths[plot.sel].point_count; i++) {
		if(calc_dist(plot.paths[plot.sel].points[i], (plot_point_t){x, y}) <=
		   radius) {
			return i;
		}
	}

	return -1;
}

int plot_point_set(int index, double x, double y) {
	plot.paths[plot.sel].points[index].x = x;
	plot.paths[plot.sel].points[index].y = y;

	return 0;
}
int plot_point_set_radius(int index, double x, double y) {
	// if index is zero, I can help you. sorry.
	if(index == 0) {
		return -1;
	}

	// set the index to be even if nessasery.
	if(index % 2 != 0) {
		index++;
	}

	plot.paths[plot.sel].radii[index / 2].x = x;
	plot.paths[plot.sel].radii[index / 2].y = y;

	return 0;
}

int plot_export(char* buffer, int buffer_size) {
	return -1;
}
int plot_import(char* buffer, int buffer_size) {
	return -1;
}

double plot_width() {
	return plot.width;
}
double plot_height() {
	return plot.height;
}

double plot_top() {
	return plot.origin.y + plot.height;
}

double plot_bottom() {
	return plot.origin.y;
}

double plot_right() {
	return plot.origin.x + plot.width;
}

double plot_left() {
	return plot.origin.x;
}

plot_point_t plot_origin() {
	return plot.origin;
}

plot_point_t plot_coord(double x, double y) {
	return (plot_point_t){plot_left() + x, plot_bottom() + y};
}

double plot_x(double x) {
	return plot_left() + x;
}

double plot_y(double y) {
	return plot_top() - (plot_bottom() + y);
}

plot_path_part_t calc_path_part(plot_point_t start, plot_point_t mid,
								plot_point_t end, double radius) {
	plot_path_part_t part = {0};

	// double		 r	= calc_dist(start, end) / 2;
	double		 r	= radius;
	plot_point_t co = calc_circ_center(start, end, r);

	double o1 = start.y > co.y ? start.y - co.y : co.y - start.y;
	double o2 = end.y > co.y ? end.y - co.y : co.y - end.y;

	// make opposite 1 and 2 absolute. Don't think this is nessasery.
	// o1 = o1 < 0 ? -o1 : o1;
	// o2 = o2 < 0 ? -o2 : o2;

	double a1 = calc_quadrant(asin(o1 / r), start.x - co.x, start.y - co.y);
	double a2 = calc_quadrant(asin(o2 / r), end.x - co.x, end.y - co.y);

	// make angle 1 and 2 unsigned to make maths work.
	if(a1 < 0) {
		a1 += 2 * M_PI;
	}
	if(a2 < 0) {
		a2 += 2 * M_PI;
	}

	// delta angle
	double da = a1 - a2;
	da		  = fmod(da + M_PI, 2 * M_PI) - M_PI;

	part.start_angle = a1;
	part.end_angle	 = a2;

	part.delta_angle = da;

	part.radius = r;
	part.origin = co;

	part.start = start;
	part.end   = end;
	part.mid   = mid;

	// get a projection of the mid point onto the line between the start and end
	// points.
	plot_point_t proj = calc_point_project(
		(plot_point_t){cos(a1 - da / 2) * r, sin(a1 - da / 2) * r},
		(plot_point_t){mid.x - co.x, mid.y - co.y});

	if(isnan(proj.x) | isnan(proj.y)) {
		puts("projected error");
		printf("a1 point=(%f, %f), a2 point(%f, %f)\n", start.x - co.x,
			   start.y - co.y, end.x - co.x, end.y - co.y);
		printf("o1=%f, o2=%f\n", o1, o2);
		printf("r=%f, dist=%f co.x=%f, co.y=%f\n", r, calc_dist(start, end) / 2,
			   co.x, co.y);
		printf("a1=%f, a2=%f, ad=%f\n", a1, a2, da);
		printf("p1=(%f, %f)\n", cos(a1 - da / 2) * r, sin(a1 - da / 2) * r);
		exit(0);
	}

	part.mid.x = proj.x + co.x;
	part.mid.y = proj.y + co.y;

	return part;
}

plot_point_t calc_point_project(plot_point_t p1, plot_point_t p2) {
	// y = mx + b  (aka the equation for a line)
	double m_1 = p1.y / p1.x;
	// m_2 must be the opposite of m_1 because we want to intersect the halfway
	// line.
	double a   = atan(p1.y / p1.x) + M_PI_2;
	double d   = sqrt(p1.x * p1.x + p1.y * p1.y);
	double m_2 = (sin(a) * d) / (cos(a) * d);

	// this is just the reverse of y = mx + b which is b = y - mx ... I think?
	double b = p2.y - (m_2 * p2.x);

	// with the system of equations y = m_1 * x, y = m_2 * x + b, to get x we
	// can use the equation x = b / (m_1 - m_2)

	// to get y we can use y = m_1 * x or y = m_2 * x + b. (either one works)

	double x = b / (m_1 - m_2);
	double y = m_1 * x;

	return (plot_point_t){x, y};
}

double calc_dist(plot_point_t p1, plot_point_t p2) {
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

double calc_map(double x, double in_min, double in_max, double out_max,
				double out_min) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

plot_point_t calc_circ_center(plot_point_t p1, plot_point_t p2, double r) {
	// get the center as p3
	plot_point_t p3 = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};

	double dist = calc_dist(p1, p2);

	if(dist / 2 >= r) {
		// if dist is greater than or equal to the radius, just return the
		// center of the two points as the circle origin.
		return (plot_point_t){p3.x,
							  p3.y};
	}

	// to be honest, I don't entierly get it but hey, what works works so no
	// need to question it.
	return (plot_point_t){
		p3.x + sqrt(r * r - (dist / 2) * (dist / 2)) * (p1.y - p2.y) / dist,
		p3.y + sqrt(r * r - (dist / 2) * (dist / 2)) * (p2.x - p1.x) / dist};
}

double calc_circ_radius(plot_point_t p1, plot_point_t p2, plot_point_t p3) {
	// p4 is calculated to be the mid-point between p1 and p2.
	plot_point_t p4 = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};

	double base	  = calc_dist(p2, p1);
	double height = calc_dist(p4, p3);

	double area = (base * height) / 2;

	// again, no idea what's happening but it also works.
	return calc_dist(p1, p2) * calc_dist(p2, p3) * calc_dist(p3, p1) /
		   (area * 4);
}

double calc_quadrant(double angle, double x, double y) {
	if(x >= 0 && y >= 0) { // Qaudrant 0, dont do anything.
	}
	else if(x <= 0 && y >= 0) { // Qaudrant II, 180 - theta
		angle = M_PI - angle;
	}
	else if(x <= 0 && y <= 0) { // Qaudrant III, theta - 180
		angle = angle - M_PI;
	}
	else if(x >= 0 && y <= 0) { // Qaudrant IV, 360 - theta
		angle = 2 * M_PI - angle;
	}
	else { // x and y are at zero and I'm not really sure what that means.
           angle = 0;
	}

	return angle;
}

int plotbot_set_pos(double x, double y, double rot) {
	return -1;
}

int plotbot_drive(double right, double left) {
	return -1;
}
int plotbot_drive_to(double x, double y, double radius) {
	return -1;
}

int plotbot_rotate(double speed) {
	return -1;
}
int plotbot_rotate_to(double angle) {
	return -1;
}