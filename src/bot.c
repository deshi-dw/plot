#include "bot.h"

#include <corecrt_math.h>
#include <corecrt_math_defines.h>

#include "plot.h"

static bot_t bot;

bot_t* bot_init(double x, double y, double axel_width, double wheel_diameter) {
	bot.x = x;
	bot.y = y;

	bot.axel_width     = axel_width;
	bot.wheel_diameter = wheel_diameter;

	bot.width  = bot.axel_width;
	bot.height = bot.axel_width * 1.75;

	bot.gear_ratio_r = 15.0 / 45.0;
	bot.gear_ratio_l = 15.0 / 45.0;

	return &bot;
}

int bot_tick(double dt) {
	// dt (delta time) is in seconds.

	// calculate the pulses per second by converting the max rpm into seconds
	// and multiplying it by the motor speed. Assuming the resolution of the
	// encoder (ie samples per rotation) is 128, multiply the rps by the number
	// of samples.
	double pps_r = (BOT_MAX_RPM / 60.0) * bot.speed_r * 128 * dt;
	double pps_l = (BOT_MAX_RPM / 60.0) * bot.speed_l * 128 * dt;

	// get how much distance is covered in one wheel spin.
	// remember divide by the resolution so we get the step size of just a
	// single sample.
	// I know I could just not multiply pps_r and pps_l by 128 but this is more
	// so for "accurate emulation" in the loosest use of the term than being
	// efficient.
	double step = M_PI * bot.wheel_diameter / 128;

	plot_vec3_t newpos = calc_skid_transform(bot.x, bot.y, bot.rot, pps_r,
	                                         pps_l, step, bot.axel_width);

	bot.x   = newpos.x;
	bot.y   = newpos.y;
	bot.rot = newpos.z;

	return 0;
}

int bot_tank(double r, double l) {
	bot.speed_r = r;
	bot.speed_l = l;

	return 0;
}

int bot_arcade(double turn, double speed, double range) {
	// clamp turn and speed values between -range and range.

	double x = turn > range ? range : turn < -range ? -range : turn;
	double y = speed > range ? range : speed < -range ? -range : speed;

	// get absolute x and y values.
	double abs_x = x < 0 ? -x : x;
	double abs_y = y < 0 ? -y : y;

	// the formula used here was gotten from the following article:
	// https://home.kendra.com/mauser/Joystick.html
	double v = (range - abs_x) * (y / range) + y;
	double w = (range - abs_y) * (x / range) + x;

	double right = (v + w) / 2;
	double left  = (v - w) / 2;

	bot_tank(right, left);

	return 0;
}