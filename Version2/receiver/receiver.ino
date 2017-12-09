// ---------- Configuration ----------
// Input pin, must be interrupt-capable pin
const int input_pin = 2;
// Input inversion: 0 (laser high-active) or 1 (laser low-active, e.g. using pull-up)
const int invert_input = 1;
// Period time in microseconds as power of 2, e.g. 8 -> 2^8 us = 256 us
// Must match sender configuration!
const int period_time_micros_power = 7; // tested with 7, 8, 9
// -----------------------------------

const uint8_t round_bits = 6;
const uint8_t valid_bits = period_time_micros_power - round_bits;
const unsigned long round_pattern = (1ul << round_bits) - 1;
const unsigned long valid_pattern = ((1ul << valid_bits) - 1) << round_bits;

volatile unsigned long last_micros = 0;
volatile uint8_t active = 0;
volatile uint16_t xfer_data[2];
volatile uint8_t xfer_num_bits[2];

#define GET_BIT(X, N)  (((X) >> (N)) & 1)

void callback() {
	// Quickly get time and input state
  int input_state = digitalRead(input_pin);
  unsigned long now_micros = micros();
  // Calculate time since last interrupt
  unsigned long delta = now_micros - last_micros;
  last_micros = now_micros;
  // Get number of cycles since last interrupt
  unsigned long rounded = (delta + (1ul << (round_bits - 1))) & ~round_pattern;
  int num_cycles = (rounded >> (round_bits + valid_bits));
  bool valid = ((rounded & valid_pattern) == 0) && (num_cycles <= 10);

  if (!valid) {
    xfer_data[active] = 0;
    xfer_num_bits[active] = 1;
	  return;
  }

  if (xfer_num_bits[active] > 0) {
    xfer_data[active] = xfer_data[active] << num_cycles;
    if (input_state == invert_input) {
      xfer_data[active] |= ((1 << num_cycles) - 1);
    }
    xfer_num_bits[active] += num_cycles;
  }
  else {
    xfer_num_bits[active] = 1;
  }

  if (xfer_num_bits[active] >= 11) {
    active ^= 1;
    xfer_data[active] = 0;
    xfer_num_bits[active] = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(input_pin, INPUT);
  active = 0;
  xfer_data[0] = 0;
  xfer_data[1] = 0;
  xfer_num_bits[0] = 0;
  xfer_num_bits[1] = 0;
  attachInterrupt(digitalPinToInterrupt(input_pin), callback, CHANGE);
}

uint8_t receive_data() {
  uint8_t last_active = active;
  while (last_active == active)
    delayMicroseconds(1);
  return (xfer_data[last_active] >> 1) & 0xff;
}

uint8_t last_data = 0;
bool first = true;
unsigned long num_recv = 0;
unsigned long num_err = 0;
void loop() {
  uint8_t data = receive_data();
  num_recv++;
  if (!first) {
    if ((data != (last_data + 1)) && (data!=0 || last_data!=0xff)) {
      num_err++;
    }
  }
  else {
    first = false;
  }
  if ((num_recv % 1000) == 0) {
    Serial.print("Received ");
    Serial.print(num_recv);
    Serial.print(", Errors ");
    Serial.println(num_err);
  }
  last_data = data;
}

/*void loop() {
  uint8_t data = receive_data();
  Serial.print(data, HEX);
  Serial.print("   ");
  Serial.println(data, BIN);
}*/

