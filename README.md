# ESP32 Desktop - Графический интерфейс в стиле ОС

Проект реализует полноценный графический интерфейс «рабочего стола» для ESP32 с TFT-дисплеем MKS TS35-R V2.0 и тачскрином. Интерфейс имитирует классическую ОС с иконками, системными часами и полноэкранными приложениями.

## 📁 Структура проекта

```
/workspace
├── platformio.ini          # Конфигурация PlatformIO
├── include/
│   ├── lv_conf.h           # Конфигурация LVGL
│   ├── User_Setup.h        # Настройки дисплея TFT_eSPI
│   ├── app_desktop.h       # Заголовок рабочего стола
│   ├── app_clock.h         # Заголовок часов
│   ├── app_calculator.h    # Заголовок калькулятора
│   ├── app_draw.h          # Заголовок рисовалки
│   ├── app_media.h         # Заголовок медиаплеера
│   └── system_time.h       # Заголовок модуля времени
├── src/
│   ├── main.cpp            # Точка входа
│   ├── system_time.cpp     # Реализация времени (NTP, NVS, BT)
│   ├── app_desktop.cpp     # Рабочий стол с иконками
│   ├── app_clock.cpp       # Приложение часов
│   ├── app_calculator.cpp  # Калькулятор
│   ├── app_draw.cpp        # Рисовалка
│   └── app_media.cpp       # Медиаплеер
└── data/                   # Для файловых ресурсов (опционально)
```

## 🔧 Технические требования

### Оборудование
- **Микроконтроллер**: ESP32 (с PSRAM, например ESP32-WROVER)
- **Дисплей**: MKS TS35-R V2.0 (480x320, ILI9486/ILI9488)
- **Тачскрин**: XPT2046 (резистивный)

### Библиотеки (указаны в `platformio.ini`)
| Библиотека | Версия | Назначение |
|------------|--------|------------|
| lvgl/lvgl | 8.3.0 | Графический фреймворк |
| bodmer/TFT_eSPI | 2.5.43 | Драйвер дисплея |
| ESPAsyncWebServer | 1.2.4 | Веб-сервер для медиа |
| AsyncTCP | 1.1.4 | Асинхронные TCP соединения |
| NTPClient | 3.2.1 | Синхронизация времени |
| Time | 1.6.1 | Работа со временем |

## ⚙️ Настройка

### 1. Конфигурация Wi-Fi
Откройте `src/main.cpp` и замените:
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
```

### 2. Калибровка тачскрина
Уже настроена в `include/User_Setup.h`:
```cpp
#define TOUCH_CALIBRATION_X 216, 3601
#define TOUCH_CALIBRATION_Y 236, 3579
#define TOUCH_CALIBRATION_ROTATION 3
```

### 3. Настройка LVGL
Файл `include/lv_conf.h` содержит оптимизации для ESP32:
- Буферы в PSRAM
- Двойная буферизация
- Разрешение 480x320

## 🚀 Сборка и прошивка

### Установка PlatformIO
```bash
# Если не установлен
pip install platformio
```

### Сборка
```bash
cd /workspace
pio run
```

### Прошивка
```bash
pio run --target upload
```

### Мониторинг
```bash
pio device monitor --baud 115200
```

## 📱 Функционал

### Рабочий стол
- Фон в стиле Windows XP (градиент)
- 4 иконки приложений: Часы, Калькулятор, Рисовалка, Медиаплеер
- Системные часы в правом нижнем углу

### Приложение «Часы»
- Отображение текущего времени
- Настройка даты/времени через ролики
- Сохранение в NVS (энергонезависимая память)
- Синхронизация через NTP (Wi-Fi)
- Прием времени по Bluetooth (протокол: `0xAA 0xBB [timestamp]`)

### Калькулятор
- Полноэкранный режим
- Базовые операции: +, -, ×, ÷
- Ввод с экрана
- Кнопка очистки и удаления последнего символа

### Рисовалка
- Белый холст на весь экран
- Палитра из 16 цветов
- Кнопка очистки холста
- Поддержка тач-ввода

### Медиаплеер
- Потоковый прием JPEG через WebSocket
- Веб-интерфейс с инструкциями
- Авто-скрытие кнопки закрытия через 5 секунд
- Показ кнопки при касании экрана

## ⚠️ Узкие места ESP32 и решения

| Проблема | Описание | Решение |
|----------|----------|---------|
| **Память RAM** | Всего ~520KB, мало для буферов | Использовать PSRAM (4MB+) |
| **Частота кадров видео** | Максимум 10-15 FPS через Wi-Fi | Сжимать JPEG до 320x240, качество 60-80% |
| **Bluetooth пропускная способность** | ~100-200 KB/s реальных | Использовать Wi-Fi для медиа, BT только для команд |
| **Дебаунс тачскрина** | Ложные срабатывания | LVGL встроенный дебаунс + аппаратный фильтр |
| **Скорость отрисовки** | SPI 40MHz лимит | Двойной буфер LVGL, оптимизация областей обновления |

## 📡 Протоколы

### NTP синхронизация
Автоматически при подключении к Wi-Fi:
```cpp
time_sync_ntp("pool.ntp.org");
```

### Bluetooth передача времени
Формат команды от смартфона:
```
[0xAA] [0xBB] [timestamp: 4 байта little-endian]
```

Пример для Android (Kotlin):
```kotlin
val timestamp = System.currentTimeMillis() / 1000
val data = byteArrayOf(
    0xAA.toByte(), 0xBB.toByte(),
    (timestamp and 0xFF).toByte(),
    ((timestamp shr 8) and 0xFF).toByte(),
    ((timestamp shr 16) and 0xFF).toByte(),
    ((timestamp shr 24) and 0xFF).toByte()
)
bluetoothSocket.outputStream.write(data)
```

### Медиапоток WebSocket
URL: `ws://<IP_ESP32>:81/stream`

Отправка JPEG (binary):
```javascript
// JavaScript пример
const ws = new WebSocket('ws://192.168.1.100:81/stream');
ws.binaryType = 'arraybuffer';

function sendFrame(canvas) {
    canvas.toBlob(blob => {
        ws.send(blob);
    }, 'image/jpeg', 0.8);
}
```

## 🔍 Отладка

### Включить логирование LVGL
В `lv_conf.h`:
```cpp
#define LV_LOG_LEVEL LV_LOG_LEVEL_INFO
#define LV_LOG_PRINTF 1
```

### Проверка PSRAM
В мониторе порта при старте:
```
[LVGL] Буферы выделены в PSRAM
```

### Тест тачскрина
Коснитесь экрана — координаты выводятся в Serial.

## 📝 Лицензия

Проект создан для образовательных целей. Используйте свободно.

---

**Автор**: Senior Embedded Developer  
**Дата**: 2024  
**Платформа**: ESP32 + LVGL 8.3 + Arduino
