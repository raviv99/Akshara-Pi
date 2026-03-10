#ifndef WAVESHARE_BRIDGE_H
#define WAVESHARE_BRIDGE_H

#include "DEV_Config.h"
#include "EPD_5in83.h"  // Points specifically to the 5.83" driver

/**
 * AKSHARA-PI HARDWARE HANDSHAKE
 * Maps generic display commands to Waveshare-specific HAL
 */

static inline void display_init() {
    DEV_Module_Init();
    EPD_5in83_Init();
    EPD_5in83_Clear();
}

static inline void display_refresh_full(unsigned char *framebuffer) {
    EPD_5in83_Display(framebuffer);
}

static inline void display_sleep() {
    EPD_5in83_Sleep();
    DEV_Module_Exit();
}

// Define the screen dimensions for the rest of the app
#define EPD_WIDTH  EPD_5in83_WIDTH
#define EPD_HEIGHT EPD_5in83_HEIGHT

#endif
