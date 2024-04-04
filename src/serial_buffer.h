#ifndef SERIAL_BUFFER_H_INCLUDED
#define SERIAL_BUFFER_H_INCLUDED

#include <Arduino.h>

class SerialBuffer
{
public:
  SerialBuffer() : m_recv_index{0}
  {
    memset(m_recv_buffer, 0, RECV_BUFFER_SIZE);
  }

  bool read_line()
  {
    while (Serial.available() > 0)
    {
      int rchar = Serial.read();

      if (rchar <= 0)
      {
        return false;
      }

      // TODO: remove debug
      // Serial.print("$");
      // Serial.println(rchar);

      if (m_recv_index < RECV_BUFFER_SIZE - 1)
      {
        m_recv_buffer[m_recv_index] = (byte)rchar;
        m_recv_index += 1;
        m_recv_buffer[m_recv_index + 1] = 0;
      }

      // ln check
      if (rchar == 10)
      {
        return true;
      }
    }

    return false;
  }

  const char *const get_recv_buffer()
  {
    return m_recv_buffer;
  }

  void clear()
  {
    m_recv_index = 0;
    m_recv_buffer[0] = 0;
  }

private:
  static const byte RECV_BUFFER_SIZE = 32;
  char m_recv_buffer[RECV_BUFFER_SIZE];
  byte m_recv_index;
};

#endif // !SERIAL_BUFFER_H_INCLUDED