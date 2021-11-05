#ifndef UI_H
#define UI_H

typedef struct ui_pos_t	  ui_pos_t;
typedef struct ui_rect_t  ui_rect_t;
typedef struct ui_color_t ui_color_t;

typedef struct ui_text_t	ui_text_t;
typedef struct ui_textbox_t ui_textbox_t;
typedef struct ui_btn_t		ui_btn_t;

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

struct ui_text_t {
	ui_rect_t rect;
	char*	  text;
};

struct ui_btn_t {
	ui_rect_t rect;

	void (*on_press)();

	int	  is_hover;
	int	  is_held;
	char* text;
};

enum ui_event_type_t {
	// mouse button events
	UI_EVENT_MOUSE_PRESS,
	UI_EVENT_MOUSE_RELEASE,
	UI_EVENT_MOUSE_HOLD,

	// mouse move events
	UI_EVENT_MOUSE_MOVE,

	// keyboard events
	UI_EVENT_KEY_PRESS,
	UI_EVENT_KEY_RELEASE,
	UI_EVENT_KEY_HOLD,

	// ui events
	UI_EVENT_BTN_CLICK
};

struct ui_event_t {
	int type;
	union {
		// mouse button events
		struct ui_event_mouse_press_t {
			int button;
			int x;
			int y;
		} mouse_press;

		struct ui_event_mouse_release_t {
			int button;
			int x;
			int y;
		} mouse_release;

		struct ui_event_mouse_hold_t {
			int button;
			int x;
			int y;
		} mouse_hold;

		// mouse move events
		struct ui_event_mouse_move_t {
			int x;
			int y;
			int delta_x;
			int delta_y;
		} mouse_move;

		// keyboard events
		struct ui_event_key_press_t {
			int mod;
			int key;
		} key_press;

		struct ui_event_key_release_t {
			int mod;
			int key;
		} key_release;

		struct ui_event_key_hold_t {
			int mod;
			int key;
		} key_hold;

		// ui events
		struct ui_event_btn_click_t {
			ui_btn_t* btn;
		} btn_click;

	} e;
};

struct ui_t {
	int mouse_x;
	int mouse_y;

	ui_color_t col_a;
	ui_color_t col_p;
	ui_color_t col_bg_s;
	ui_color_t col_bg;

	char* font_path;
	int	  font_size;
};

int ui_init();

int ui_tick();
int ui_draw();

int ui_text_tick(ui_text_t* text);
int ui_text_draw(ui_text_t* text);

int ui_btn_tick(ui_btn_t* btn);
int ui_btn_draw(ui_btn_t* btn);

#endif
