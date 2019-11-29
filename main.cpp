/* 
* Authors : Meghna R Holla (B00812604) and Gaganpreet Singh ()
* Course  : Ubiquitous Computing (CSCI 6609)
* A "Fall Detection" program to help monitor the health of trekkers and notify their friends/EHS in case of emergencies.
 */
#include "FXOS8700.h"
#include "Hexi_KW40Z.h"
#include "Hexi_OLED_SSD1351.h"
#include "images.h"
#include "mbed.h"
#include "string.h"
#include "time.h"

// LED Light Connection
#define LED_ON 0
#define LED_OFF 1
DigitalOut redLed(LED1);
DigitalOut greenLed(LED2);
DigitalOut blueLed(LED3);


// Pin connections
Serial pc(USBTX, USBRX); // Serial interface
FXOS8700 accel(PTC11, PTC10);
FXOS8700 mag(PTC11, PTC10);
SSD1351 oled(PTB22, PTB21, PTC13, PTB20, PTE6,
             PTD15); // SSD1351 OLED Driver (MOSI,SCLK,POWER,CS,RST,DC)
KW40Z kw40z_device(PTE24, PTE25);

//Timer set-up
Timer t;

void welcomeScreen(void);
void fallTriggered(void);
void prompt (void);
void timeOutAutomaticSoSCall (void);
void timerStart(void);
void timerStop(void);

// Storage for the data from the sensor - Accelerometer and Magnetometer
float accel_data[3];   
float mag_data[3];

// Text Buffer for dynamic value displayed
char text1[20]; 
char text2[20]; 
char text3[20]; 

//Flags 
int flag = 0;
int newflag = 0;
int timeFlag = false;
bool fallDetected = false;
bool promptScrn = false;

//Start time variable
float startTime;


