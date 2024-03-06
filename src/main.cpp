#include <Arduino.h>
#include <HX711.h>
#include "stepper.h"
#include "limit_switch.h"

const byte EN_PIN = 8;
const byte X_STP_PIN = 5;
const byte X_DIR_PIN = 2;
const byte LIMIT_SWITCH_LEFT_PIN = A6;
const byte LIMIT_SWITCH_RIGHT_PIN = A7;
const byte LOADCELL_DOUT_PIN = 12;
const byte LOADCELL_SCK_PIN = 13;

const long STP_SIZE = 10;
const float LOAD_CELL_SCALE_FACTOR = 0.00025f;
const long LOAD_CELL_OFFSET = -35000;

Stepper stepper(EN_PIN, X_DIR_PIN, X_STP_PIN);
LimitSwitch limit_left(LIMIT_SWITCH_LEFT_PIN);
LimitSwitch limit_right(LIMIT_SWITCH_RIGHT_PIN);
HX711 load_cell;

static void move_to(long relative_steps)
{
  long dir = (relative_steps > 0) ? 1 : -1;
  long abs_steps = abs(relative_steps);

  const long DELTA_STEPS = 10;

  while (abs_steps > 0)
  {
    long remaing = min(abs_steps, DELTA_STEPS);

    if (limit_left.is_limit_reached())
    {
      Serial.println("left limit switch reached hard limit");
      break;
    }

    if (limit_right.is_limit_reached())
    {
      Serial.println("right limit switch reached hard limit");
      break;
    }

    stepper.move_to(remaing * dir);
    abs_steps -= DELTA_STEPS;
  }
}

static void homing_sequence()
{
  Serial.println("Tensile tester is Homing . . . . . . . . . . . ");

  while (!limit_right.is_limit_reached())
  {
    stepper.move_to(-10);
  }

  stepper.reset_pos();

  Serial.println("Tensile Tester homed . . . . . . . . . . . ");
}

void setup()
{
  Serial.begin(9600);

  Serial.println("setup");
  delay(500);

  stepper.enable();
  load_cell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  homing_sequence();
}

bool is_load_cell_display_shown = false;

void loop()
{
  if (is_load_cell_display_shown)
  {
    if (load_cell.wait_ready_timeout(1000))
    {
      long reading = load_cell.read();
      Serial.print("Load cell reading: ");
      Serial.print(reading);
      Serial.print("  ");
      Serial.print(float(reading - LOAD_CELL_OFFSET) * LOAD_CELL_SCALE_FACTOR);
      Serial.println(" N");
    }
    else
    {
      Serial.println("Load cell not found.");
    }

    delay(10);
  }

  if (Serial.available() > 0)
  {
    int val = Serial.read();

    switch (val)
    {
    case 'l':
    case 'L':
      is_load_cell_display_shown = !is_load_cell_display_shown;
      break;
    case 'h':
    case 'H':
      homing_sequence();
      break;
    case 68:
      Serial.print("left: ");
      if (stepper.pos() <= 3000)
      {
        move_to(10);
      }
      Serial.println(stepper.pos());
      break;
    case 67:
      Serial.print("right: ");

      if (stepper.pos() >= 10)
      {
        move_to(-10);
      }
      Serial.println(stepper.pos());
      break;
    }
  }
}
