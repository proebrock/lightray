#include "TimerOne.h"

int output_pin = 2;
int invert_output = 1; // 0 (laser high-active) or 1 (laser low-active)

const unsigned long period_time_micros = 500; // min: around 380

volatile uint64_t xfer_data = 0;
volatile uint8_t xfer_num_bits = 0;

#define GET_BIT(X, N)  (((X) >> (N)) & 1)

void callback()
{
  if (xfer_num_bits > 0) {
    digitalWrite(output_pin, (xfer_data & 1) == invert_output ? LOW : HIGH);
    xfer_data = xfer_data >> 1;
    xfer_num_bits--;
  }
}

void setup() {
  //Serial.begin(115200);
  pinMode(output_pin, OUTPUT);
  digitalWrite(output_pin, invert_output ? HIGH : LOW);

  Timer1.initialize(period_time_micros);
  Timer1.attachInterrupt(callback); 
}

// Input: 16 bits of data, output: 36 bits of data including start+stop bits
uint64_t manchester_encode(uint16_t data) {
  //Serial.println((uint32_t)(data >> 0), BIN);
  uint64_t result = (1ull << 34) | (1ull << 1);
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t offs = GET_BIT(data, i);
    result |= 1ull << (2 + 2 * i + offs);
  }
  //Serial.print((uint32_t)(result >> 32), BIN);
  //Serial.print(",");
  //Serial.println((uint32_t)(result >> 0), BIN);
  return result;
}

// Input: 4 bits of data, output: 8 bits of data
uint8_t hamming_encode_nibble(uint8_t input) {
  // Extract bits
  uint8_t d0 = GET_BIT(input, 0);
  uint8_t d1 = GET_BIT(input, 1);
  uint8_t d2 = GET_BIT(input, 2);
  uint8_t d3 = GET_BIT(input, 3);
  // Calculate hamming bits
  uint8_t h0 = (d1 + d2 + d3) & 1;
  uint8_t h1 = (d0 + d2 + d3) & 1;
  uint8_t h2 = (d0 + d1 + d3) & 1;
  // Calculate overall parity
  uint8_t p = d0 ^ d1 ^ d2 ^ d3 ^ h0 ^ h1 ^ h2;
  // Put everything together
  return (p << 7) | (h2 << 6) | (h1 << 5) | (h0 << 4) | (input & 0xf);
}

// Input: 8 bits of data, output: 16 bits of data
uint16_t hamming_encode_byte(uint8_t input) {
  return (hamming_encode_nibble((input >> 4) & 0xf) << 8) | hamming_encode_nibble(input & 0xf);
}

void send_data(uint8_t data) {
  uint16_t data_hamming = hamming_encode_byte(data);
  xfer_data = manchester_encode(data_hamming);
  xfer_num_bits = 36;
  while (xfer_num_bits > 0)
    delayMicroseconds(10);
}

uint8_t value = 0;
void loop() {
  send_data(value);
  value++;
  delay(500);
}
