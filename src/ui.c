#include "ui.h"

#include <stdlib.h>
#include <string.h>

#include "raylib.h"

ui_t ui = {0};

// UI DRAWING AND EVENTS

// static void ui_btn_list_draw(void* data) {
// 	ui_btn_list_t* btn_list = data;

// 	if(! btn_list->show) {
// 		return;
// 	}

// 	for(int i = 0; i < btn_list->btn_count; i++) {
// 		ui_btn_draw(&btn_list->btns[i]);
// 	}
// }

// static int ui_btn_list_event(void* data, ui_event_t e) {
// 	ui_btn_list_t* btn_list = data;

// 	int ret = UI_RET_NONE;

// 	for(int i = 0; i < btn_list->btn_count; i++) {
// 		ret |= ui_btn_event(&btn_list->btns[i], e);
// 	}

// 	return ret;
// }

// static int ui_rename_btn_event(void* data, ui_event_t e) {
// }

int ui_init(int width, int height) {
	ui.width  = width;
	ui.height = height;

	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(width, width, "plot");
	SetTargetFPS(30);

	ui.col_a    = (ui_color_t){255, 0, 0};
	ui.col_p    = (ui_color_t){0, 0, 0};
	ui.col_bg_s = (ui_color_t){229, 229, 229};
	ui.col_bg   = (ui_color_t){255, 255, 255};

	ui_load_font("C:/Windows/Fonts/arial.ttf");
	ui.font_size = 12;

	return 0;
}

int ui_is_closed() {
	return WindowShouldClose();
}

static int ui_mb_press() {
	// I could to a bitfield but we don't really need more than one mouse button
	// event at a time.
	if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		return MOUSE_LEFT_BUTTON;
	}
	else if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
		return MOUSE_RIGHT_BUTTON;
	}
	else if(IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
		return MOUSE_MIDDLE_BUTTON;
	}

	return -1;
}
static int ui_mb_release() {
	// I could to a bitfield but we don't really need more than one mouse button
	// event at a time.
	if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		return MOUSE_LEFT_BUTTON;
	}
	else if(IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
		return MOUSE_RIGHT_BUTTON;
	}
	else if(IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON)) {
		return MOUSE_MIDDLE_BUTTON;
	}

	return -1;
}
static int ui_mb_hold() {
	// I could to a bitfield but we don't really need more than one mouse button
	// event at a time.
	if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		return MOUSE_LEFT_BUTTON;
	}
	else if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
		return MOUSE_RIGHT_BUTTON;
	}
	else if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
		return MOUSE_MIDDLE_BUTTON;
	}

	return -1;
}

int ui_tick() {
	ui.mouse_last_x = ui.mouse_x;
	ui.mouse_last_y = ui.mouse_y;
	ui.mouse_x      = GetMouseX();
	ui.mouse_y      = GetMouseY();

	// clear old event data.
	memset(ui.events, 0, sizeof(ui.events));

	int i = 0;

// fire events in case of mouse or keyboard activity.
#define UPDATE_MOUSE(index)                                          \
	ui.events[index].e.mouse.x       = ui.mouse_x;                   \
	ui.events[index].e.mouse.y       = ui.mouse_y;                   \
	ui.events[index].e.mouse.delta_x = ui.mouse_x - ui.mouse_last_x; \
	ui.events[index].e.mouse.delta_y = ui.mouse_y - ui.mouse_last_y;

	double time = GetTime();

	if(ui_mb_release() != -1) {
		if(time - ui.mouse_release_time < 0.15) {
			ui.events[i].e.type = UI_EVENT_MOUSE_DOUBLE_CLICK;

			ui.events[i].e.mouse.button = ui_mb_release();

			UPDATE_MOUSE(i)
			i++;
		}

		ui.mouse_release_time = time;
	}

	if(ui_mb_press() != -1) {
		ui.events[i].e.type = UI_EVENT_MOUSE_PRESS;

		ui.events[i].e.mouse.button = ui_mb_press();
		UPDATE_MOUSE(i)

		i++;
	}
	if(ui_mb_release() != -1) {

		ui.events[i].e.type = UI_EVENT_MOUSE_RELEASE;

		ui.events[i].e.mouse.button = ui_mb_release();
		UPDATE_MOUSE(i)

		i++;
	}
	if(ui_mb_hold() != -1) {
		ui.events[i].e.type = UI_EVENT_MOUSE_HOLD;

		ui.events[i].e.mouse.button = ui_mb_hold();
		UPDATE_MOUSE(i)

		i++;
	}

	if(ui.mouse_x != ui.mouse_last_x || ui.mouse_y != ui.mouse_last_y) {
		ui.events[i].e.type = UI_EVENT_MOUSE_MOVE;

		ui.events[i].e.mouse.button = -1;
		UPDATE_MOUSE(i)

		i++;
	}

	if(GetMouseWheelMove() != 0.0f) {
		ui.events[i].e.type = UI_EVENT_MOUSE_WHEEL;

		ui.events[i].e.mouse.button = -1;
		ui.events[i].e.mouse.wheel  = GetMouseWheelMove();
		UPDATE_MOUSE(i)

		i++;
	}

	// use all the unused event slots. A bit odd but this ensures that empty
	// events aren't used.
	for(; i < UI_MAX_EVENTS; i++) {
		ui.events[i].used = 1;
	}

#undef UPDATE_MOUSE

	// update each ui element.
	for(int i = 0; i < UI_MAX_ELEMENTS; i++) {
		if(ui.elements[i].data == NULL) {
			continue;
		}

		// update the ui element with events.
		for(int j = 0; j < UI_MAX_EVENTS; j++) {
			if(ui.events[j].used) {
				continue;
			}

			if(ui.elements[i].event != NULL) {
				int ret =
				    ui.elements[i].event(ui.elements[i].data, ui.events[j].e);

				if(ret & UI_RET_REDRAW) {
					ui.elements[i].dirty = 1;
				}
				if(ret & UI_RET_USE_EVENT) {
					ui.events[j].used = 1;
				}
			}
		}
	}

	return 0;
}

int ui_draw() {
	// draw all elements.
	ui.is_drawing = 1;
	BeginDrawing();

	// TODO reimplement ui.elements[i].dirty check but better. Maybe with
	// BeginScissorMode drawing or something
	ClearBackground(WHITE);

	for(int i = 0; i < UI_MAX_ELEMENTS; i++) {
		if(ui.elements[i].data == NULL || ! ui.elements[i].dirty) {
			continue;
		}

		if(ui.elements[i].draw != NULL) {
			ui.elements[i].draw(ui.elements[i].data);
			draw_reset();
		}
	}

	EndDrawing();

	ui.is_drawing = 0;

	return 0;
}

int ui_add(enum ui_element_type_t type, void* element) {
	for(int i = UI_MAX_ELEMENTS - 1; i >= 0; i--) {
		if(ui.elements[i].data != NULL) {
			continue;
		}

		switch(type) {
#define UI_ELEMENT(name, id, body, ...)      \
	case id:                                 \
		ui.elements[i].draw  = name##_draw;  \
		ui.elements[i].event = name##_event; \
		name##_init(element);                \
		break;
#include "ui.def"
#undef UI_ELEMENT

			default:
				return -1;
		}

		ui.elements[i].data  = element;
		ui.elements[i].dirty = 1;

		return i;
	}

	return -1;
}
