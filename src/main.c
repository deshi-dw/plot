#include <corecrt_math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <corecrt_math_defines.h>

#include "raylib.h"

#include "plot.h"
#include "bot.h"
#include "ui.h"

struct {
	int sel;
	int is_moving;
} cursor = {0};

int main(void) {
	const int screenWidth  = 800;
	const int screenHeight = 450;

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "plot");
	SetTargetFPS(30);

	plot_t* plot = plot_new(800, 450);
	plot_path_add("new_path_1");
	plot_path_sel("new_path_1");

	plot_point_add(100, 50);
	plot_point_add(400, 150);
	plot_point_add(700, 50);
	// plot_point_add(300, 125);
	// plot_point_add(720, 420);
	// plot_point_add(630, 220);
	// plot_point_add(630, 420);

	bot_t* bot = bot_init(10, 10, 45.0, 6.0);

	Camera2D cam   = {(Vector2){0.0f, 0.0f}, (Vector2){0.0f, 0.0f}, 0.0f, 1.0f};
	Vector2  mouse = {0};

	ui_init();

	ui_btn_t btn_file    = {0};
	btn_file.rect.x      = 0;
	btn_file.rect.y      = 0;
	btn_file.rect.width  = 30;
	btn_file.rect.height = 12;
	btn_file.text        = "file";

	ui_btn_t btn_edit    = {0};
	btn_edit.rect.x      = 30;
	btn_edit.rect.y      = 0;
	btn_edit.rect.width  = 30;
	btn_edit.rect.height = 12;
	btn_edit.text        = "edit";

	double steer_x = 0.0;
	double steer_y = 0.0;

	int cam_off_x = 0;
	int cam_off_y = 0;

	int is_bot_following = 0;
	while(! WindowShouldClose()) {
		double sensetivity = 0.25;
		double stop_speed  = 0.5;

		if(IsKeyDown(KEY_RIGHT)) {
			steer_x = steer_x + sensetivity > 1.0 ? 1.0 : steer_x + sensetivity;
		}
		else if(IsKeyDown(KEY_LEFT)) {
			steer_x =
			    steer_x - sensetivity < -1.0 ? -1.0 : steer_x - sensetivity;
		}
		else {
			// move steer_x to zero. If the hundredth place rounds to zero, set
			// steer_x to zero.
			steer_x = round(steer_x * stop_speed * 100) == 0
			              ? 0
			              : steer_x * stop_speed;
		}

		if(IsKeyDown(KEY_UP)) {
			steer_y = steer_y + sensetivity > 1.0 ? 1.0 : steer_y + sensetivity;
		}
		else if(IsKeyDown(KEY_DOWN)) {
			steer_y =
			    steer_y - sensetivity < -1.0 ? -1.0 : steer_y - sensetivity;
		}
		else {
			// move steer_y to zero. If the hundredth place rounds to zero, set
			// steer_y to zero.
			steer_y = round(steer_y * stop_speed * 100) == 0
			              ? 0
			              : steer_y * stop_speed;
		}

		if(IsKeyReleased(KEY_R)) {
			bot      = bot_init(0, 0, bot->axel_width, bot->wheel_diameter);
			bot->rot = 0.0;
		}

		if(IsKeyPressed(KEY_F)) {
			bot = bot_init(plot->paths[plot->sel].points[0].x,
			               plot->paths[plot->sel].points[0].y, bot->axel_width,
			               bot->wheel_diameter);
			// TODO make bot initial rotation to be the start point angle + 90
			// degrees (I think that's right)
			bot->rot         = 0.0;
			is_bot_following = 1;
		}

		if(IsKeyReleased(KEY_F)) {
			is_bot_following = 0;
		}

		int cam_off_sen = 50;
		if(IsKeyReleased(KEY_W)) {
			cam_off_y += cam_off_sen;
		}
		else if(IsKeyReleased(KEY_S)) {
			cam_off_y -= cam_off_sen;
		}
		if(IsKeyReleased(KEY_D)) {
			cam_off_x += cam_off_sen;
		}
		else if(IsKeyPressed(KEY_A)) {
			cam_off_x -= cam_off_sen;
		}

		if(! is_bot_following) {
			bot_arcade(-steer_x, steer_y, 1.0);
		}
		bot_tick(GetFrameTime());

		// camera follows robot.
		cam.offset.x = -bot->x + GetScreenWidth() / 2 - cam_off_x;
		cam.offset.y = bot->y - GetScreenHeight() / 2 + cam_off_y;

		mouse = GetScreenToWorld2D(GetMousePosition(), cam);

		if(IsMouseButtonPressed(0)) {
			cursor.sel =
			    plot_point_get_index(plot_x(mouse.x), plot_y(mouse.y), 4.0);
			if(cursor.sel >= 0) {
				cursor.is_moving = 1;
			}
			else {
				cursor.is_moving = 0;
			}
		}
		else if(IsMouseButtonReleased(0)) {
			cursor.is_moving = 0;
		}

		if(cursor.is_moving) {
			plot_point_set(cursor.sel, plot_x(mouse.x), plot_y(mouse.y));
		}

		if(IsMouseButtonReleased(1)) {
			double      mx = plot_x(mouse.x);
			double      my = plot_y(mouse.y);
			plot_vec2_t prev =
			    plot->paths[plot->sel]
			        .points[plot->paths[plot->sel].point_count - 1];
			plot_point_add((prev.x + mx) / 2 + 1, (prev.y + my) / 2);
			plot_point_add(mx, my);
		}

		if(IsKeyDown(KEY_DELETE)) {
			if(cursor.sel != -1 &&
			   cursor.sel < plot->paths[plot->sel].point_count) {
				plot_point_del(cursor.sel);
			}
		}

		ui_tick();
		ui_btn_tick(&btn_file);
		ui_btn_tick(&btn_edit);

		BeginDrawing();
		ClearBackground(WHITE);

		ui_btn_draw(&btn_file);
		ui_btn_draw(&btn_edit);

		BeginMode2D(cam);

		// Draw robot
		DrawRectanglePro((Rectangle){plot_x(bot->x), plot_y(bot->y),
		                             bot->height, bot->width},
		                 (Vector2){bot->height / 2, bot->width / 2},
		                 -bot->rot * TO_DEG, DARKGREEN);

// draw plot borders
#define THICK 3.0f
		DrawLineEx((Vector2){plot_left(), plot_bottom()},
		           (Vector2){plot_left(), plot_top()}, THICK, BLACK);
		DrawLineEx((Vector2){plot_right(), plot_bottom()},
		           (Vector2){plot_right(), plot_top()}, THICK, BLACK);
		DrawLineEx((Vector2){plot_left(), plot_top()},
		           (Vector2){plot_right(), plot_top()}, THICK, BLACK);
		DrawLineEx((Vector2){plot_left(), plot_bottom()},
		           (Vector2){plot_right(), plot_bottom()}, THICK, BLACK);
#undef THICK

		// draw mouse coordinates
		char mousePosText[64];
		sprintf(mousePosText, "(%.0f,%.0f)", plot_x(mouse.x), plot_y(mouse.y));
		DrawText(mousePosText, mouse.x + 10, mouse.y, 11, BLACK);

		// draw points
		for(int i = 0; i < PLOT_MAX_PATHS; i++) {
			if(plot->paths[i].points == NULL) {
				continue;
			}

			// TODO support more than three points.
			for(int j = 0; j < plot->paths[i].point_count; j += 2) {
				if(j + 3 > plot->paths[i].point_count) {
					break;
				}

				plot_vec2_t start = plot->paths[i].points[j];
				plot_vec2_t mid   = plot->paths[i].points[j + 1];
				plot_vec2_t end   = plot->paths[i].points[j + 2];

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

				plot->paths[i].points[j + 1] = part.mid;

				int detail = 10;

				for(int k = 0; k < detail; k++) {
					double angle_part = part.delta_angle / detail * k;

					double px =
					    cos(part.start_angle - angle_part) * part.radius;
					double py =
					    sin(part.start_angle - angle_part) * part.radius;

					DrawCircle(plot_x(px + part.origin.x),
					           plot_y(py + part.origin.y), 2.0f, BLACK);
				}

				DrawCircle(plot_x(start.x), plot_y(start.y), 3.0f, BLACK);
				DrawCircle(plot_x(end.x), plot_y(end.y), 3.0f, BLACK);
				DrawCircle(plot_x(mid.x), plot_y(mid.y), 3.0f, BLACK);

				// TODO make the robot drive from the very start of the path to
				// the end of the path.

				// FIXME if add is too high, imediate crash. no idea why.
				double add = 10;
				double tx = cos(part.start_angle - add * TO_RAD) * part.radius +
				            part.origin.x;
				double ty = sin(part.start_angle - add * TO_RAD) * part.radius +
				            part.origin.y;

				DrawCircle(plot_x(tx), plot_y(ty), 6.0f, ORANGE);
				DrawLine(plot_x(part.origin.x), plot_y(part.origin.y),
				         plot_x(tx), plot_y(ty), ORANGE);

				if(i == plot->sel && is_bot_following) {
					// bad stuff, delete and replace
					if(bot->rot == 0) {
						bot->rot = part.start_angle + M_PI / 2;
					}

					double rot_delta = calc_rot_delta(
					    bot->x, bot->y, tx, ty, part.origin.x, part.origin.y);

					plot_vec2_t vels = calc_skid_velocities(
					    part.origin, rot_delta, bot->axel_width, part.radius);

					printf("ICC(%f,%f)  l=%f  r=%f  rd=%f  ", part.origin.x,
					       part.origin.y, bot->axel_width, part.radius,
					       rot_delta);
					printf("target(%f,%f)  ", tx, ty);
					printf("vels(%f,%f)", vels.x, vels.y);
					puts("");

					double mag = sqrt(vels.x * vels.x + vels.y * vels.y);
					vels.x     = vels.x / mag;
					vels.y     = vels.y / mag;

					bot_tank(vels.x, vels.y);
				}
			}
		}

		EndMode2D();
		EndDrawing();
	}

	CloseWindow();

	return 0;
}