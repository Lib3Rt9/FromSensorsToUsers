#include "esp_camera.h"
#include "FS.h"
#include "SPI.h"
// #include <LittleFS.h>
#include "EEPROM.h"
#include "driver/rtc_io.h"
#include "Arduino.h"              // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "SD.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems           // read and write from flash memory
#include <ESP_Mail_Client.h>  // library for sending emails
#include <WiFi.h>
// #include <SPI.h>
// #include <SD.h>

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

#define ID_ADDRESS            0x00
#define COUNT_ADDRESS         0x01
#define ID_BYTE               0xAA
#define EEPROM_SIZE           0x0F

uint16_t nextImageNumber = 0;

#define WIFI_SSID             "STARRY 9079"
#define WIFI_PASSWORD         "=2W9022p"

#define emailSenderAccount    "someoneusearduino@outlook.com"
#define emailSenderPassword   "gwumxhqslfigmdbp"

#define emailRecipient        "kepodin804@elahan.com"
// #define emailRecipient        "phucpg.m23ict@usth.edu.vn"
// #define emailRecipient2     "recipient2@email.com"

//The Email Sending data object contains config and data to send
SMTPSession smtp;
SMTP_Message message;

int pictureNumber = 0;

//Callback function to get the Email sending status
void smtpCallback(SMTP_Status status);

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Waking up ...");
  Serial.println("Booting...");

  pinMode(4, INPUT);              //GPIO for LED flash
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);  //diable pin hold if it was enabled before sleeping

  //connect to WiFi network
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
    
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  //init with high specs to pre-allocate larger buffers
  if(psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else 
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  //initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //set the camera parameters
  sensor_t * s = esp_camera_sensor_get();
  s->set_contrast(s, 2);    //min=-2, max=2
  s->set_brightness(s, 2);  //min=-2, max=2
  s->set_saturation(s, 2);  //min=-2, max=2
  delay(100);               //wait a little for settings to take effect
  
  //mount SD card
  Serial.println("Mounting SD Card...");
  MailClient.sdBegin(14,2,15,13);

  Serial.println("Starting SD Card");
 
  delay(500);
  // if(!SD_MMC.begin()){
  if(!SD.begin()){
  // if(!SD.begin(5)){
    Serial.println("SD Card Mount Failed");
    //return;
  }
 
  // // uint8_t cardType = SD_MMC.cardType();
  // uint8_t cardType = SD.cardType();
  // if(cardType == CARD_NONE){
  //   Serial.println("No SD Card attached");
  //   return;
  // }

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
  //initialize EEPROM & get file number
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("Failed to initialise EEPROM"); 
    Serial.println("Exiting now"); 
    while(1);   //wait here as something is not right
  }

