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

		if(IsKeyDown(KEY_R)) {
			bot = bot_init(0, 0, bot->axel_width, bot->wheel_diameter);
		}

		bot_arcade(-steer_x, steer_y, 1.0);
		bot_tick(GetFrameTime());

		// cam.offset.x = bot->x;
		// cam.offset.y = bot->y;
		printf("bot(%.2f,%.2f  %.2f)  steer(%.2f,%.2f)  speed(%.2f,%.2f)  "
		       "cam(%.2f,%.2f)   dt=%.2f\n",
		       bot->x, bot->y, bot->rot, steer_x, steer_y, bot->speed_r,
		       bot->speed_l, cam.offset.x, cam.offset.y, GetFrameTime());
		printf("  pps(%.2f,%.2f) step=%.2f\n",
		       (BOT_MAX_RPM / 60.0) * bot->speed_r * 128 * GetFrameTime(),
		       (BOT_MAX_RPM / 60.0) * bot->speed_l * 128 * GetFrameTime(),
		       M_PI * bot->wheel_diameter / 128);

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
			}
		}

		EndMode2D();
		EndDrawing();
	}

	CloseWindow();

	return 0;
}