#include <string.h>
#include <math.h>
#include <vector>
#include <cstdlib>

#include "libraries/pico_display_2/pico_display_2.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include "button.hpp"

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "modules/pico-onewire/api/one_wire.h"
#include "font8_data.hpp"
#include "font14_outline_data.hpp"

using namespace pimoroni;

#define rowHeight 20
#define valOffset 250

ST7789 st7789(320, 240, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);

void toggle_rVal(void);

Button button_a(PicoDisplay2::A);
Button button_b(PicoDisplay2::B);
Button button_x(PicoDisplay2::X);
Button button_y(PicoDisplay2::Y);

static char printBuf[256];
bool toggle_led = false;
static uint8_t rVal = 0;
static Point text_location(0, 0);

static float setPoint{50};
static float currentTemperature{0};

typedef struct
{
  int red;
  int green;
  int blue;
} RGB_t;

static bool ready = true;

int main()
{
  // Define RGB values for a gradient from blue to green/yellow to red
  RGB_t gradient[] = {
      {0, 0, 255},    // Blue
      {30, 144, 255}, // Dodger Blue
      {0, 255, 0},    // Lime Green
      {255, 255, 0},  // Yellow
      {255, 165, 0},  // Orange
      {255, 69, 0},   // Red-Orange
      {255, 0, 0},    // Red
      {128, 0, 0},    // Maroon
      {139, 0, 139},  // Dark Magenta
      {148, 0, 211}   // Dark Violet
  };

  stdio_init_all();
  One_wire one_wire(2); // GP2
  one_wire.init();
  rom_address_t address{};
  static repeating_timer_t timer;

  st7789.set_backlight(255);

  Pen WHITE = graphics.create_pen(255, 255, 255);

  printf("\nBegin main loop");
  led.set_brightness(64);

  while (true)
  {
    one_wire.single_device_read_rom(address);
    one_wire.convert_temperature(address, false, false);
    currentTemperature = one_wire.temperature(address);
    // Calculate the index in the gradient array based on the temperature
    int index = ((int)round(currentTemperature) * 9) / 100;
    // Create a pen with the RGB values from the selected index
    Pen BG = graphics.create_pen(gradient[index].red, gradient[index].green, gradient[index].blue);
    // Set the background colour
    graphics.set_pen(BG);
    graphics.clear();

    // Check if the currentTemperature is within 2 degrees of setPoint
    int withinTwoDegrees = (currentTemperature >= setPoint - 2 && currentTemperature <= setPoint + 2);

    ready = withinTwoDegrees ? true : false;

    if (ready)
    {
      led.set_rgb(gradient[index].red, gradient[index].green, gradient[index].blue);
    }
    else
    {
      led.set_rgb(0, 0, 0);
    }

    if (button_a.raw())
    {
      if (setPoint >= 5.0)
      {
        setPoint -= 5.0;
      }
    }

    if (button_b.raw())
    {
      if (setPoint >= 1.0)
      {
        setPoint -= 1.0;
      }
    }

    if (button_x.raw())
    {
      if (setPoint <= 100.0)
      {
        setPoint += 5.0;
      }
    }

    if (button_y.raw())
    {
      if (setPoint <= 100.0)
      {
        setPoint += 1.0;
      }
    }

    graphics.set_pen(WHITE);

    text_location.x += 48;
    graphics.set_font(&font8);
    graphics.text("--- Wax Melt Monitor ---", text_location, 320, 2);
    text_location.x -= 48;

    graphics.set_font(&font14_outline);
    text_location.y += rowHeight * 5;
    text_location.x += 130;
    snprintf(printBuf, 256, "%3.1f C", currentTemperature);
    graphics.text(printBuf, text_location, 320);
    text_location.x -= 130;

    text_location.y += rowHeight * 5;
    text_location.x += 20;
    graphics.set_font(&font8);
    snprintf(printBuf, 256, "Set point: %3.1f C", setPoint);
    graphics.text(printBuf, text_location, 320);
    text_location.x -= 20;

    text_location.y -= rowHeight * 10;

    sleep_ms(50);

    // update screen
    st7789.update(&graphics);
  }

  return 0;
}
