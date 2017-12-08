// ---------- Configuration ----------
// Output pin
const int output_pin = 2;
// Output inversion: 0 (laser high-active) or 1 (laser low-active)
const int invert_output = 1;
// Period time in microseconds as power of 2, e.g. 8 -> 2^8 us = 256 us
const int period_time_micros_power = 8;
// -----------------------------------

#include "TimerOne.h"

volatile uint16_t xfer_data = 0;
volatile uint8_t xfer_num_bits = 0;

#define GET_BIT(X, N)  (((X) >> (N)) & 1)

void callback()
{
  if (xfer_num_bits > 0) {
    digitalWrite(output_pin, GET_BIT(xfer_data, xfer_num_bits - 1) == invert_output ? LOW : HIGH);
    xfer_num_bits--;
  }
}

void setup() {
  //Serial.begin(115200);
  pinMode(output_pin, OUTPUT);
  digitalWrite(output_pin, invert_output ? HIGH : LOW);

  Timer1.initialize(1ul << period_time_micros_power);
  Timer1.attachInterrupt(callback); 
}

void send_data(uint8_t data) {
  xfer_data = (data << 2) | 0b10000000010;
  noInterrupts();
  xfer_num_bits = 11;
  interrupts();
  while (xfer_num_bits > 0)
    delayMicroseconds(10);
}

uint8_t value = 0;
void loop() {
  send_data(value);
  value+=1;
  delay(100);
}
