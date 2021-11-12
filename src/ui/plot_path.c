#include "ui.h"

#include <corecrt_math.h>

#include "plot.h"

int ui_plot_path_init(void* data) {
	ui_plot_path_t* path = data;

	path->highlight_index = -1;
	path->selected        = -1;

	return 0;
}

void ui_plot_path_draw(void* data) {
	// FIXME rethink redrawing because this ain't it cheif.
	ui_plot_path_t* path = data;

	for(int i = 0; i < path->path->point_count; i++) {
		// TODO color properly.
		ui_color_t fill      = (ui_color_t){0, 0, 0};
		ui_color_t highlight = (ui_color_t){60, 60, 60};

		draw_fill(fill);
		draw_weight(0);

		if(i == path->highlight_index || i == path->selected) {
			draw_weight(2);
			draw_stroke(highlight);
			draw_fill(highlight);
		}

		draw_circle((ui_pos_t){plot_x(path->path->points[i].x),
		                       plot_y(path->path->points[i].y)},
		            3);

		plot_path_part_t part   = path->path->parts[i / 2];
		double           detail = abs(part.delta_angle * part.radius) / 12;
		for(int k = 0; k < detail; k++) {
			double angle_part = part.delta_angle / detail * k;

			double px = cos(part.start_angle - angle_part) * part.radius;
			double py = sin(part.start_angle - angle_part) * part.radius;

			draw_fill((ui_color_t){0});
			draw_weight(0);

			draw_circle((ui_pos_t){plot_x(px + part.origin.x),
			                       plot_y(py + part.origin.y)},
			            2);
		}
	}
}

int ui_plot_path_event(void* data, ui_event_t e) {
	// TODO make sure to return redraw stuff.
	ui_plot_path_t* path = data;
	plot_vec2_t*    pts  = path->path->points;

	int ret = UI_RET_NONE;

	for(int i = 0; i < path->path->point_count; i++) {
		if(ui_collide_circle(e.mouse.x, e.mouse.y, plot_x(pts[i].x),
		                     plot_y(pts[i].y), 6)) {
			path->highlight_index = i;

			ret |= UI_RET_REDRAW;
			break;
		}
		path->highlight_index = -1;
	}

	if(e.type == UI_EVENT_MOUSE_RELEASE) {
		path->selected = -1;
		ret |= UI_RET_REDRAW | UI_RET_USE_EVENT;
	}
	else if(e.type == UI_EVENT_MOUSE_PRESS) {
		if(path->highlight_index >= 0) {
			path->selected = path->highlight_index;
			ret |= UI_RET_REDRAW | UI_RET_USE_EVENT;
		}
	}
	else if(e.type == UI_EVENT_MOUSE_MOVE && path->selected >= 0) {
		pts[path->selected].x = plot_x(e.mouse.x);
		pts[path->selected].y = plot_y(e.mouse.y);
	}

	return ret;
}