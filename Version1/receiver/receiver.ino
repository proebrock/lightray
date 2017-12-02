int input_pin = 2;

const unsigned long period_time_micros = 500;
const unsigned long period_time_delta = 50;
const unsigned long range1_min = 1 * period_time_micros - period_time_delta;
const unsigned long range1_max = 1 * period_time_micros + period_time_delta;
const unsigned long range2_min = 2 * period_time_micros - period_time_delta;
const unsigned long range2_max = 2 * period_time_micros + period_time_delta;

volatile unsigned long last_micros = 0;
volatile uint8_t active = 0;
volatile uint64_t xfer_data[2];
volatile uint8_t xfer_num_bits[2];

#define GET_BIT(X, N)  (((X) >> (N)) & 1)

void callback() {
  // get input state
  uint64_t input_state = (digitalRead(input_pin) == HIGH) ? 1 : 0;
  // Calculate time since last edge detected
  unsigned long now_micros = micros();
  unsigned long delta = now_micros - last_micros;
  last_micros = now_micros;

  if (xfer_num_bits[active] >= 35) {
    active ^= 1;
    xfer_data[active] = 0;
    xfer_num_bits[active] = 0;
  }

  if ((delta > range1_min) && (delta < range1_max)) {
    // One period time since last interrupt
    xfer_data[active] |= (input_state << xfer_num_bits[active]);
    xfer_num_bits[active]++;
  }
  else if ((delta > range2_min) && (delta < range2_max)) {
    // Two periods time since last interrupt
    xfer_data[active] |= (input_state << xfer_num_bits[active]);
    xfer_num_bits[active]++;
    xfer_data[active] |= (input_state << xfer_num_bits[active]);
    xfer_num_bits[active]++;
  }
  else {
    // Neither one nor two periods since last interrupt
    xfer_data[active] = 0;
    xfer_num_bits[active] = 0;
    xfer_data[active] |= (input_state << xfer_num_bits[active]);
    xfer_num_bits[active]++;
    // TODO: count errors!?
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

// Input: 36 bits of data including start+stop bits, output: 16 bits of data
uint8_t manchester_decode(uint64_t input, uint16_t *output) {
  *output = 0;
  for (uint8_t i = 0; i < 16; i++) {
    *output |= GET_BIT(input, 3 + 2 * i) << i;
  }
  return 0; // TODO: Check for errors
}

// Input: 8 bits if data, output: 4 bits of data
uint8_t hamming_decode_nibble(uint8_t input, uint8_t *output) {
  // Extract bits
  uint8_t d0 = GET_BIT(input, 0);
  uint8_t d1 = GET_BIT(input, 1);
  uint8_t d2 = GET_BIT(input, 2);
  uint8_t d3 = GET_BIT(input, 3);
  uint8_t h0 = GET_BIT(input, 4);
  uint8_t h1 = GET_BIT(input, 5);
  uint8_t h2 = GET_BIT(input, 6);
  uint8_t p  = GET_BIT(input, 7);
  // Calculate syndrome
  uint8_t s0 = (d1 + d2 + d3 + h0) & 1;
  uint8_t s1 = (d0 + d2 + d3 + h1) & 1;
  uint8_t s2 = (d0 + d1 + d3 + h2) & 1;
  uint8_t syndrome = (s0 << 2) | (s1 << 1) | s2;
  // Check for error and fix it if necessary
  if (syndrome != 0) {
    const uint8_t syndrome_check[] = { -1, 6, 5, 0, 4, 1, 2, 3 };
    input ^= 1 << syndrome_check[syndrome];
  }
  // Calculate parity
  uint8_t p_ = 0;
  for (int i = 0; i < 7; i++)
    p_ ^= GET_BIT(input, i);
  // Determine result
  *output = input & 0xf;
  if (p_ == p) {
    if (syndrome == 0) {
      // No errors detected
      return 0;
    }
    else {
      // Single error detected and fixed
      return 1;
    }
  }
  else {
    if (syndrome == 0) {
      // Syndrome ok but parity not: Parity bit has error, data still ok
      return 1;
    }
    else {
      // Double error detected, cannot be fixed
      return 2;
    }
  }
  // More than 2 errors do not allow them to be detected or fixed
}

// Input: 16 bits if data, output: 8 bits of data
uint8_t hamming_decode_byte(uint16_t input, uint8_t *output) {
  uint8_t output_msb, output_lsb;
  uint8_t err_msb, err_lsb;

  err_msb = hamming_decode_nibble((input >> 8) & 0xff, &output_msb);
  err_lsb = hamming_decode_nibble(input & 0xff, &output_lsb);

  *output = (output_msb << 4) | output_lsb;

  return (err_msb >= err_lsb) ? err_msb : err_lsb;
}

uint8_t receive_data() {
  uint8_t last_active = active;
  while (last_active == active)
    delayMicroseconds(10);
  uint64_t raw_data = xfer_data[last_active];
  uint16_t manchester_decoded;
  uint8_t err = manchester_decode(raw_data, &manchester_decoded);
  uint8_t hamming_decoded;
  err = hamming_decode_byte(manchester_decoded, &hamming_decoded);
  return hamming_decoded;
}

void loop() {
  uint8_t data = receive_data();
  Serial.println(data);
}

