#include "ui.h"

#include <stdlib.h>

#include "raylib.h"

static ui_t ui		= {0};
static Font ui_font = {0};

static int ui_pos_in_rect(int x, int y, ui_rect_t rect) {
	return x >= rect.x && x <= rect.x + rect.width && y >= rect.y &&
		   y <= rect.y + rect.height;
}

static inline Color ui_c2c(ui_color_t c) {
	return (Color){c.r, c.g, c.b, 255};
}

static inline void ui_draw_text(char* text, ui_rect_t rect, int fontsize,
								ui_color_t color) {
	Vector2 txtsize = MeasureTextEx(ui_font, text, fontsize, 1.2f);
	DrawTextEx(ui_font, text,
			   (Vector2){rect.x + rect.width / 2 - txtsize.x / 2,
						 rect.y + rect.height / 2 - txtsize.y / 2},
			   fontsize, 1.2f, ui_c2c(color));
}

static inline void ui_draw_rect(ui_rect_t rect, int outline, ui_color_t color,
								ui_color_t outline_color) {
	DrawRectangle(rect.x, rect.y, rect.width, rect.height, ui_c2c(color));
	DrawRectangleLinesEx((Rectangle){rect.x, rect.y, rect.width, rect.height},
						 outline, ui_c2c(outline_color));
}

int ui_init() {
	ui.col_a	= (ui_color_t){255, 0, 0};
	ui.col_p	= (ui_color_t){0, 0, 0};
	ui.col_bg_s = (ui_color_t){229, 229, 229};
	ui.col_bg	= (ui_color_t){255, 255, 255};

	ui.font_path = "C:/Windows/Fonts/arial.ttf";
	ui.font_size = 12;
	ui_font		 = LoadFont(ui.font_path);

	return 0;
}

int ui_tick() {
	ui.mouse_x = GetMouseX();
	ui.mouse_y = GetMouseY();

	return 0;
}

int ui_draw() {
	return 0;
}

int ui_text_tick(ui_text_t* text) {
	return 0;
}

int ui_text_draw(ui_text_t* text) {
	ui_draw_text(text->text, text->rect, ui.font_size, ui.col_p);

	return 0;
}

int ui_btn_tick(ui_btn_t* btn) {
	if(ui_pos_in_rect(ui.mouse_x, ui.mouse_y, btn->rect)) {
		btn->is_hover = 1;

		if(IsMouseButtonReleased(0)) {
			if(btn->on_press != NULL) {
				btn->on_press();
			}
		}
		else if(IsMouseButtonDown(0)) {
			btn->is_held = 1;
		}
		else {
			btn->is_held = 0;
		}
	}
	else {
		btn->is_hover = 0;
	}

	return 0;
}

int ui_btn_draw(ui_btn_t* btn) {
	ui_color_t bg = btn->is_hover ? ui.col_bg_s : ui.col_bg;
	bg			  = btn->is_held ? ui.col_bg : bg;

	ui_draw_rect(btn->rect, 0, bg, ui.col_p);

	ui_draw_text(btn->text, btn->rect, ui.font_size, ui.col_p);

	return 0;
}
