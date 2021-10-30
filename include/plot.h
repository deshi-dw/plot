#ifndef PLOT_H
#define PLOT_H

typedef struct plot_point_t		plot_point_t;
typedef struct plot_path_t		plot_path_t;
typedef struct plot_path_part_t plot_path_part_t;

typedef struct plotbot_t plotbot_t;
typedef struct plot_t	 plot_t;

struct plot_point_t {
	double x;
	double y;
};

struct plot_path_part_t {
	plot_point_t start;
	plot_point_t mid;
	plot_point_t end;
	plot_point_t origin;

	double radius;

	double start_angle;
	double end_angle;
	double delta_angle;
};

struct plot_path_t {
	plot_point_t* points;
	plot_point_t* radii; // radii is the plural of radius by the way.
	int			  point_count;
	int			  size;
};

struct plotbot_t {
	double x;
	double y;
	double rot;

	double width;
	double height;

	// movement calculation stuff
	double wheel_radius;
	double axel_width;
};

struct plot_t {
	double width;
	double height;

	plot_point_t origin;

#define PLOT_MAX_PATHS 256
	plot_path_t	  paths[PLOT_MAX_PATHS];
	const char*	  path_names[PLOT_MAX_PATHS];
	unsigned long path_hashes[PLOT_MAX_PATHS];

	int sel;
};

plot_t* plot_new(double width, double height);
int		plot_load_img(char* path_name);

int plot_path_add(char* name);
int plot_path_del(char* name);
int plot_path_sel(char* name);

int plot_point_add(double x, double y);
int plot_point_del(int index);
int plot_point_get_index(double x, double y, double radius);
int plot_point_set(int index, double x, double y);
int plot_point_set_radius(int index, double x, double y);

int plot_export(char* buffer, int buffer_size);
int plot_import(char* buffer, int buffer_size);

double plot_width();
double plot_height();

double plot_top();
double plot_bottom();
double plot_right();
double plot_left();

plot_point_t plot_origin();
plot_point_t plot_coord(double x, double y);
double		 plot_x(double x);
double		 plot_y(double y);

plot_path_part_t calc_path_part(plot_point_t start, plot_point_t mid,
								plot_point_t end, double radius);

plot_point_t calc_point_project(plot_point_t p1, plot_point_t p2);

/**
 * @brief calculates the distance between two points.
 *
 * @param p1 the first point
 * @param p2 the second point
 * @return double the distance between the two.
 */
double calc_dist(plot_point_t p1, plot_point_t p2);

double calc_map(double x, double in_min, double in_max, double out_max,
				double out_min);

/**
 * @brief calculates the origin of a circle given two points and a radius.
 *
 * @param p1 the first point
 * @param p2 the second point
 * @param r the circle radius
 * @return plot_point_t the origin of the circle.
 */
plot_point_t calc_circ_center(plot_point_t p1, plot_point_t p2, double r);

/**
 * @brief calculates a circle radius given three points on the circumfrence of
 * the circle. Note that not all possible point combination will return a valid
 * radius.
 *
 * @param p1 the first point
 * @param p2 the second point
 * @param p3 the third point
 * @return double the radius of the circle.
 */
double calc_circ_radius(plot_point_t p1, plot_point_t p2, plot_point_t p3);

/**
 * @brief takes a point and an angle local to one quadrant and calculates the
 * "global" angle of that point.
 *
 * @param angle the local quadrant angle
 * @param x the x of a point in the angle's quadrant
 * @param y the y of a point in the angle's quadrant
 * @return double the global angle of the supplied angle.
 */
double calc_quadrant(double angle, double x, double y);

int plotbot_set_pos(double x, double y, double rot);

int plotbot_drive(double right, double left);
int plotbot_drive_to(double x, double y, double radius);

int plotbot_rotate(double speed);
int plotbot_rotate_to(double angle);

#endif
