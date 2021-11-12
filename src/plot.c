#include "plot.h"

#include <corecrt_malloc.h>
#include <corecrt_math.h>
#include <corecrt_memcpy_s.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <corecrt_math_defines.h>

static plot_t plot = {0};

// Hash function from http://www.cse.yorku.ca/~oz/hash.html
static unsigned long hash(char* str) {
	unsigned long hash = 5381;
	int           c;

	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

plot_t* plot_new(double width, double height) {
	memset(&plot, 0, sizeof(plot));

	plot.width  = width;
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

		plot.path_names[i]  = name;
		plot.path_hashes[i] = hash(name);
		memset(&plot.paths[i], 0, sizeof(plot.paths[i]));

		plot.paths[i].size   = 4;
		plot.paths[i].points = malloc(sizeof(plot_vec2_t) * plot.paths[i].size);
		plot.paths[i].radii  = malloc(sizeof(plot_vec2_t) * plot.paths[i].size);
		plot.paths[i].parts =
		    calloc(sizeof(plot_path_part_t), plot.paths[i].size);

		return 0;
	}

	return -1;
}

int plot_path_del(char* name) {
	unsigned long h = hash(name);

	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(plot.path_hashes[i] == h) {
			plot.path_names[i]  = NULL;
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

int plot_path_calc() {
	for(int i = 0; i < plot.paths[plot.sel].point_count; i += 2) {
		if(i + 3 > plot.paths[plot.sel].point_count) {
			break;
		}

		// FIXME if start == end, mid will go delete itself.
		plot_vec2_t start = plot.paths[plot.sel].points[i];
		plot_vec2_t mid   = plot.paths[plot.sel].points[i + 1];
		plot_vec2_t end   = plot.paths[plot.sel].points[i + 2];

		double r = calc_dist(start, end) / 2;

		double           real_radius = 0.0;
		plot_path_part_t part;

		// do initial part calculation with baseline radius.
		part        = calc_path_part(start, mid, end, r);
		real_radius = calc_circ_radius(start, end, part.mid);

		// calculate part again but this time with a "real radius".
		if(real_radius != NAN && real_radius != INFINITY) {
			// roud real radius to get rid of small rounding errors that
			// mess up calc_circ_center.

			// calc_circ_center uses the square root of radius^2 -
			// calc_dist(start, end)^2 amoung some other calculations to
			// figure out the origin. if the radius isn't greater than
			// or equal to the calc_dist, we get a NAN output which
			// breaks everything.

			// for example my radius was 299.99999999999994 when it was
			// supposed to be 300 and that was enough to break it. 1
			// million point precision should be good enough for
			// literally everything short of rocketship code so I think
			// we're fine.
			real_radius = round(real_radius * 1000000.0) / 1000000.0;
			// real_radius = r + 100;
			part = calc_path_part(start, part.mid, end, real_radius);
		}

		plot.paths[plot.sel].points[i + 1] = part.mid;

		plot.paths[plot.sel].parts[i / 2] = part;
	}

	return 0;
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
		                       plot.paths[plot.sel].size * sizeof(plot_vec2_t),
		                       new_size * sizeof(plot_vec2_t));

		// now do the same but for the radii. note that radii has half the size
		// because there is only 1 radii point for every two points.
		plot.paths[plot.sel].radii =
		    plot_buffer_resize(plot.paths[plot.sel].radii,
		                       plot.paths[plot.sel].size * sizeof(plot_vec2_t),
		                       new_size * sizeof(plot_vec2_t) / 2);

		// resize the parts buffer to be half of the size of the points buffer.
		plot.paths[plot.sel].parts = plot_buffer_resize(
		    plot.paths[plot.sel].parts,
		    plot.paths[plot.sel].size * sizeof(plot_path_part_t),
		    new_size * sizeof(plot_path_part_t) / 2);

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

// FIXME deleting admittedly works better than I thought it would but it's still
// broke. Adding new points after delteting points is odd.
int plot_point_del(int index) {
	memcpy(plot.paths[plot.sel].points + index - 1,
	       plot.paths[plot.sel].points + index,
	       plot.paths[plot.sel].point_count - index);

	plot.paths[plot.sel].point_count--;

	return 0;
}

int plot_point_get_index(double x, double y, double radius) {
	for(int i = 0; i < plot.paths[plot.sel].point_count; i++) {
		if(calc_dist(plot.paths[plot.sel].points[i], (plot_vec2_t){x, y}) <=
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

plot_vec2_t plot_origin() {
	return plot.origin;
}

plot_vec2_t plot_coord(double x, double y) {
	return (plot_vec2_t){plot_left() + x, plot_bottom() + y};
}

double plot_x(double x) {
	return plot_left() + x;
}

double plot_y(double y) {
	return plot_top() - (plot_bottom() + y);
}

// TODO deal with cases where things are zero. Or more accuratly, figure out all
// the restrictions with the formulas I used.

plot_path_part_t calc_path_part(plot_vec2_t start, plot_vec2_t mid,
                                plot_vec2_t end, double radius) {
	plot_path_part_t part = {0};

	plot_vec2_t real_middle =
	    (plot_vec2_t){(start.x + end.x) / 2, (start.y + end.y) / 2};

	// double		 r	= calc_dist(start, end) / 2;
	double      r  = radius;
	plot_vec2_t co = calc_circ_center(start, end, r);

	// reflect the circle depending on the middle point.
	// n = normal,  f = flipped
	// p1 = start, p2 = end
	/* p1 \                     /  p2       p2 \                     / p1
	 *     \                   /                \                   /
	 *   n  \  f           f  /  n            f  \  n           n  /  f
	 *       \               /                    \               /
	 *        \ p2       p1 /                      \ p1       p2 /
	 */

	// get the line of start and end point using the y = mx + b forumula and
	// then figure out which side the mid point is on.

	//     p2.y - p1.y
	// m = -----------    slope
	//     p2.x - p1.x
	double m = (end.y - start.y) / (end.x / start.x);

	// b = y - mx
	double b = real_middle.y - m * real_middle.x;

	// mx + b - y > 0
	double side = m * mid.x + b - mid.y;

	int flip = 0;
	if(side < 0) {
		flip = 1;
	}

	// reflect the circle origin. Do this when flipping the sides.
	if(flip) {
		plot_vec2_t rco = calc_reflect_p3(start, end, co);
		if(! isnan(rco.x) && ! isnan(rco.y)) {
			co = rco;
		}
	}

	// double o1 = start.y > co.y ? start.y - co.y : co.y - start.y;
	// double o2 = end.y > co.y ? end.y - co.y : co.y - end.y;

	double o1 = co.y - start.y;
	double o2 = co.y - end.y;

	// make opposite 1 and 2 absolute. This is to make the below calculation
	// always work.
	o1 = o1 < 0 ? -o1 : o1;
	o2 = o2 < 0 ? -o2 : o2;

	// make it so that o1 and o2 are never greater than the radius or the asin
	// function will return NAN.
	o1 = o1 > r ? r : o1;
	o2 = o2 > r ? r : o2;

	double a1 = calc_quadrant(asin(o1 / r), start.x - co.x, start.y - co.y);
	double a2 = calc_quadrant(asin(o2 / r), end.x - co.x, end.y - co.y);

	// make angle 1 and 2 unsigned to make maths work.
	a1 = a1 < 0 ? a1 + 2 * M_PI : a1;
	a2 = a2 < 0 ? a2 + 2 * M_PI : a2;

	// delta angle
	// gets the signed distance between angle 1 and angle 2.
	double da = atan2(sin(a1 - a2), cos(a1 - a2));

	part.start_angle = a1;
	part.end_angle   = a2;

	part.delta_angle = da;

	part.radius = r;
	part.origin = co;

	part.start = start;
	part.end   = end;

	// get a projection of the mid point onto the line between the start and end
	// points.
	plot_vec2_t proj = calc_point_project(
	    co,
	    (plot_vec2_t){cos(a1 - da / 2) * r + co.x, sin(a1 - da / 2) * r + co.y},
	    mid);

	// restrict the mid point to inside the circle.
	if(calc_dist(proj, co) > r) {
		proj.x = cos(a1 - da / 2) * r + co.x;
		proj.y = sin(a1 - da / 2) * r + co.y;
	}

	part.mid.x = round(proj.x * 1000000) / 1000000;
	part.mid.y = round(proj.y * 1000000) / 1000000;

	return part;
}

plot_vec2_t calc_point_project(plot_vec2_t p1, plot_vec2_t p2, plot_vec2_t p3) {
	// get A vector
	double Ax = p3.x - p1.x;
	double Ay = p3.y - p1.y;

	// get B vector
	double Bx = p2.x - p1.x;
	double By = p2.y - p1.y;

	// normalize B vector
	double Bmag = sqrt(Bx * Bx + By * By);
	Bx          = Bx / Bmag;
	By          = By / Bmag;

	// get the dot product of the A and B vector.
	double s = Ax * Bx + Ay * By;

	// convert vectors back into a point.
	return (plot_vec2_t){Bx * s + p1.x, By * s + p1.y};
}

plot_vec2_t calc_reflect_p3(plot_vec2_t p1, plot_vec2_t p2, plot_vec2_t p3) {
	// get the point projected onto a line (intersect point)
	plot_vec2_t i = calc_point_project(p1, p2, p3);

	// get the reflection of p3 along the line using the intersection point.
	return (plot_vec2_t){i.x - p3.x + i.x, i.y - p3.y + i.y};
}

double calc_dist(plot_vec2_t p1, plot_vec2_t p2) {
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

double calc_map(double x, double in_min, double in_max, double out_max,
                double out_min) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

plot_vec2_t calc_circ_center(plot_vec2_t p1, plot_vec2_t p2, double r) {
	// get the center as p3
	plot_vec2_t p3 = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};

	double dist = calc_dist(p1, p2);

	if(dist / 2 >= r) {
		// if dist is greater than or equal to the radius, just return the
		// center of the two points as the circle origin.
		return (plot_vec2_t){p3.x, p3.y};
	}

	// to be honest, I don't entierly get it but hey, what works works so no
	// need to question it.
	return (plot_vec2_t){
	    p3.x + sqrt(r * r - (dist / 2) * (dist / 2)) * (p1.y - p2.y) / dist,
	    p3.y + sqrt(r * r - (dist / 2) * (dist / 2)) * (p2.x - p1.x) / dist};
}

double calc_circ_radius(plot_vec2_t p1, plot_vec2_t p2, plot_vec2_t p3) {
	// p4 is calculated to be the mid-point between p1 and p2.
	plot_vec2_t p4 = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};

	double base   = calc_dist(p2, p1);
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

// calculation help:
// http://www.cs.columbia.edu/~allen/F17/NOTES/icckinematics.pdf
// https://www8.cs.umu.se/kurser/5DV122/HT13/material/Hellstrom-ForwardKinematics.pdf
// http://robotsforroboticists.com/drive-kinematics/

// note: this doesn't account for slippage which is more common than one would
// think.
plot_vec3_t calc_skid_transform(double x, double y, double rot, double count_r,
                                double count_l, double step, double width) {
	if(count_r == 0 && count_l == 0) {
		return (plot_vec3_t){x, y, rot};
	}

	if(count_r == count_l) {
		// return (plot_vec3_t){x + cos(rot) * (count_r * step),
		// 					 y + sin(rot) * (count_r * step), rot};
		// TODO stop doing this.
		count_l += 0.001;
	}
	else if(count_r == -count_l) {
		// TODO rotate in place around the middle.
	}
	else if(count_r == 0) {
		// TODO rotate around the right wheel with radius = l / 2
	}
	else if(count_l == 0) {
		// TODO rotate around the left wheel with radius = l / 2
	}

	double radius    = width / 2.0 * (count_l + count_r) / (count_r - count_l);
	double rot_delta = (count_r - count_l) * step / width;

	plot_vec2_t origin =
	    (plot_vec2_t){x - radius * sin(rot), y + radius * cos(rot)};

	// rotation matrix: roatate by rotation delta around the z axis. (the up
	// one)

	// [ RMx1y1   RMx2y1   RMx3y1 ]
	// [                          ]
	// [ RMx1y2   RMx2y2   RMx3y2 ]
	// [                          ]
	// [ RMx1y3   RMx2y3   RMx3y3 ]

	// [ cos(rot_delta)   -sin(rot_delta)   0  ]
	// [                                       ]
	// [ sin(rot_delta)   cos(rot_delta)    0  ]
	// [                                       ]
	// [       0                0           1  ]

	double RMx1y1 = cos(rot_delta);
	double RMx1y2 = sin(rot_delta);
	double RMx1y3 = 0.0;

	double RMx2y1 = -sin(rot_delta);
	double RMx2y2 = cos(rot_delta);
	double RMx2y3 = 0.0;

	double RMx3y1 = 0.0;
	double RMx3y2 = 0.0;
	double RMx3y3 = 1.0;

	// traslate to zero matrix: translates the origin to zero.

	// [ TZx1y1 ]
	// [        ]
	// [ TZx1y2 ]
	// [        ]
	// [ TZx1y3 ]

	// [ x - origin.x ]
	// [              ]
	// [ y - origin.y ]
	// [              ]
	// [      rot     ]

	double TZx1y1 = x - origin.x;
	double TZx1y2 = y - origin.y;
	double TZx1y3 = rot;

	// translate back matrix: translates back to the original position. (undoes)
	// translate to zero matrix.

	// [ TBx1y1 ]
	// [        ]
	// [ TBx1y2 ]
	// [        ]
	// [ TBx1y3 ]

	// [   origin.x  ]
	// [             ]
	// [   origin.y  ]
	// [             ]
	// [  rot_delta  ]

	double TBx1y1 = origin.x;
	double TBx1y2 = origin.y;
	double TBx1y3 = rot_delta;

	// traslation calculation is (RM * TZ) + TB
	// I really need to look into matrix transformations because I don't really
	// have any idea why this equation works.

	// What I THINK it does is place the robot at (0,0), apply the new rotation
	// and then project the robot's position to a new position based on it's new
	// rotation.

	// final translation: the new position and rotation of our robot.
	// multiply RM and TZ matrices.
	double FTx1y1 = RMx1y1 * TZx1y1 + RMx2y1 * TZx1y2 + RMx3y1 * TZx1y3;
	double FTx1y2 = RMx1y2 * TZx1y1 + RMx2y2 * TZx1y2 + RMx3y2 * TZx1y3;
	double FTx1y3 = RMx1y3 * TZx1y1 + RMx2y3 * TZx1y2 + RMx3y3 * TZx1y3;

	// add multiplied RM and TZ matrix with TB matrix.
	FTx1y1 += TBx1y1;
	FTx1y2 += TBx1y2;
	FTx1y3 += TBx1y3;

	if(isnan(FTx1y1) | isnan(FTx1y2) | isnan(FTx1y3)) {
		puts("calc_skid_transform error");
		printf("FT=[%.2f, %.2f, %.2f]\n", FTx1y1, FTx1y2, FTx1y3);
		exit(-1);
	}

	return (plot_vec3_t){FTx1y1, FTx1y2, FTx1y3};
}

// double calc_target(double x, double y, double ICCx, double ICCy) {}

double calc_rot_delta(double x, double y, double t_x, double t_y, double ICCx,
                      double ICCy) {
	// We can calulate the sin(w) and cos(w) from a series of linear equations
	// which we have from the calc_skid_transform function.

	// x` = (x - ICCx)cos(w) - (y - ICCy)sin(w) + ICCx
	// y` = (x - ICCx)sin(w) + (y - ICCy)cos(w) + ICCy
	// where 	x`   = target position x		x = robot position x
	//			y`   = target position y		y = robot position y
	//			ICCx = circle origin x			w = rotation delta
	//			ICCy = circle origin y

	// To get from these equations to where we need was a nightmare that I don't
	// want to put anyone else through so I will spare you the details. Here is
	// the final equations:

	// sin(w) = (x - ICCx)cos(w) + ICCx - x`
	//			----------------------------
	//			         y - ICCy
	//
	//          (x - ICCx)ICCx - (x - ICCx)x`     ICCy - y`
	//			-----------------------------  -  ----------
	//                 -(y - ICCy)^2               y - ICCy
	// cos(w) = --------------------------------------------
	//                       (x - ICCx)^2
	//                       ------------  +  1
	//                       (y - ICCy)^2

	// I didn't try very hard to simplify this so I'm sure it can be done but it
	// just wont be me who does it.

	double cosw =
	    (((x - ICCx) * ICCx - (x - ICCx) * t_x) / (-(y - ICCy) * (y - ICCy)) -
	     (ICCy - t_y) / (y - ICCy)) /
	    ((x - ICCx) * (x - ICCx) / ((y - ICCy) * (y - ICCy)) + 1);

	double sinw = ((x - ICCx) * cosw + ICCx - t_x) / (y - ICCy);

	double w_1 = calc_x_of_sinx(sinw);
	double w_2 = calc_x_of_cosx(sinw);

	// TODO remove this when calc_x_of_cosx can properly calculate negative x
	// values.
	if(w_1 < 0) {
		w_2 *= -1;
	}

	return w_2;
}

double calc_x_of_sinx(double sinx) {
	// Using sine estimation x estimation, and good ol' Newton's method, we can
	// calculate x. Note that this works better for "small angles". We can
	// estimate the value of x as sin(x) because they are roughly equal when
	// small enough.

	// We can then use the equation x_n+1 = f(x_n) / f`(x_n) which requires the
	// "function derivative" or the rate of change of the function. This can be
	// retrived by using a list of know rules.

	// Our sine estimation equation is x - x^3 / 3! but can be rearanged to be
	// -x^3 + 6x - 6sin(x) = 0

	// our derivative is then 3 sin(x)^2 - 6
	// found through https://www.rapidtables.com/math/calculus/derivative.html

	// Plugging those functions into Newton's Method equation becomes
	// x_n+1 = sin(x) - sin(x)^3 / ( 3sin(x)^2 - 6 )
	// we use sin(x) here because for "small angles", it is a close estimation
	// for x. https://brilliant.org/wiki/small-angle-approximation/

	// the restrictions on x are  -1 <= x <= 1

	// The accuracy of calc_x_of_sinx isn't as good as calc_x_of_cosx because we
	// need to estimate x. For small x values this differnce is negligable but
	// it becomes a problem near the upper and lower limits of -1 and 1.

	return sinx - (sinx * sinx * sinx) / (3 * sinx * sinx - 6);
}

double calc_x_of_cosx(double cosx) {
	// Getting x of cos(x) is worlds similer than sin(x) because it works with
	// even exponents. To estimate cos(x) use the equation
	// 1 - x^2 / 2 + x^4 / 4!
	// which can be rearanged to
	// x^4 - 12x^2 + 24 - 24cos(x) = 0
	// which is a quadratic formula if u = x^2 and we write it as
	// u^2 - 12u + 24 - 24cos(x)
	// use the quadratic formula to solve, substitute back in x^2 and get
	// -+ sqrt(6 - 2sqrt(3 + 6cos(x)))

	// TODO make calc_x_of_cosx figure out the sign of x.
	// In it's current incarnation, x will always be returned as positive. I
	// don't yet have a way to fix this.

	// the restrictions on x are  -1 <= x <= 1

	// In terms of accuracy, calc_x_of_cosx always better because we don't need
	// to estimate x. The only loss in accuracy comes from the cos() function
	// estimation.

	return sqrt(6 - 2 * sqrt(3 + 6 * cosx));
}

plot_vec2_t calc_skid_velocities(plot_vec2_t origin, double rot_delta,
                                 double width, double radius) {
	double v_r = rot_delta * (radius + width / 2);
	double v_l = rot_delta * (radius - width / 2);

	return (plot_vec2_t){v_r, v_l};
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