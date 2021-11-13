#ifndef UI_H
#define UI_H

// include once up here so we can get all of the internal includes included if
// that makes sense.
#include "ui.def"

typedef struct ui_pos_t   ui_pos_t;
typedef struct ui_rect_t  ui_rect_t;
typedef struct ui_color_t ui_color_t;

typedef struct ui_event_t ui_event_t;

typedef struct ui_t ui_t;

struct ui_pos_t {
	int x;
	int y;
};

struct ui_rect_t {
	int x;
	int y;
	int width;
	int height;
};

struct ui_color_t {
	int r;
	int g;
	int b;
};

enum ui_element_type_t {
	UI_ELEMENT_NONE = 0,
#define UI_ELEMENT(name, id, body, ...) id,
#include "ui.def"
#undef UI_ELEMENT
};

#define UI_ELEMENT(name, id, body, ...) typedef struct name##_t name##_t;
#include "ui.def"
#undef UI_ELEMENT

#define UI_ELEMENT(name, id, body, ...) struct name##_t body;
#include "ui.def"
#undef UI_ELEMENT

enum ui_event_type_t {
	// mouse button events
	UI_EVENT_MOUSE_PRESS,
	UI_EVENT_MOUSE_RELEASE,
	UI_EVENT_MOUSE_HOLD,
	UI_EVENT_MOUSE_DOUBLE_CLICK,

	// mouse wheel events
	UI_EVENT_MOUSE_WHEEL,

	// mouse move events
	UI_EVENT_MOUSE_MOVE,

	// keyboard events
	UI_EVENT_KEY_PRESS,
	UI_EVENT_KEY_RELEASE,
	UI_EVENT_KEY_HOLD,
};

enum ui_ret_flags_t {
	UI_RET_NONE      = 0,
	UI_RET_REDRAW    = 1 << 1,
	UI_RET_USE_EVENT = 1 << 2,
};

struct ui_event_t {
	enum ui_event_type_t type;
	union {
		struct ui_event_mouse_t {
			int   button;
			float wheel;
			int   x;
			int   y;
			int   delta_x;
			int   delta_y;
		} mouse;

		// keyboard events
		struct ui_event_key_t {
			int mod;
			int key;
		} key;
	};
};

// TODO implement
struct ui_action_t {
	void (*doit)();
	void (*undo)();
};

typedef void (*ui_draw_func_t)(void* element);
typedef int (*ui_event_func_t)(void* element, ui_event_t e);

struct ui_t {
	int width;
	int height;

	int   mouse_x;
	int   mouse_y;
	int   mouse_last_x;
	int   mouse_last_y;
	float mwheel;

	double mouse_release_time;

	ui_color_t col_a;
	ui_color_t col_p;
	ui_color_t col_bg_s;
	ui_color_t col_bg;

#define UI_MAX_EVENTS 32
	struct {
		int        used;
		ui_event_t e;
	} events[UI_MAX_EVENTS];

#define UI_MAX_ELEMENTS 256
	struct {
		void*           data;
		int             dirty;
		ui_draw_func_t  draw;
		ui_event_func_t event;
	} elements[UI_MAX_ELEMENTS];

	char* font_path;
	int   font_size;

	int is_drawing;
};

extern ui_t ui;

int ui_init(int width, int height);

int ui_is_closed();

int ui_tick();
int ui_draw();

int ui_add(enum ui_element_type_t type, void* element);
int ui_del(void* element);

int ui_collide_rect(int x, int y, ui_rect_t rect);
int ui_collide_circle(int x, int y, int cx, int cy, int radius);

int ui_load_font(char* font_path);

void draw_stroke(ui_color_t color);
void draw_fill(ui_color_t color);
void draw_weight(int weight);

void draw_transform_mode(int is_absolute);
void draw_translate(int x, int y);
void draw_rotate(float rot);
void draw_scale(float scale);
void draw_reset();

ui_pos_t draw_screen_to_world(int x, int y);
ui_pos_t draw_world_to_point(int x, int y);

void draw_text(char* text, ui_rect_t rect, int size);
void draw_rect(ui_rect_t rect);
void draw_circle(int x, int y, int radius);

#define UI_ELEMENT(name, id, body, ...)          \
	void name##_draw(void* data);                \
	int  name##_event(void* data, ui_event_t e); \
	int  name##_init(void* data);
#include "ui.def"
#undef UI_ELEMENT

#endif
