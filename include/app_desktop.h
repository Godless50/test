/**
 * @file app_desktop.h
 * @brief Заголовок модуля рабочего стола
 */

#ifndef APP_DESKTOP_H
#define APP_DESKTOP_H

#include <lvgl.h>

/* Инициализация рабочего стола */
void desktop_init(void);

/* Обработчик нажатия на иконку */
void desktop_on_icon_click(const char* app_name);

/* Обновление системных часов */
void desktop_update_clock(void);

/* Очистка ресурсов */
void desktop_deinit(void);

#endif /* APP_DESKTOP_H */
