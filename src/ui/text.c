#include "ui.h"

int ui_text_init(void* data) {
	return 0;
}

void ui_text_draw(void* data) {
	ui_text_t* text = data;

	draw_fill(ui.col_p);
	draw_text(text->text, text->rect, ui.font_size);
}

int ui_text_event(void* data, ui_event_t e) {
	return 0;
}