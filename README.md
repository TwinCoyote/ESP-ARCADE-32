# ESP-ARCADE-32

# ESPARCADE Firmware Architecture (ESP32-S3)

## Overview

ESPARCADE is a modular handheld gaming console based on **ESP32-S3**, designed with scalability, maintainability and portability in mind.

The firmware follows a layered architecture to separate:

* Hardware abstraction
* System services
* Core engine logic
* User interface
* Games

This allows:

* Multiple display compatibility (OLED/TFT)
* Modular game integration
* OTA firmware updates
* RTOS-based multitasking
* Dual-core optimization
* Easy scalability for future hardware revisions

---

# System Architecture

```txt
+--------------------------------------------------+
|                    APPLICATION                   |
|--------------------------------------------------|
| Snake | Pong | Tetris | Future Games            |
+--------------------------------------------------+

+--------------------------------------------------+
|                    CORE ENGINE                   |
|--------------------------------------------------|
| Game Manager                                     |
| Scene Manager                                    |
| Render Engine                                    |
| Input Manager                                    |
| Audio Manager                                    |
+--------------------------------------------------+

+--------------------------------------------------+
|                    SYSTEM SERVICES               |
|--------------------------------------------------|
| OTA Service                                      |
| Save Service                                     |
| WiFi Service                                     |
| Battery Service                                  |
| File System Service                              |
+--------------------------------------------------+

+--------------------------------------------------+
|                 HARDWARE ABSTRACTION             |
|--------------------------------------------------|
| Display Driver Interface                         |
| Input Driver                                     |
| Audio Driver                                     |
| Storage Driver                                   |
| Power Driver                                     |
+--------------------------------------------------+

+--------------------------------------------------+
|                     HARDWARE                     |
|--------------------------------------------------|
| ESP32-S3                                         |
| OLED / TFT Display                               |
| Buttons                                          |
| Speaker                                          |
| Battery                                          |
| USB-C                                            |
+--------------------------------------------------+
```

---

# Project Structure

```txt
ESPARCADE/
│
├── include/
│
├── src/
│   │── main.cpp
│   │
│   ├── core/
│   │   ├── game_manager.cpp
│   │   ├── game_manager.h
│   │   │
│   │   ├── scene_manager.cpp
│   │   ├── scene_manager.h
│   │   │
│   │   ├── render_engine.cpp
│   │   ├── render_engine.h
│   │   │
│   │   ├── input_manager.cpp
│   │   └── input_manager.h
│   │
│   ├── drivers/
│   │   ├── display/
│   │   │   ├── display_interface.h
│   │   │   ├── ssd1306_display.cpp
│   │   │   ├── sh1106_display.cpp
│   │   │   └── tft_display.cpp
│   │   │
│   │   ├── input/
│   │   │   ├── buttons.cpp
│   │   │   └── buttons.h
│   │   │
│   │   ├── audio/
│   │   └── storage/
│   │
│   ├── services/
│   │   ├── ota_service.cpp
│   │   ├── save_service.cpp
│   │   ├── wifi_service.cpp
│   │   ├── battery_service.cpp
│   │   └── filesystem_service.cpp
│   │
│   ├── games/
│   │   ├── game.h
│   │   │
│   │   ├── snake/
│   │   │   ├── snake.cpp
│   │   │   └── snake.h
│   │   │
│   │   ├── pong/
│   │   │   ├── pong.cpp
│   │   │   └── pong.h
│   │   │
│   │   └── tetris/
│   │
│   ├── ui/
│   │   ├── menu.cpp
│   │   ├── menu.h
│   │   ├── animations.cpp
│   │   └── icons.cpp
│   │
│   └── config/
│       ├── pins.h
│       ├── display_config.h
│       ├── game_config.h
│       └── system_config.h
│
├── test/
│
├── docs/
│   ├── architecture.md
│   ├── hardware.md
│   └── roadmap.md
│
└── platformio.ini
```

---

# Dual-Core Architecture (FreeRTOS)

The ESP32-S3 dual-core processor is used to separate **real-time game execution** from **background system services**.

## Core Allocation

### Core 0 → System Services

Responsible for background and non-critical tasks.

```txt
CORE 0
│
├── WiFi Task
├── OTA Task
├── Battery Monitoring
├── Save Service
└── File System Operations
```

Responsibilities:

* OTA firmware updates
* WiFi management
* Background save operations
* Battery monitoring
* File system access

This prevents system services from blocking gameplay.

---

### Core 1 → Game Engine

Responsible for real-time console behavior.

```txt
CORE 1
│
├── Input Task
├── Game Logic Task
├── Render Task
├── Audio Task
└── UI/Menu Task
```

Responsibilities:

* Reading buttons
* Running game logic
* Display rendering
* Audio playback
* UI transitions

This ensures responsive gameplay and smooth rendering.

---

# RTOS Task Architecture

```txt
+--------------------------------------+
|              CORE 0                 |
+--------------------------------------+
| WiFi Task                            |
| OTA Task                             |
| Save Task                            |
| Battery Task                         |
+--------------------------------------+

+--------------------------------------+
|              CORE 1                 |
+--------------------------------------+
| Input Task                           |
| Game Loop Task                       |
| Render Task                          |
| Audio Task                           |
+--------------------------------------+
```

---

# Game Architecture

All games inherit from a base abstract class.

```cpp
class Game {
public:
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
};
```

Example:

```cpp
class Snake : public Game
```

This allows:

* Dynamic game switching
* Scalable game development
* Plug-and-play integration of future games

---

# Display Abstraction Layer

To support multiple displays without changing game logic, ESPARCADE uses a display interface.

```cpp
class DisplayInterface {
public:
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void drawPixel(int x, int y) = 0;
    virtual void update() = 0;
};
```

Supported displays may include:

* SSD1306 OLED
* SH1106 OLED
* SPI TFT Displays

Games do not interact with hardware directly.

Instead:

```cpp
display.drawPixel(x, y);
```

This makes the firmware hardware-independent.

---

# Development Philosophy

ESPARCADE is designed around:

* Modular firmware architecture
* Scalability
* Hardware abstraction
* Maintainability
* Professional embedded development practices
* CI/CD integration
* OTA firmware updates
* RTOS multitasking
* Cross-display compatibility

Goal:

Build a professional-grade ESP32 handheld gaming platform suitable for embedded systems portfolio and firmware engineering experience.
