#ifndef BOT_H
#define BOT_H

typedef struct bot_t bot_t;

#define BOT_MAX_RPM 250

struct bot_t {
	double x;
	double y;
	double rot;

	double width;
	double height;

	double wheel_diameter;
	double axel_width;

	double speed_r;
	double speed_l;

	double gear_ratio_r;
	double gear_ratio_l;
};

bot_t* bot_init(double x, double y, double axel_width, double wheel_diameter);

int bot_tick(double dt);

int bot_tank(double r, double l);
int bot_arcade(double turn, double speed, double range);

#endif