#include "mbed.h"
#include "FXOS8700.h"
#include "Hexi_OLED_SSD1351.h"
#include "images.h"
#include "string.h"
#include "time.h"
#include "Hexi_KW40Z.h"

#define LED_ON      0
#define LED_OFF     1
   
void StartHaptic(void);
void StopHaptic(void const *n);

DigitalOut redLed(LED1);
DigitalOut greenLed(LED2);
DigitalOut blueLed(LED3);
DigitalOut haptic(PTB9);

// Pin connections
DigitalOut led1(LED_GREEN); // RGB LED
Serial pc(USBTX, USBRX); // Serial interface
FXOS8700 accel(PTC11, PTC10);
FXOS8700 mag(PTC11, PTC10);
SSD1351 oled(PTB22,PTB21,PTC13,PTB20,PTE6, PTD15); // SSD1351 OLED Driver (MOSI,SCLK,POWER,CS,RST,DC)
KW40Z kw40z_device(PTE24, PTE25);
RtosTimer hapticTimer(StopHaptic, osTimerOnce);

// Variables
float accel_data[3]; // Storage for the data from the sensor
float accel_rms=0.0; // RMS value from the sensor
float mag_data[3];   float mag_rms=0.0;
float ax, ay, az; // Integer value from the sensor to be displayed
// const uint8_t *image1; // Pointer for the image1 to be displayed
char text1[20]; // Text Buffer for dynamic value displayed
char text2[20]; // Text Buffer for dynamic value displayed
char text3[20]; // Text Buffer for dynamic value displayed
int flag = 0;
int fallDetected = 0;

void ButtonUp(void)
{
    StartHaptic();
    
    redLed      = LED_ON;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;
}

void ButtonDown(void)
{
    StartHaptic();

    redLed      = LED_OFF;
    greenLed    = LED_ON;
    blueLed     = LED_OFF;
}

void ButtonRight(void)
{
    StartHaptic();
    fallDetected = 0;
    if(flag == 0 ){
        redLed      = LED_OFF;
        greenLed    = LED_ON;
        blueLed     = LED_OFF;
    }
    else if(flag == 1 ){
        redLed      = LED_ON;
        greenLed    = LED_ON;
        blueLed     = LED_OFF;
    }
    else{
        redLed      = LED_ON;
        greenLed    = LED_OFF;
        blueLed     = LED_OFF;
    }
    
    // oled.FillScreen(COLOR_BLACK);
}

void ButtonLeft(void)
{
    StartHaptic();
    
    redLed      = LED_OFF;
    greenLed    = LED_ON;
    blueLed     = LED_OFF;
}

void ButtonSlide(void)
{
    // StartHaptic();
    
    redLed      = LED_ON;
    greenLed    = LED_ON;
    blueLed     = LED_ON;
}

void StartHaptic(void)
{
    hapticTimer.start(75);
    haptic = 1;
}

void StopHaptic(void const *n) {
    haptic = 0;
    hapticTimer.stop();
    redLed      = LED_OFF;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;
}

void textPropertyChange(uint16_t color, char * str){
    printf("%d",color);
    oled_text_properties_t textProperties = {0};
    oled.GetTextProperties(&textProperties); 

    textProperties.fontColor = color;
    textProperties.alignParam = OLED_TEXT_ALIGN_CENTER;
    oled.SetTextProperties(&textProperties);  
    strcpy((char *) text1, str);
    oled.Label((uint8_t *)text1,3,45);
}

int main() {
        
    // Configure Accelerometer FXOS8700, Magnetometer FXOS8700
    accel.accel_config();
    mag.mag_config();

    // Fill 96px by 96px Screen with 96px by 96px Image starting at x=0,y=0
    oled.FillScreen(COLOR_BLACK);

    // kw40z_device.attach_buttonUp(&ButtonUp);
    // kw40z_device.attach_buttonDown(&ButtonDown);
    // kw40z_device.attach_buttonLeft(&ButtonLeft);
    // kw40z_device.attach_buttonRight(&ButtonRight);

    while (true) 
    {
      
      accel.acquire_accel_data_g(accel_data);
      accel_rms = sqrt(((accel_data[0]*accel_data[0])+(accel_data[1]*accel_data[1])+(accel_data[2]*accel_data[2]))/3);

      ax = accel_data[0];
      ay = accel_data[1];
      az = accel_data[2];             
      mag.acquire_mag_data_uT(mag_data);
      
      if(accel_data[0]>=0.99 && mag_data[1]<=-15.0){
          fallDetected = 1;
          oled_text_properties_t optionProperties = {0};
          oled.GetTextProperties(&optionProperties); 
          optionProperties.fontColor = COLOR_WHITE;
          optionProperties.alignParam = OLED_TEXT_ALIGN_RIGHT;
          strcpy((char *) text3,"YES");
          oled.TextBox((uint8_t *)text3,70,79,20,15);
      }

      if(fallDetected){
          
          if(flag == 0){ //I am fine
                textPropertyChange(COLOR_GREEN, (char *)"      I am fine      ");
                flag = 1;
                // ThisThread::sleep_for(2500);
            }
            else if(flag == 1){ //Notify Friend
                textPropertyChange(COLOR_YELLOW, (char *)"  Notify Friend  ");
                flag = 2;
                // ThisThread::sleep_for(2500);
            }
            else { //SOS call
                textPropertyChange(COLOR_RED, (char *)"      SOS Call      ");
                flag = 0;
                
            }
            ThisThread::sleep_for(2500);
            kw40z_device.attach_buttonRight(&ButtonRight);

      }
      kw40z_device.attach_buttonLeft(&ButtonLeft);
      
    }
}