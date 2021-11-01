#include <corecrt_math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <corecrt_math_defines.h>

#include "raylib.h"

#include "plot.h"

struct {
	int sel;
	int is_sel;
} cursor = {0};

int main(void) {
	const int screenWidth  = 800;
	const int screenHeight = 450;

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "plot");
	SetTargetFPS(30);

	// x / (x / 10)

	plot_t* plot = plot_new(800, 450);
	plot_path_add("new_path_1");
	plot_path_sel("new_path_1");

	plot_point_add(100, 50);
	plot_point_add(400, 150);
	plot_point_add(700, 50);
	// plot_point_add(300, 125);
	// plot_point_add(720, 420);
	// plot_point_add(630, 220);

	Camera2D cam   = {(Vector2){0.0f, 0.0f}, (Vector2){0.0f, 0.0f}, 0.0f, 1.0f};
	Vector2	 mouse = {0};

	int is_flipped = 0;

	while(! WindowShouldClose()) {
		float cam_sensetivity = 10.0f;
		if(IsKeyDown(KEY_RIGHT)) {
			cam.offset.x -= cam_sensetivity;
		}
		else if(IsKeyDown(KEY_LEFT)) {
			cam.offset.x += cam_sensetivity;
		}

		if(IsKeyDown(KEY_UP)) {
			cam.offset.y += cam_sensetivity;
		}
		else if(IsKeyDown(KEY_DOWN)) {
			cam.offset.y -= cam_sensetivity;
		}

		if(IsKeyPressed(KEY_F)) {
			is_flipped = ! is_flipped;
		}

		mouse = GetScreenToWorld2D(GetMousePosition(), cam);

		if(IsMouseButtonPressed(0)) {
			cursor.sel =
				plot_point_get_index(plot_x(mouse.x), plot_y(mouse.y), 4.0);
			if(cursor.sel >= 0) {
				cursor.is_sel = 1;
			}
			else {
				cursor.is_sel = 0;
			}
		}
		else if(IsMouseButtonReleased(0)) {
			cursor.sel	  = -1;
			cursor.is_sel = 0;
		}

		if(cursor.is_sel) {
			plot_point_set(cursor.sel, plot_x(mouse.x), plot_y(mouse.y));
		}

		BeginDrawing();
		ClearBackground(WHITE);

		BeginMode2D(cam);

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

			for(int j = 0; j < plot->paths[i].point_count; j += 3) {
				plot_point_t start = plot->paths[i].points[j];
				plot_point_t mid   = plot->paths[i].points[j + 1];
				plot_point_t end   = plot->paths[i].points[j + 2];

				/*plot_point_t p4 = calc_point_project(start, end, mid);

				DrawCircle(plot_x(start.x), plot_y(start.y), 3.0f, BLACK);
				DrawCircle(plot_x(end.x), plot_y(end.y), 3.0f, BLACK);
				DrawCircle(plot_x(mid.x), plot_y(mid.y), 3.0f, BLACK);

				DrawCircle(plot_x(p4.x), plot_y(p4.y), 3.0f, RED);

				DrawLine(plot_x(start.x), plot_y(start.y), plot_x(end.x),
				plot_y(end.y), BLACK);

				continue;*/

				double r = calc_dist(start, end) / 2;

				plot_point_t real_middle = (plot_point_t){
					(start.x + end.x) / 2, (start.y + end.y) / 2};

				DrawCircle(plot_x(real_middle.x), plot_y(real_middle.y), 3.0f,
						   BLUE);

				double			 real_radius = 0.0;
				plot_path_part_t part;

				// do initial part calculation with baseline radius.
				part		= calc_path_part(start, mid, end, r);
				real_radius = calc_circ_radius(start, end, part.mid);

				// calculate part again but this time with a "real radius".
				if(real_radius != NAN && real_radius != INFINITY) {
					// roud real radius to get rid of small rounding errors that
					// mess up calc_circ_center.

					// calc_circ_center uses the square root of radius^2 -
					// calc_dist(start, end)^2 amoung some other calculations to
					// figure out the origin. if the radius isn't greater than
					// the calc_dist, we get a NAN output which breaks
					// everything.

					// for example my radius was 299.99999999999994 when it was
					// supposed to be 300 and that was enough to break it. 1
					// million point precision should be good enough for
					// literally everything short of rocketship code so I think
					// we're fine.
					real_radius = round(real_radius * 1000000.0) / 1000000.0;
					// real_radius = r + 100;
					part = calc_path_part(start, part.mid, end, real_radius);
				}

				DrawLine(plot_x(part.origin.x), plot_y(part.origin.y),
						 plot_x(cos(part.start_angle - part.delta_angle / 2) *
									part.radius +
								part.origin.x),
						 plot_y(sin(part.start_angle - part.delta_angle / 2) *
									part.radius +
								part.origin.y),
						 PINK);

				char text[256];
#define TO_DEG (180 / M_PI)

				double Mx = mid.x - real_middle.x;
				double My = mid.y - real_middle.y;

				sprintf(text,
						"start=%.2f, end=%.2f, delta=%.2f  flipped=%d, flip "
						"dist=%.2f",
						part.start_angle * TO_DEG, part.end_angle * TO_DEG,
						part.delta_angle * TO_DEG, is_flipped,
						mid.x * Mx + mid.y * My);
				DrawText(text, 10, 10, 16, BLACK);

				sprintf(text, "mid=(%.2f, %.2f)  real_mid=(%.2f, %.2f)", mid.x,
						mid.y, real_middle.x, real_middle.y);
				DrawText(text, 10, 24, 16, BLACK);

				sprintf(text, "start=(%.2f, %.2f)  end=(%.2f, %.2f)", start.x,
						start.y, end.x, end.y);
				DrawText(text, 10, 38, 16, BLACK);

				DrawCircle(plot_x(part.mid.x), plot_y(part.mid.y), 3.0f, RED);
				plot->paths[i].points[j + 1] = part.mid;

				DrawLine(plot_x(part.origin.x - part.radius),
						 plot_y(part.origin.y),
						 plot_x(part.origin.x + part.radius),
						 plot_y(part.origin.y), RED);

				DrawLine(plot_x(part.origin.x),
						 plot_y(part.origin.y - part.radius),
						 plot_x(part.origin.x),
						 plot_y(part.origin.y + part.radius), RED);

				DrawCircleLines(plot_x(part.origin.x), plot_y(part.origin.y),
								part.radius, RED);

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