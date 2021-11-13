#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plot.h"

int ui_plot_map_init(void* data) {
	ui_plot_map_t* map = data;

	map->zoom = 1.0f;

	return 0;
}

void ui_plot_map_draw(void* data) {
	ui_plot_map_t* map = data;

	draw_transform_mode(1);
	draw_translate(map->rect.x, map->rect.y);
	draw_scale(map->zoom);

	draw_fill((ui_color_t){244, 244, 244});
	draw_stroke((ui_color_t){24, 24, 24});
	draw_weight(3);

	draw_rect(
	    (ui_rect_t){plot_left(), plot_bottom(), plot_right(), plot_top()});

	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(map->plot->paths[i].points == NULL) {

			if(map->ui_paths[i].path != NULL) {
				map->ui_paths[i].path = NULL;
			}

			continue;
		}

		if(map->ui_paths[i].path == NULL) {
			memset(&map->ui_paths[i], 0, sizeof(ui_plot_path_t));
			map->ui_paths[i].path = &map->plot->paths[i];
		}

		ui_plot_path_draw(&map->ui_paths[i]);
	}

	draw_text(map->position_text,
	          (ui_rect_t){map->position_x, map->position_y, 100, 10}, 11);

	draw_reset();
}

int ui_plot_map_event(void* data, ui_event_t e) {
	ui_plot_map_t* map = data;

	if(e.type >= UI_EVENT_MOUSE_PRESS && e.type <= UI_EVENT_MOUSE_MOVE) {
		draw_transform_mode(1);
		draw_translate(map->rect.x, map->rect.y);
		draw_scale(map->zoom);

		ui_pos_t pos = draw_screen_to_world(e.mouse.x, e.mouse.y);
		e.mouse.x    = pos.x;
		e.mouse.y    = pos.y;

		draw_reset();
	}

	if(e.type == UI_EVENT_MOUSE_DOUBLE_CLICK) {
		plot_point_add(plot_x(e.mouse.x), plot_y(e.mouse.y));
		plot_path_calc();
	}
	else if(e.type == UI_EVENT_MOUSE_MOVE) {
		map->position_x = e.mouse.x;
		map->position_y = e.mouse.y;
		sprintf_s(map->position_text, sizeof(map->position_text), "(%.1f,%.1f)",
		          plot_x(e.mouse.x), plot_y(e.mouse.y));
	}
	else if(e.type == UI_EVENT_MOUSE_WHEEL) {
		map->zoom = map->zoom + e.mouse.wheel * 0.25;

		map->zoom = map->zoom < 0.25f ? 0.25f : map->zoom;
		map->zoom = map->zoom > 5.00f ? 5.00f : map->zoom;
	}

	int ret = UI_RET_NONE;
	for(int i = 0; i < PLOT_MAX_PATHS; i++) {
		if(map->plot->paths[i].points == NULL) {

			if(map->ui_paths[i].path != NULL) {
				map->ui_paths[i].path = NULL;
			}

			continue;
		}

		if(map->ui_paths[i].path == NULL) {
			memset(&map->ui_paths[i], 0, sizeof(ui_plot_path_t));
			map->ui_paths[i].path = &map->plot->paths[i];
		}

		ret |= ui_plot_path_event(&map->ui_paths[i], e);

		if(ret & UI_RET_USE_EVENT) {
			break;
		}
	}

	if(! (ret & UI_RET_USE_EVENT) && e.type == UI_EVENT_MOUSE_HOLD) {
		map->rect.x -= e.mouse.delta_x * (1 / map->zoom);
		map->rect.y -= e.mouse.delta_y * (1 / map->zoom);
	}

	if(e.type >= UI_EVENT_MOUSE_PRESS && e.type <= UI_EVENT_MOUSE_MOVE) {
		e.mouse.x = ui.mouse_x;
		e.mouse.y = ui.mouse_y;
	}

	return ret;
}