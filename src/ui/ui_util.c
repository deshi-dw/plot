#include "ui.h"

#include "raylib.h"

static Font ui_font = {0};

static int        ui_weight = 0;
static ui_color_t ui_fill   = {255};
static ui_color_t ui_stroke = {0};

static Camera2D cam     = {(Vector2){0.0f, 0.0f}, (Vector2){0.0f, 0.0f}, 0.0f,
                       1.0f};
static int      is_mode = 0;
static int      cam_is_abs = 0;

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

void draw_transform_mode(int is_absolute) {
	cam_is_abs = is_absolute;
}

void draw_translate(int x, int y) {
	if(cam_is_abs) {
		cam.target.x = x;
		cam.target.y = y;
	}
	else {
		cam.target.x += x;
		cam.target.y += y;
	}
	if(ui.is_drawing) {
		is_mode = 1;
	}
}

void draw_rotate(float rot) {
	if(cam_is_abs) {
		cam.rotation = rot;
	}
	else {
		cam.rotation += rot;
	}

	if(ui.is_drawing) {
		is_mode = 1;
	}
}

void draw_scale(float scale) {
	if(cam_is_abs) {
		cam.zoom = scale;
	}
	else {
		cam.zoom += scale;
	}

	if(ui.is_drawing) {
		is_mode = 1;
	}
}

void draw_reset() {
	is_mode = 0;

	cam.target.x = 0;
	cam.target.y = 0;

	cam.offset.x = 0;
	cam.offset.y = 0;

	cam.rotation = 0.0f;
	cam.zoom     = 1.0f;
}

ui_pos_t draw_screen_to_world(int x, int y) {
	Vector2 pos = GetScreenToWorld2D((Vector2){x, y}, cam);

	return (ui_pos_t){pos.x, pos.y};
}
ui_pos_t draw_world_to_point(int x, int y) {
	Vector2 pos = GetWorldToScreen2D((Vector2){x, y}, cam);

	return (ui_pos_t){pos.x, pos.y};
}

void draw_text(char* text, ui_rect_t rect, int size) {
	if(is_mode) {
		BeginMode2D(cam);
	}

	Vector2 txtsize = MeasureTextEx(ui_font, text, size, 1.2f);
	DrawTextEx(ui_font, text,
	           (Vector2){rect.x + rect.width / 2 - txtsize.x / 2,
	                     rect.y + rect.height / 2 - txtsize.y / 2},
	           size, 1.2f, c2c(ui_fill));

	if(is_mode) {
		EndMode2D();
	}
}

void draw_rect(ui_rect_t rect) {
	if(is_mode) {
		BeginMode2D(cam);
	}

	DrawRectangle(rect.x, rect.y, rect.width, rect.height, c2c(ui_fill));
	DrawRectangleLinesEx((Rectangle){rect.x, rect.y, rect.width, rect.height},
	                     ui_weight, c2c(ui_stroke));

	if(is_mode) {
		EndMode2D();
	}
}

void draw_circle(int x, int y, int radius) {
	if(is_mode) {
		BeginMode2D(cam);
	}

	DrawCircle(x, y, radius + ui_weight, c2c(ui_stroke));
	DrawCircle(x, y, radius, c2c(ui_fill));

	if(is_mode) {
		EndMode2D();
	}
}