//Starting Screen 
void welcomeScreen() {
  flag = 0;
  redLed = LED_OFF;
  greenLed = LED_OFF;
  blueLed = LED_OFF;
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

//Start Timer
void timerStart(){
    t.start();
    startTime = t.read();
    timeFlag = true;
}

//Stop Timer
void timerStop(){
    t.reset();
    t.stop();
    timeFlag = false;
}

//Fall Detection screen with static text
void fallTriggered() {
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

//Changing prompt texts in a loop
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
  ThisThread::sleep_for(3000);
  //Reset all variables
  oled.FillScreen(COLOR_BLACK);
  fallDetected = false;
  promptScrn = false;
  flag = 0;
  newflag = 0;
  redLed = LED_OFF;
  greenLed = LED_OFF;
  blueLed = LED_OFF;
  t.stop(); 
}



void ButtonRightPressed(void) {

  
  if (flag == 0) { //I am fine 
    redLed = LED_OFF;
    greenLed = LED_ON;
    blueLed = LED_OFF;
  } else if (flag == 1) { //Notify friend
    redLed = LED_OFF;
    greenLed = LED_OFF;
    blueLed = LED_ON;
  } else { //SOS call
    redLed = LED_ON;
    greenLed = LED_OFF;
    blueLed = LED_OFF;
  }
  
  //display prompt messages
  prompt(); 
}

//Method to show that a call has been connected or prompt has been exited
void connected(void){
  ThisThread::sleep_for(3000);
  oled.FillScreen(COLOR_BLACK);
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_CYAN;
  oled.SetTextProperties(&textProperties);
  if(newflag==1){
      strcpy((char *)text1, "Exiting...");
      oled.Label((uint8_t *)text1, 8, 4);
  }
  else{
      strcpy((char *)text1, "Call Connected!");
      oled.Label((uint8_t *)text1, 8, 4);
  }

  reset();
}

//Method to get out of a prompt screen
void promptdeclined(void){
    reset();
}

//Method to display a false alarm or to show that the device is triggering a call
void promptpressed(void){
  oled.FillScreen(COLOR_BLACK);
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_CYAN;
  oled.SetTextProperties(&textProperties);
  
  if(newflag ==1){
    strcpy((char *)text1, "False Alarm");
    oled.Label((uint8_t *)text1, 8, 4);
  }
  else{
    strcpy((char *)text1, "Calling...");
    oled.Label((uint8_t *)text1, 8, 4);
  }

  connected();  
}

//Prompt Screen
void prompt(void){

  oled.FillScreen(COLOR_BLACK);
  promptScrn = true;
    /* Get OLED Class Default Text Properties */
  oled_text_properties_t textProperties = {0};
  oled.GetTextProperties(&textProperties);

  /* Set text properties to white and right aligned for the dynamic text */
  textProperties.fontColor = COLOR_CYAN;
  oled.SetTextProperties(&textProperties);

  /* Display Legends */
  strcpy((char *)text1, "Are you sure");
  oled.Label((uint8_t *)text1, 8, 4);
  if (flag == 0) {
    strcpy((char *)text1, "you are fine?");
    oled.Label((uint8_t *)text1, 25, 18);
  } else if (flag == 1) {
    strcpy((char *)text1, "call friend?");
    oled.Label((uint8_t *)text1, 25, 18);
  } else {
    strcpy((char *)text1, "call EHS?");
    oled.Label((uint8_t *)text1, 25, 18);
  }

  textProperties.fontColor = COLOR_WHITE;
  oled.SetTextProperties(&textProperties);
  strcpy((char *)text3, "YES");
  oled.Label((uint8_t *)text3, 70, 79);
  oled.TextBox((uint8_t *)" ", 1, 38, 94, 30);
  strcpy((char *)text3, "NO");
  oled.Label((uint8_t *)text3, 10, 79);
  oled.TextBox((uint8_t *)" ", 1, 38, 94, 30);
    
}

//Method called after 20 seconds of the timer running
void timeOutAutomaticSoSCall(void){
    oled.FillScreen(COLOR_BLACK);
    oled_text_properties_t textProperties = {0};
    oled.GetTextProperties(&textProperties);
    redLed = LED_ON;
    greenLed = LED_OFF;
    blueLed = LED_OFF;
    /* Set text properties to white and right aligned for the dynamic text */
    textProperties.fontColor = COLOR_RED;
    oled.SetTextProperties(&textProperties);

    strcpy((char *)text1, "Calling SOS");
    oled.Label((uint8_t *)text1, 8, 4);
    connected();
}

int main() {

  // Configure Accelerometer FXOS8700, Magnetometer FXOS8700
  accel.accel_config();
  mag.mag_config();
  
  // Fill 96px by 96px Screen with 96px by 96px Image starting at x=0,y=0
  oled.FillScreen(COLOR_BLACK);
  
  //timer started
  timerStart();

  while (true) {
    
    accel.acquire_accel_data_g(accel_data);
    mag.acquire_mag_data_uT(mag_data);
    
    //Threshold value for fall detection
    if (accel_data[0] >= 0.99 && mag_data[1] <= -10.0) {
      fallDetected = true; //set fall to true
    }


    if (!fallDetected) {  //when fall is not detected
      welcomeScreen();
    }
    else if(!promptScrn) {  //when fall is detected and options aren't selected
     
      fallTriggered();
      if(!timeFlag){
        timerStart();
      }

      //Different options being displayed
      if (flag == 0) {
        // I am fine
        textPropertyChange(COLOR_GREEN, (char *)"    I am fine!  ");
        newflag = 1;
        
      } else if (flag == 1) {
        // Notify Friend
        textPropertyChange(COLOR_BLUE, (char *)"Notify Friend?");
        newflag = 2;
      } else {
        // SOS call
        textPropertyChange(COLOR_RED, (char *)"    Call SoS?  ");
        newflag = 0;
      }
      if(t.read() - startTime > 20.0){
          timerStop();
          timeOutAutomaticSoSCall();
      }

      //Option 'YES' that leads to more prompts
      kw40z_device.attach_buttonRight(&ButtonRightPressed);
      ThisThread::sleep_for(2500);

    // t.reset();// This reset the timer
    }
    flag = newflag;
    if(promptScrn){
        timerStop();
        
        //Option 'YES' after the prompt screen is displayed
        kw40z_device.attach_buttonRight(&promptpressed);

        //Option 'NO' after the prompt screen is displayed
        kw40z_device.attach_buttonLeft(&promptdeclined);
    }

    //Reset button
    kw40z_device.attach_buttonUp(&reset);
    
  }
  
}