/**
 * @file app_draw.h
 * @brief Заголовок модуля рисовалки
 */

#ifndef APP_DRAW_H
#define APP_DRAW_H

#include <lvgl.h>

/* Инициализация рисовалки */
void draw_app_init(void);

/* Установка цвета кисти */
void draw_set_brush_color(lv_color_t color);

/* Очистка холста */
void draw_clear_canvas(void);

/* Очистка ресурсов */
void draw_app_deinit(void);

#endif /* APP_DRAW_H */
