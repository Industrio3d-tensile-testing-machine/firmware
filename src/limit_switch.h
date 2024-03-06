#include <Arduino.h>

class LimitSwitch
{
public:
  LimitSwitch(byte pin) : m_pin(pin)
  {
    pinMode(pin, INPUT_PULLUP);
  }

  bool is_limit_reached()
  {
    return analogRead(m_pin) > 128;
  }

private:
  byte m_pin;
};