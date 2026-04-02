/**
 * @file app_media.h
 * @brief Заголовок модуля медиаплеера
 */

#ifndef APP_MEDIA_H
#define APP_MEDIA_H

#include <lvgl.h>

/* Инициализация медиаплеера */
void media_app_init(void);

/* Старт приема потока */
void media_start_stream(void);

/* Стоп приема потока */
void media_stop_stream(void);

/* Обработка полученного кадра (вызывается из веб-сервера) */
void media_on_frame_received(uint8_t* data, size_t len);

/* Очистка ресурсов */
void media_app_deinit(void);

#endif /* APP_MEDIA_H */
