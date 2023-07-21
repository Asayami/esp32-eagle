// turn on debug messages
#define VERBOSE
#include "EloquentSurveillanceUpdated.h"
#include "TelegramChat.h"

/**
 * Replace with your WiFi credentials
 */
#define WIFI_SSID "wifi_name"
#define WIFI_PASS "wifi_password"
#define BOT_TOKEN ""
#define CHAT_ID 1000000000

/**
 * 80 is the port to listen to
 * You can change it to whatever you want, 80 is the default for HTTP
 */
EloquentSurveillance::StreamServer streamServer(80);
EloquentSurveillance::Motion motion;
EloquentSurveillance::TelegramChat chat(BOT_TOKEN, CHAT_ID);

void setSensor(int id, int val)
{
  if(id==1)
	{
		if(val==0) camera.vflip(false);
		else if(val==1) camera.vflip(true);
	}
	else if(id==2)
	{
		if(val==0) camera.hmirror(false);
		else if(val==1) camera.hmirror(true);
	}
	else if(id==3)
	{
		if(val >= -2 and val <= 2)
		{
      int8_t brightness = val;
			camera.setBrightness(brightness);
		}
	}
  setSensorData(id, val);
}

FuncPtr sensorPtr = &setSensor;

void setup() {
    Serial.begin(115200);
    delay(3000);
    debug("INFO", "Init");

    /**
     * Configure camera model
     * You have access to the global variable `camera`
     * Allowed values are:
     *  - aithinker()
     *  - m5()
     *  - m5wide()
     *  - eye()
     *  - wrover()
     */
    camera.aithinker();
    /**
     * Configure camera resolution
     * Allowed values are:
     * - _96x96()
     * - qqvga()
     * - qcif()
     * - hqvga()
     * - _240x240()
     * - qvga()
     * - cif()
     * - hvga()
     * - vga()
     * - svga()
     * - xga()
     * - hd()
     * - sxga()
     * - uxga()
     * - fhd()
     * - p_hd()
     * - p_3mp()
     * - qxga()
     * - qhd()
     * - wqxga()
     * - p_fhd()
     * - qsxga()
     */
    camera.svga();
    /**
     * Configure JPEG quality
     * Allowed values are:
     *  - lowQuality()
     *  - highQuality()
     *  - bestQuality()
     *  - setQuality(quality), ranging from 10 (best) to 64 (lowest)
     */
    camera.highQuality();
    
     /**
     * Configure motion detection
     *
     * > setMinChanges() accepts a number from 0 to 1 (percent) or an integer
     *   At least the given number of pixels must change from one frame to the next
     *   to trigger the motion.
     *   The following line translates to "Trigger motion if at least 10% of the pixels
     *   in the image changed value"
     */
    motion.setMinChanges(0.1);

    /**
     * > setMinPixelDiff() accepts an integer
     *   Each pixel value must differ at least of the given amount from one frame to the next
     *   to be considered as different.
     *   The following line translates to "Consider a pixel as changed if its value increased
     *   or decreased by 10 (out of 255)"
     */
    motion.setMinPixelDiff(10);

    /**
     * > setMinSizeDiff() accepts a number from 0 to 1 (percent) or an integer
     *   To speed up the detection, you can exit early if the image size is almost the same.
     *   This is an heuristic that says: "If two consecutive frames have a similar size, they
     *   probably have the same contents". This is by no means guaranteed, but can dramatically
     *   reduce the computation cost.
     *   The following line translates to "Check for motion if the filesize of the current image
     *   differs by more than 5% from the previous".
     *
     *   If you don't feel like this heuristic works for you, delete this line.
     */
    motion.setMinSizeDiff(0.05);

    /**
     * Initialize the camera
     * If something goes wrong, print the error message
     */

    /**
    * Turn on debouncing.
    * It accepts the number of milliseconds between two consecutive events.
    * The following line translates to "Don't trigger a new motion event if
    * 10 seconds didn't elapsed from the previous"
    */
    motion.debounce(10000L);
    while (!camera.begin())
        debug("ERROR", camera.getErrorMessage());

    /**
     * Connect to WiFi
     * If something goes wrong, print the error message
     */
    while (!wifi.connect(WIFI_SSID, WIFI_PASS))
        debug("ERROR", wifi.getErrorMessage());

    /**
     * Initialize stream web server
     * If something goes wrong, print the error message
     */
    while (!streamServer.begin())
        debug("ERROR", streamServer.getErrorMessage());

    /**
     * Display address of stream server
     */
    debug("SUCCESS", streamServer.getWelcomeMessage());
    bool messageResponse = chat.sendMessage(streamServer.getWelcomeMessage());
    debug("TELEGRAM MSG", messageResponse ? "OK" : "ERR");

    setUpLamp();
}


void loop() {
    /**
     * Try to capture a frame
     * If something goes wrong, print the error message
     */
    if (getSensorData(5) == 0 && getSensorData(6) == 1) {
      if (!camera.capture()) {
          debug("ERROR", camera.getErrorMessage());
          return;
      }
  
      /**
       * Look for motion.
       * In the `true` branch, you can handle a motion event.
       * For the moment, just print the processing time for motion detection.
       */
      if (!motion.update())
          return;
  
      if (motion.detect()) {
          debug("INFO", String("Motion detected in ") + motion.getExecutionTimeInMicros() + " us");
          bool messageResponse = chat.sendMessage("Motion detected");
          debug("TELEGRAM MSG", messageResponse ? "OK" : "ERR");
  
          /**
           * @bug: this always returns false, even on success
           */
          bool photoResponse = chat.sendPhoto();
          debug("TELEGRAM PHOTO", photoResponse ? "OK" : "ERR");
          delay(2000);
      }
      else if (!motion.isOk()) {
          /**
           * Something went wrong.
           * This part requires proper handling if you want to integrate it in your project
           * because you can reach this point for a number of reason.
           * For the moment, just print the error message
           */
          debug("ERROR", motion.getErrorMessage());
      }
    } else {
        delay(5000);
    }
}
