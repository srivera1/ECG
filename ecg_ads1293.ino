/*

   Simplest 3-Lead 24-bit ECG with an ads1293 and arduino Mega 2560

   Author: Sergio Rivera
   Copyright (c) 2018 Sergio Rivera

  This software is licensed under the MIT License(http://opensource.org/licenses/MIT).

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  https://github.com/srivera1


  SPI conection:

  ads1293     Mega2560
  DRDYB       D6
  SDO         D50 (slave out - master in )
  SDI         D51 (slave in  - master out)
  SCLK        D52
  CSB         D53
  ALARMB      -

*/

#include <SPI.h>

const int pin_DRDYB = 6;  // data ready
const int pin_SS    = 53; // CSB

void setup() {
  pinMode(pin_DRDYB, INPUT);
  pinMode(pin_SS,   OUTPUT);

  Serial.begin(9600); // (less than 115200 will decimate the signal -> de facto LPF)

  SPI.begin();
  setup_ECG();
}

int32_t ecgTmp = 0;
int32_t ecgTmp2 = 0;

void loop() {
  if (digitalRead(pin_DRDYB) == false) {

    int32_t ecgVal;

    // sampled data is located at 3 8-bit registers
    byte x1 = readRegister(0x37);
    byte x2 = readRegister(0x38);
    byte x3 = readRegister(0x39);

    // 3 8-bit registers combination on a 24 bit number
    ecgVal = x1;
    ecgVal = (ecgVal << 8) | x2;
    ecgVal = (ecgVal << 8) | x3;

    // exponential smoothing as LPF
    //    1) short range smoothing
    ecgTmp = ecgTmp * .5 + ecgVal * .5;
    //    2) Baseline
    ecgTmp2 = ecgTmp2 * .90 + ecgVal * .10;

    Serial.println(ecgTmp - ecgTmp2);
  }
}

void setup_ECG() { // datasheet ads1293
  //Follow the next steps to configure the device for this example, starting from default registers values.
  //1. Set address 0x01 = 0x11: Connect channel 1’s INP to IN2 and INN to IN1.
  writeRegister(0x01, 0x11);
  //2. Set address 0x02 = 0x19: Connect channel 2’s INP to IN3 and INN to IN1.
  writeRegister(0x02, 0x19);
  //3. Set address 0x0A = 0x07: Enable the common-mode detector on input pins IN1, IN2 and IN3.
  writeRegister(0x0A, 0x07);
  //4. Set address 0x0C = 0x04: Connect the output of the RLD amplifier internally to pin IN4.
  writeRegister(0x0C, 0x04);
  //5. Set address 0x12 = 0x04: Use external crystal and feed the internal oscillator's output to the digital.
  writeRegister(0x12, 0x04);
  //6. Set address 0x14 = 0x24: Shuts down unused channel 3’s signal path.
  writeRegister(0x14, 0x24);
  //7. Set address 0x21 = 0x02: Configures the R2 decimation rate as 5 for all channels.
  writeRegister(0x21, 0x02);
  //8. Set address 0x22 = 0x02: Configures the R3 decimation rate as 6 for channel 1.
  writeRegister(0x22, 0x02);
  //9. Set address 0x23 = 0x02: Configures the R3 decimation rate as 6 for channel 2.
  writeRegister(0x23, 0x02);
  //10. Set address 0x27 = 0x08: Configures the DRDYB source to channel 1 ECG (or fastest channel).
  writeRegister(0x27, 0x08);
  //11. Set address 0x2F = 0x30: Enables channel 1 ECG and channel 2 ECG for loop read-back mode.
  writeRegister(0x2F, 0x30);
  //12. Set address 0x00 = 0x01: Starts data conversion.
  writeRegister(0x00, 0x01);
}

byte readRegister(byte reg) {
  byte data;
  reg |= 1 << 7;
  digitalWrite(pin_SS, LOW);
  SPI.transfer(reg);
  data = SPI.transfer(0);
  digitalWrite(pin_SS, HIGH);
  return data;
}

void writeRegister(byte reg, byte data) {
  reg &= ~(1 << 7);
  digitalWrite(pin_SS, LOW);
  SPI.transfer(reg);
  SPI.transfer(data);
  digitalWrite(pin_SS, HIGH);
}

