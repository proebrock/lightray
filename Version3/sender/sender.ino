#include <Adafruit_BME280.h>

#include "RS_FEC.h"

const int switch_pin = 2;

Adafruit_BME280 sensor;


// Message
uint8_t test_message_index = 0;
const uint8_t msg_len = 16;
char msg_buffer[msg_len + 1]; // zero terminated
// FEC encoded
const uint8_t ecc_len = 16;
RS::ReedSolomon<msg_len, ecc_len> rs;
const uint8_t encoded_len = msg_len + ecc_len;
uint8_t encoded_buffer[encoded_len];
// Final message with start byte and size
const uint8_t START = 0xfc;
const uint8_t ESCAPE = 0xfd;
const uint8_t send_maxlen = 2 + 2 * encoded_len;
uint8_t send_len;
uint8_t send_buffer[send_maxlen];


void setup() {
  Serial.begin(4800);

  pinMode(switch_pin, INPUT);
  //pinMode(pin, INPUT_PULLUP);

  // Initialize sensor
  if (!sensor.begin()) {
    while (true);
  }
}


void loop() {

  // Generate message
  if (digitalRead(switch_pin)) {
    sprintf(msg_buffer, "%i;%i;%i",
      static_cast<int>(round(10.0F * sensor.readTemperature())),
      static_cast<int>(round(sensor.readPressure()) / 10.0F),
      static_cast<int>(round(sensor.readHumidity()))
      );
  }
  else {
    for (uint8_t i = 0; i < msg_len; i++)
      msg_buffer[i] = test_message_index++;
    msg_buffer[msg_len] = '\0';
  }

  // Forward error correction (FEC) encoding
  rs.Encode(msg_buffer, encoded_buffer);

  // Assemble message
  send_buffer[0] = START;
  send_len = 2;
  for (uint8_t i = 0; i < encoded_len; i++) {
    switch (encoded_buffer[i])
    {
      case START:
      case ESCAPE:
      {
        send_buffer[send_len++] = ESCAPE;
        send_buffer[send_len++] = encoded_buffer[i] ^ 0x20;
        break;
      }
      default:
      {
        send_buffer[send_len++] = encoded_buffer[i];
        break;
      }
    }
  }
  send_buffer[1] = send_len;

  // Sending data
  Serial.write(send_buffer, send_len);
  
  if (digitalRead(switch_pin)) {
    delay(2000);
  }
  else {
    delay(500);
  }
}
