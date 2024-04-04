#include <Arduino.h>
#include <HX711.h>
#include "stepper.h"
#include "limit_switch.h"
#include "serial_buffer.h"

const byte EN_PIN = 8;
const byte X_STP_PIN = 5;
const byte X_DIR_PIN = 2;
const byte LIMIT_SWITCH_LEFT_PIN = A6;
const byte LIMIT_SWITCH_RIGHT_PIN = A7;
const byte LOADCELL_DOUT_PIN = 12;
const byte LOADCELL_SCK_PIN = 13;
const byte STEPS_PER_ROTATION = 200;
const float MM_TO_ROTATION = 1.0f / 3.0f;
const long REPORT_VALUES_INTERVAL_MS = 1000 / 5;

const long STP_SIZE = 10;
const float LOAD_CELL_SCALE_FACTOR = 0.00025f;
const long LOAD_CELL_OFFSET = -35000;

static SerialBuffer serial_buffer{};
static Stepper stepper{EN_PIN, X_DIR_PIN, X_STP_PIN};
static LimitSwitch limit_left{LIMIT_SWITCH_LEFT_PIN};
static LimitSwitch limit_right{LIMIT_SWITCH_RIGHT_PIN};
static HX711 load_cell;

static long tensile;

static float speed = 1.0f;
static float max_pos = 0.0f;
static bool is_testing = false;
static long prev_time = millis();

// TODO: rename
static void report_values()
{
  static long last_update = 0;

  if ((millis() - last_update) > REPORT_VALUES_INTERVAL_MS)
  {
    last_update = millis();

    long pos_steps = stepper.pos();

    float pos = float(pos_steps) / float(STEPS_PER_ROTATION) / float(MM_TO_ROTATION);

    tensile = load_cell.read();
    Serial.print("X:");
    Serial.print(pos);
    Serial.print(" T:");
    Serial.print(tensile);
    Serial.println();
  }
}

static void move_to(long relative_steps)
{
  long dir = (relative_steps > 0) ? 1 : -1;
  long abs_steps = abs(relative_steps);

  const long DELTA_STEPS = 50;

  while (abs_steps > 0)
  {
    long remaing = min(abs_steps, DELTA_STEPS);

    if (limit_left.is_limit_reached() and dir == 1)
    {
      // Serial.println("left limit switch reached hard limit");
      break;
    }

    if (limit_right.is_limit_reached() and dir == -1)
    {
      // Serial.println("right limit switch reached hard limit");
      break;
    }

    stepper.move_to(remaing * dir);
    abs_steps -= DELTA_STEPS;
    report_values();
  }
}

static void move_to_fast(long relative_steps)
{
  long dir = (relative_steps > 0) ? 1 : -1;
  long abs_steps = abs(relative_steps);

  const long DELTA_STEPS = 50;

  while (abs_steps > 0)
  {
    long remaing = min(abs_steps, DELTA_STEPS);

    if (limit_left.is_limit_reached() and dir == 1)
    {
      // Serial.println("left limit switch reached hard limit");
      break;
    }

    if (limit_right.is_limit_reached() and dir == -1)
    {
      // Serial.println("right limit switch reached hard limit");
      break;
    }

    stepper.move_to_fast(remaing * dir);
    abs_steps -= DELTA_STEPS;
    report_values();
  }
}

static void homing_sequence()
{
  // Serial.println("Tensile tester is Homing . . . . . . . . . . . ");

  while (!limit_right.is_limit_reached())
  {
    stepper.move_to_fast(-10);
  }

  stepper.reset_pos();

  // Serial.println("Tensile Tester homed . . . . . . . . . . . ");
}

static void jog_to_pos(long pos)
{
  float rotations = float(pos) * MM_TO_ROTATION;
  long steps = long(rotations * float(STEPS_PER_ROTATION) + 0.5f);

  move_to_fast(steps - stepper.pos());
}

static bool babystep_rel(float rel_move)
{
  float rotations = rel_move * MM_TO_ROTATION;
  long steps = long(rotations * float(STEPS_PER_ROTATION) + 0.5f);

  if (abs(steps) > 0)
  {
    move_to(steps);
    return true;
  }

  return false;
}

void setup()
{
  Serial.begin(115200);

  Serial.println("setup");
  Serial.setTimeout(1);
  delay(500);

  stepper.enable();
  load_cell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop()
{
  report_values();

  if (is_testing)
  {
    float delta = millis() - prev_time;
    float distance = speed * (delta / 1000.0);

    if (babystep_rel(-distance))
    {
      prev_time = millis();
    }

    long pos_steps = stepper.pos();
    float pos = float(pos_steps) / float(STEPS_PER_ROTATION) / float(MM_TO_ROTATION);

    if (pos < max_pos + 5.0)
    {
      is_testing = false;
      Serial.println("\nok");
    }
  }
  else
  {
    prev_time = millis();
  }

  if (serial_buffer.read_line())
  {
    auto line_ptr = serial_buffer.get_recv_buffer();

    if (memcmp(line_ptr, "M0", 2) == 0)
    {
      Serial.println("\nok");
    }

    // jog
    if (memcmp(line_ptr, "G0 X", 4) == 0)
    {
      long to_pos = atol(&line_ptr[4]);
      jog_to_pos(to_pos);
      Serial.println("\nok");
    }

    // start tensile test
    if (memcmp(line_ptr, "M700 S", 6) == 0)
    {
      speed = atof(&line_ptr[6]);
      is_testing = true;
    }

    // homing sequence command
    if (memcmp(line_ptr, "G28", 3) == 0)
    {
      is_testing = false;
      homing_sequence();
      delay(100);
      Serial.println("\nok");
    }

    serial_buffer.clear();
  }
}
