/*

  Biblioteca de comunicação com o Wii Nunchuk baseada em
  http://todbot.com/blog/bionicarduino/

*/

// Nunchuk I2C address
#define NUNCHUK_ADDRESS 0x52

#include <Arduino.h>

// Stores nunchuk data.
static uint8_t nunchuk_buf[6];

// Uses port C (analog in) pins as power & ground for Nunchuk.
static void nunchuk_setpowerpins() {
#define pwrpin PORTC3 // pin A2
#define gndpin PORTC2 // pin A3
  DDRC |= _BV(pwrpin) | _BV(gndpin);
  PORTC &= ~ _BV(gndpin);
  PORTC |=  _BV(pwrpin);
  delay(100);  // wait for things to stabilize.
}

// Initialize the I2C system, join the I2C bus,
// and tell the Nunchuk we're talking to it.
static void nunchuk_init() {
  Wire.begin();
  Wire.beginTransmission(NUNCHUK_ADDRESS);
  Wire.write((uint8_t)0x40);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
}

// Send a request for data to the nunchuk.
static void nunchuk_send_request() {
  Wire.beginTransmission(NUNCHUK_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
}

// Encode data to format that most wiimote drivers except
// only needed if you use one of the regular wiimote drivers
static char nunchuk_decode_byte (char x) {
  x = (x ^ 0x17) + 0x17;
  return x;
}

// Receive data back from the nunchuk.
// Returns true on successful read and false on failure.
static bool nunchuk_get_data() {
  int cnt = 0;
  // Requesting 6 bytes from Nunchuk.
  Wire.requestFrom(NUNCHUK_ADDRESS, 6);
  while (Wire.available()) {
    nunchuk_buf[cnt] = nunchuk_decode_byte(Wire.read());
    cnt++;
  }
  nunchuk_send_request();  // send request for next data payload
  // Check if we received 6 bytes.
  return (cnt >= 5);
}

// Returns ZButton state: 
// True = pressed; False = notpressed
static bool nunchuk_zbutton() {
  return ((nunchuk_buf[5] >> 0) & 1) ? 0 : 1;
}

// Returns CButton state: 
// True = pressed; False = notpressed
static bool nunchuk_cbutton() {
  return ((nunchuk_buf[5] >> 1) & 1) ? 0 : 1;
}

// Returns value of x-axis joystick.
static uint8_t nunchuk_joyx() {
  return nunchuk_buf[0];
}

// Returns value of y-axis joystick.
static uint8_t nunchuk_joyy() {
  return nunchuk_buf[1];
}

// Returns value of x-axis accelerometer.
static uint16_t  nunchuk_accelx() {
  return ((uint16_t) nunchuk_buf[2] << 2) | ((nunchuk_buf[5] >> 2) & 3);
}

// Returns value of y-axis accelerometer.
static uint16_t nunchuk_accely() {
  return ((uint16_t) nunchuk_buf[3] << 2) | ((nunchuk_buf[5] >> 4) & 3);
}

// Returns value of z-axis accelerometer.
static uint16_t nunchuk_accelz() {
  return ((uint16_t) nunchuk_buf[4] << 2) | ((nunchuk_buf[5] >> 6) & 3);
}

