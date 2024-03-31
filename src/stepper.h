#ifndef STEPPER_SWITCH_H_INCLUDED
#define STEPPER_SWITCH_H_INCLUDED

#include <Arduino.h>

class Stepper
{
public:
  Stepper(byte en_pin, byte dir_pin, byte stp_pin)
      : m_en_pin(en_pin), m_dir_pin(dir_pin), m_stp_pin(stp_pin), m_pos(0)
  {
    pinMode(en_pin, OUTPUT);
    pinMode(dir_pin, OUTPUT);
    pinMode(stp_pin, OUTPUT);
    disable();
  }

  void move_to(long relative_steps)
  {
    digitalWrite(m_dir_pin, relative_steps < 0);

    for (int i = 0; i < abs(relative_steps); i++)
    {
      digitalWrite(m_stp_pin, HIGH);
      delayMicroseconds(1250);
      digitalWrite(m_stp_pin, LOW);
      delayMicroseconds(1250);
    }

    m_pos += relative_steps;
  }
  void move_to_fast(long relative_steps)
  {
    digitalWrite(m_dir_pin, relative_steps < 0);

    for (int i = 0; i < abs(relative_steps); i++)
    {
      digitalWrite(m_stp_pin, HIGH);
      delayMicroseconds(620);
      digitalWrite(m_stp_pin, LOW);
      delayMicroseconds(620);
    }

    m_pos += relative_steps;
  }

  void reset_pos()
  {
    m_pos = 0;
  }

  void enable()
  {
    digitalWrite(m_en_pin, LOW);
  }

  void disable()
  {
    digitalWrite(m_en_pin, HIGH);
  }

  long pos() const
  {
    return m_pos;
  }

private:
  byte m_en_pin;
  byte m_dir_pin;
  byte m_stp_pin;
  long m_pos;
};

#endif // !STEPPER_SWITCH_H_INCLUDED