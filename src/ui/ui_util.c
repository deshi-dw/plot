#include "ui.h"

#include "raylib.h"

static Font ui_font = {0};

static int        ui_weight = 0;
static ui_color_t ui_fill   = {255};
static ui_color_t ui_stroke = {0};

static inline Color c2c(ui_color_t c) {
	return (Color){c.r, c.g, c.b, 255};
}

int ui_collide_rect(int x, int y, ui_rect_t rect) {
	return x >= rect.x && x <= rect.x + rect.width && y >= rect.y &&
	       y <= rect.y + rect.height;
}

int ui_collide_circle(int x, int y, int cx, int cy, int radius) {
	return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= radius * radius;
}

int ui_load_font(char* font_path) {
	ui.font_path = font_path;
	ui_font      = LoadFont(font_path);

	return 0;
}

void draw_stroke(ui_color_t color) {
	ui_stroke = color;
}
void draw_fill(ui_color_t color) {
	ui_fill = color;
}

void draw_weight(int weight) {
	ui_weight = weight;
}

void draw_text(char* text, ui_rect_t rect, int size) {
	Vector2 txtsize = MeasureTextEx(ui_font, text, size, 1.2f);
	DrawTextEx(ui_font, text,
	           (Vector2){rect.x + rect.width / 2 - txtsize.x / 2,
	                     rect.y + rect.height / 2 - txtsize.y / 2},
	           size, 1.2f, c2c(ui_fill));
}

void draw_rect(ui_rect_t rect) {
	DrawRectangle(rect.x, rect.y, rect.width, rect.height, c2c(ui_fill));
	DrawRectangleLinesEx((Rectangle){rect.x, rect.y, rect.width, rect.height},
	                     ui_weight, c2c(ui_stroke));
}

void draw_circle(ui_pos_t pos, int radius) {
	DrawCircle(pos.x, pos.y, radius + ui_weight, c2c(ui_stroke));
	DrawCircle(pos.x, pos.y, radius, c2c(ui_fill));
}