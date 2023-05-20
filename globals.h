//
// Created by Simone on 08/09/22.
//

#ifndef ELOQUENTSURVEILLANCE_GLOBALS_H
#define ELOQUENTSURVEILLANCE_GLOBALS_H
#define LAMP_PIN           4

#include <esp_camera.h>
#include <WiFi.h>
#include <math.h>

uint16_t gOffset = 0;
camera_fb_t *gFrame;

// extern function from main.ino
typedef void (*FuncPtr)(int, int);
extern FuncPtr sensorPtr;

// all sensor configuration
int vflipStatus = 0;
int hmirrorStatus = 0;
int brightness = 0;

// lamp
int lampChannel = 7;           // a free PWM channel (some channels used by camera)
const int pwmfreq = 50000;     // 50K pwm frequency
const int pwmresolution = 9;   // duty cycle bit range
const int pwmMax = pow(2,pwmresolution)-1;
int lampVal = 0;

//synchronize stream
int streamStatus = 0; // false = motionDetect ON

// motion detect
int motionDetect = 1;

void setLamp(int newVal) {
    if (newVal != -1) {
        // Apply a logarithmic function to the scale.
        int brightness = round((pow(2,(1+(newVal*0.02)))-2)/6*pwmMax);
        ledcWrite(lampChannel, brightness);
        Serial.print("Lamp: ");
        Serial.print(newVal);
        Serial.print("%, pwm = ");
        Serial.println(brightness);
		lampVal = newVal;
    }
}

void setUpLamp() {
	ledcSetup(lampChannel, pwmfreq, pwmresolution);  // configure LED PWM channel
	ledcAttachPin(LAMP_PIN, lampChannel);            // attach the GPIO pin to the channel
	setLamp(lampVal);                      			 // set default value
}

void setSensorData(int id, int val)
{
	if(id==1)
	{
		if(val==0) vflipStatus = 0;
		else if(val==1) vflipStatus = 1;
	}
	else if(id==2)
	{
		if(val==0) hmirrorStatus = 0;
		else if(val==1) hmirrorStatus = 1;
	}
	else if(id==3)
	{
		if(val >= -2 and val <= 2)
		{
			brightness = val;
		}
	}
	else if(id==4)
	{
		setLamp(val);
	}
	else if(id==5)
	{
		if(val==0) streamStatus = 0;
		else if(val==1) streamStatus = 1;
	}
	else if(id==6)
	{
		if(val==0) motionDetect = 0;
		else if(val==1) motionDetect = 1;
	}
}

int getSensorData(int id)
{
	if(id==1) return vflipStatus;
	else if(id==2) return hmirrorStatus;
	else if(id==3) return brightness;
	else if(id==4) return lampVal;
	else if(id==5) return streamStatus;
	else if(id==6) return motionDetect;
}

//----------------------------------------------------------------------------------------------------------
/**
 * Test is there's a frame in memory
 * @return
 */
bool gIsFrame() {
    return gFrame != NULL && gFrame->len > 0;
}


/**
 * Capture frame
 */
bool gCapture() {
    // release old frame first
    if (gFrame) {
        esp_camera_fb_return(gFrame);
        gFrame = NULL;
    }

    gOffset = 0;
    gFrame = esp_camera_fb_get();

    return gIsFrame();
}

/**
 * Release frame
 */
void gFree() {
    if (gFrame) {
        esp_camera_fb_return(gFrame);
        gFrame = NULL;
    }
}


/**
 * Get remaining size to consume
 * @return
 */
uint16_t gRemaining() {
    return (uint16_t) gFrame->len - (uint16_t) gOffset;
}


#endif //ELOQUENTSURVEILLANCE_GLOBALS_H
