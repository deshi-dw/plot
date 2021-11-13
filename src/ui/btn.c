#include "ui.h"

#include <stdlib.h>

int ui_btn_init(void* data) {
	ui_btn_t* btn = data;

	btn->is_held  = 0;
	btn->is_hover = 0;

	return 0;
}

void ui_btn_draw(void* data) {
	ui_btn_t*  btn = data;
	ui_color_t bg  = btn->is_hover ? ui.col_bg_s : ui.col_bg;
	bg             = btn->is_held ? ui.col_bg : bg;

	draw_fill(bg);
	draw_stroke(ui.col_p);
	draw_weight(0);

	draw_rect(btn->rect);

	draw_fill(ui.col_p);
	draw_text(btn->text, btn->rect, ui.font_size);
}

int ui_btn_event(void* data, ui_event_t e) {
	ui_btn_t* btn = data;

	// don't update button if the mouse isn't over it.
	if(! ui_collide_rect(e.mouse.x, e.mouse.y, btn->rect)) {
		if(btn->is_hover || btn->is_held) {
			// update visuals if the button was previously hovered over.
			btn->is_hover = 0;
			btn->is_held  = 0;
			return UI_RET_REDRAW;
		}

		return UI_RET_NONE;
	}

	if(e.type == UI_EVENT_MOUSE_HOLD) {
		if(! btn->is_held) {
			btn->is_held = 1;
			return UI_RET_REDRAW;
		}
	}

	if(e.type == UI_EVENT_MOUSE_MOVE && ! btn->is_hover) {
		btn->is_hover = 1;
		return UI_RET_REDRAW;
	}

	if(e.type == UI_EVENT_MOUSE_PRESS) {
		if(btn->on_press != NULL) {
			btn->on_press();
		}

		return UI_RET_USE_EVENT;
	}

	return 0;
}