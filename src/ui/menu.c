#include "ui.h"

#include <stdlib.h>
#include <string.h>

int ui_menu_init(void* data) {
	ui_menu_t* menu = data;

	menu->show = 0;

	return 0;
}

void ui_menu_draw(void* data) {
	ui_menu_t* menu = data;

	if(! menu->show) {
		return;
	}

	for(int i = 0; i < menu->btn_count; i++) {
		ui_btn_draw(menu->btns[i]);
	}
}

int ui_menu_event(void* data, ui_event_t e) {
	ui_menu_t* menu = data;

	if(! menu->show) {
		return UI_RET_NONE;
	}

	ui_rect_t collide_rect = menu->rect;
	collide_rect.y -= menu->btn_height;
	collide_rect.height += menu->btn_height;

	if(e.type == UI_EVENT_MOUSE_MOVE &&
	   ! ui_collide_rect(e.mouse.x, e.mouse.y, collide_rect)) {
		menu->show = 0;
		return UI_RET_REDRAW;
	}

	int ret = UI_RET_NONE;

	for(int i = 0; i < menu->btn_count; i++) {
		ret |= ui_btn_event(menu->btns[i], e);
	}

	return ret;
}

void ui_menu_add(ui_menu_t* menu, ui_btn_t* btn) {
	if(menu->btns == NULL) {
		menu->size = menu->size <= 0 ? 4 : menu->size;
		menu->btns = calloc(menu->size, sizeof(ui_btn_t*));
	}

	// resize btns array if it isn't big enough.
	if(menu->btn_count + 1 >= menu->size) {
		ui_btn_t** temp = calloc(menu->size * 2, sizeof(ui_btn_t*));
		memcpy(temp, menu->btns, menu->size);

		menu->size *= 2;
		free(menu->btns);
		menu->btns = temp;
	}

	menu->rect.height = menu->btn_height * (menu->btn_count + 1);

	btn->rect.width  = menu->rect.width;
	btn->rect.height = menu->btn_height;
	btn->rect.x      = menu->rect.x;
	btn->rect.y      = menu->rect.y + menu->btn_height * menu->btn_count;

	menu->btns[menu->btn_count++] = btn;
}