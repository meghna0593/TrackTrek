#include "FXOS8700.h"
#include "Hexi_KW40Z.h"
#include "Hexi_OLED_SSD1351.h"
#include "images.h"
#include "mbed.h"
#include "string.h"
#include "time.h"

#define LED_ON 0
#define LED_OFF 1

void StartHaptic(void);
void StopHaptic(void const *n);

DigitalOut redLed(LED1);
DigitalOut greenLed(LED2);
DigitalOut blueLed(LED3);

DigitalOut haptic(PTB9);

// Pin connections
Serial pc(USBTX, USBRX); // Serial interface
FXOS8700 accel(PTC11, PTC10);
FXOS8700 mag(PTC11, PTC10);
SSD1351 oled(PTB22, PTB21, PTC13, PTB20, PTE6,
             PTD15); // SSD1351 OLED Driver (MOSI,SCLK,POWER,CS,RST,DC)
KW40Z kw40z_device(PTE24, PTE25);
RtosTimer hapticTimer(StopHaptic, osTimerOnce);

// Variables
float accel_data[3];   // Storage for the data from the sensor
float accel_rms = 0.0; // RMS value from the sensor
float mag_data[3];
float mag_rms = 0.0;
float ax, ay, az; // Integer value from the sensor to be displayed
// const uint8_t *image1; // Pointer for the image1 to be displayed
char text1[20]; // Text Buffer for dynamic value displayed
char text2[20]; // Text Buffer for dynamic value displayed
char text3[20]; // Text Buffer for dynamic value displayed
int flag = 0;
int newflag = 0;
bool fallDetected = false;

void StartHaptic(void) {
  hapticTimer.start(75);
  haptic = 1;
}

void StopHaptic(void const *n) {
  haptic = 0;
  hapticTimer.stop();
  redLed = LED_OFF;
  greenLed = LED_OFF;
  blueLed = LED_OFF;
}

void welcome() {
  flag = 0;
  /* Get OLED Class Default Text Properties */
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_WHITE;
  oled.SetTextProperties(&textProperties);

  /* Display Legends */
  strcpy((char *)text1, "TrackTrek");
  oled.Label((uint8_t *)text1, 20, 4);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_GREEN;
  oled.SetTextProperties(&textProperties);

  /* Display Legends */
  strcpy((char *)text2, "Your Trekking");
  oled.Label((uint8_t *)text2, 12, 38);
  strcpy((char *)text2, "Partner");
  oled.Label((uint8_t *)text2, 30, 52);
}

void fall() {
  /* Get OLED Class Default Text Properties */
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_CYAN;
  oled.SetTextProperties(&textProperties);

  /* Display Legends */
  strcpy((char *)text1, "A fall has been");
  oled.Label((uint8_t *)text1, 8, 4);
  strcpy((char *)text1, "Detected");
  oled.Label((uint8_t *)text1, 25, 18);

  textProperties.fontColor = COLOR_WHITE;
  oled.SetTextProperties(&textProperties);
  strcpy((char *)text3, "YES");
  oled.Label((uint8_t *)text3, 70, 79);
  oled.TextBox((uint8_t *)" ", 1, 38, 94, 30);
}

void textPropertyChange(uint16_t color, char *str) {
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  textProperties.fontColor = color;
  textProperties.alignParam = OLED_TEXT_ALIGN_CENTER;
  oled.SetTextProperties(&textProperties);
  strcpy((char *)text1, str);
  oled.Label((uint8_t *)text1, 12, 45);
}

void reset(void) {  
  oled.FillScreen(COLOR_BLACK);
  welcome();
  fallDetected = false;
  flag = 0;
  newflag = 0;
  redLed = LED_OFF;
  greenLed = LED_OFF;
  blueLed = LED_OFF;
}

void ButtonRightPressed(void) {
  StartHaptic();
  if (flag == 0) {
    redLed = LED_OFF;
    greenLed = LED_ON;
    blueLed = LED_OFF;
  } else if (flag == 1) {
    redLed = LED_ON;
    greenLed = LED_OFF;
    blueLed = LED_OFF;
  } else {
    redLed = LED_OFF;
    greenLed = LED_OFF;
    blueLed = LED_ON;
  }
//   wait(30);
}

int main() {

  // Fill 96px by 96px Screen with 96px by 96px Image starting at x=0,y=0
  oled.FillScreen(COLOR_BLACK);

  while (true) {

    accel.acquire_accel_data_g(accel_data);
    accel_rms = sqrt(((accel_data[0] * accel_data[0]) +
                      (accel_data[1] * accel_data[1]) +
                      (accel_data[2] * accel_data[2])) /
                     3);

    ax = accel_data[0];
    ay = accel_data[1];
    az = accel_data[2];
    mag.acquire_mag_data_uT(mag_data);
    
    if (accel_data[0] >= 0.99 && mag_data[1] <= -10.0) {
      fallDetected = true;
    }

    if (!fallDetected) {
      welcome();
    }
    else {
      fall();

      if (flag == 0) {
        // I am fine
        textPropertyChange(COLOR_GREEN, (char *)"    I am fine!  ");
        newflag = 1;
      } else if (flag == 1) {
        // Notify Friend
        textPropertyChange(COLOR_RED, (char *)"Notify Friend?");
        newflag = 2;
      } else {
        // SOS call
        textPropertyChange(COLOR_BLUE, (char *)"    Call SoS?  ");
        newflag = 0;
      }
      kw40z_device.attach_buttonRight(&ButtonRightPressed);
      ThisThread::sleep_for(2500);
    }
    flag = newflag;
    kw40z_device.attach_buttonLeft(&reset);
  }
  
}