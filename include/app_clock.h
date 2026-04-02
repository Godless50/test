/**
 * @file app_clock.h
 * @brief Заголовок модуля часов и настройки времени
 */

#ifndef APP_CLOCK_H
#define APP_CLOCK_H

#include <lvgl.h>
#include <time.h>

/* Инициализация приложения часов */
void clock_app_init(void);

/* Открытие окна настройки времени */
void clock_app_show_settings(void);

/* Сохранение времени в NVS */
void clock_save_time(time_t t);

/* Загрузка времени из NVS */
time_t clock_load_time(void);

/* Синхронизация через NTP */
bool clock_sync_ntp(const char* ntp_server);

/* Очистка ресурсов */
void clock_app_deinit(void);

#endif /* APP_CLOCK_H */
