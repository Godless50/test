/**
 * @file system_time.h
 * @brief Модуль работы с системным временем, NTP и NVS
 */

#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <time.h>
#include <stdbool.h>

/* Инициализация модуля времени */
void time_init(void);

/* Синхронизация с NTP сервером */
bool time_sync_ntp(const char* server);

/* Сохранение текущего времени в NVS */
void time_save_to_nvs(void);

/* Загрузка времени из NVS */
bool time_load_from_nvs(time_t* out_time);

/* Получение текущего времени */
time_t time_get_current(void);

/* Форматирование времени в строку */
void time_format_string(char* buf, size_t len, time_t t);

/* Обработка Bluetooth времени (полученные данные) */
void time_handle_bt_data(const uint8_t* data, size_t len);

#endif /* SYSTEM_TIME_H */