/*
  EEPROM.get(COUNT_ADDRESS, nextImageNumber);
  Serial.println(nextImageNumber);
  nextImageNumber += 1;
  EEPROM.put(COUNT_ADDRESS, nextImageNumber);
  EEPROM.commit();
  while(1);
  */
  
  /*ERASE EEPROM BYTES START*/
  /*
  Serial.println("Erasing EEPROM...");
  for(int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0xFF);
    EEPROM.commit();
    delay(20);
  }
  Serial.println("Erased");
  while(1);
  */
  /*ERASE EEPROM BYTES END*/  

  if(EEPROM.read(ID_ADDRESS) != ID_BYTE)    //there will not be a valid picture number
  {
    Serial.println("Initializing ID byte & restarting picture count");
    nextImageNumber = 0;
    EEPROM.write(ID_ADDRESS, ID_BYTE);  
    EEPROM.commit(); 
  }
  else                                      //obtain next picture number
  {
    EEPROM.get(COUNT_ADDRESS, nextImageNumber);
    nextImageNumber +=  1;    
    Serial.print("Next image number:");
    Serial.println(nextImageNumber);
  }

  // Initialize LittleFS
  // if (!LittleFS.begin()) {
  //   Serial.println("An error occurred while mounting LittleFS");
  //   return;
  // }
  // Serial.println("LittleFS mounted successfully");

  //take new image
  camera_fb_t * fb = NULL;

  //obtain camera frame buffer
  fb = esp_camera_fb_get();
  if (!fb) 
  {
    Serial.println("Camera capture failed");
    Serial.println("Exiting now"); 
    while(1);   //wait here as something is not right
  }

  // Photo file name
  // Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  // File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  //save to SD card
  //generate file path
  String path = "/IMG" + String(nextImageNumber) + ".jpg";

  // fs::FS &fs = SD_MMC;
  fs::FS &fs = SD;
  // Photo file name
  Serial.printf("Picture file name: %s\n", path.c_str());

  // Open the file on the SD card
  // File file = fs.open(path.c_str(), FILE_WRITE);
  // File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  File file = SD.open(path.c_str(), FILE_WRITE);
  // File file = LittleFS.open(path.c_str(), FILE_WRITE);

  Serial.println("File size: " + String(file.size()));
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();

  //return camera frame buffer
  esp_camera_fb_return(fb);
  Serial.printf("Image saved: %s\n", path.c_str());
  delay(1000);
  
  
  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(sendCallback);

  // Set the session config
  Session_Config smtp_config;
  smtp_config.server.host_name = "smtp.office365.com";
  smtp_config.server.port = 587;
  smtp_config.login.email = emailSenderAccount;
  smtp_config.login.password = emailSenderPassword;
  smtp_config.login.user_domain = "";

  // Set the NTP config time
  smtp_config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  smtp_config.time.gmt_offset = 7;
  smtp_config.time.day_light_offset = 0;

  // Declare the message class
  SMTP_Message message;

  /* Enable the chunked data transfer with pipelining for large message if server supported */
  message.enable.chunking = true;

  // Set the message headers
  message.sender.name = F("ESP32-CAM");
  message.sender.email = emailSenderAccount;
  message.subject = F("Motion Detected - ESP32-CAM");
  message.addRecipient(emailRecipient, emailRecipient);

  // Send raw text message
  String textMsg = "Image captured and attached.";
  message.text.content = textMsg.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_qp;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    
  // // Before attaching the image, re-initialize the SD card
  // if (!SD_MMC.begin()) {
  //   Serial.println("Failed to re-initialize SD card");
  //   return;
  // }

  delay(1000);
  // Check if the file exists
  // if (!SD_MMC.exists(path.c_str())) {
  if (!SD.exists(path.c_str())) {
    Serial.println("File does not exist");
    return;
  }

  // Add attach files from SD card
  // if (SD_MMC.exists(path.c_str())) {
  if (SD.exists(path.c_str())) {
    delay(1000);
    // File file = SD_MMC.open(path.c_str(), FILE_READ);
    File file = SD.open(path.c_str(), FILE_READ);
    
    if(file){
      Serial.println("File size: " + String(file.size()));
      if(file.size() > 0){
        // // Create a byte array to hold the file data
        // uint8_t *fileData = new uint8_t[file.size()];

        // // Read the file into the byte array
        // file.read(fileData, file.size());

        SMTP_Attachment attachment;
        attachment.descr.filename = path.c_str();
        attachment.descr.mime = "image/jpg";
        attachment.file.path = path.c_str();
        // attachment.file.path = path;
        // attachment.file.data = fileData;
        // attachment.file.size = file.size();
        // attachment.file.storage_type = esp_mail_file_storage_type_flash;
        attachment.file.storage_type = esp_mail_file_storage_type_sd;
        // attachment.file.storage_type = esp_mail_file_storage_type_memory;
        attachment.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
        
        message.addAttachment(attachment);

        // // Close the file
        // file.close();

        // // Delete the byte array after use
        // delete[] fileData;
      } else {
        Serial.println("File is empty");
      }
      file.close();
    } else {
      Serial.println("Failed to open file");
    }
  } else {
    Serial.println("File does not exist");
  }

  delay(500);
  // Serial.println("File attachment path: ", path);
  Serial.println();

  // Connect to the server
  if (!smtp.connect(&smtp_config)){
    Serial.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  // Start sending Email and close the session
  if (!MailClient.sendMail(&smtp, &message, true))
    Serial.printf("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

  // bool success = MailClient.sendMail(&smtp, &message);

  // if (success) {
  //   Serial.println("Email sent successfully!");
  // } else {
  //   Serial.printf("Failed to send email. Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  // }

  Serial.println("\nEmail sent.\n");

  

  pinMode(4, OUTPUT);              //GPIO for LED flash
  digitalWrite(4, LOW);            //turn OFF flash LED
  rtc_gpio_hold_en(GPIO_NUM_4);    //make sure flash is held LOW in sleep

  Serial.println("Entering deep sleep mode ...");
  delay(2000);
  Serial.println("Z ... Z ... Z ...");

  Serial.flush(); 
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);   //wake up when pin 13 goes LOW
  delay(5000);                                   //wait for 10 seconds to let PIR sensor settle

  esp_deep_sleep_start();
}

void loop() 
{


}


//Callback function to get the Email sending status
void sendCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

   // You need to clear sending result as the memory usage will grow up.
   smtp.sendingResult.clear();
  }
}