
#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include "HardwareSerial.h"
#include <EEPROM.h>
#include <Crypto.h>
#include <AES.h>

#define EEPROM_SIZE 512

#define WIFI_SSID_ADDR 0
#define WIFI_PASSWORD_ADDR 64
#define AUTHOR_EMAIL_ADDR 128
#define AUTHOR_PASSWORD_ADDR 192
#define RECIPIENT_EMAIL_ADDR 256

// Define the UART instance to communicate with arduino
HardwareSerial MySerial(1);


/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.office365.com"
#define SMTP_PORT 587

// AES key -16 bytes for AES-128
byte aes_key[16] = { /* 16-byte key */ };

AES128 aes;

void smtpCallback(SMTP_Status status);

void readAndDecrypt(int addr, char* data) {
  byte encrypted[64];
  for (int i = 0; i < 64; i++) {
    encrypted[i] = EEPROM.read(addr + i);
  }
  aes.setKey(aes_key, sizeof(aes_key));
  aes.decryptBlock((byte*)data, encrypted);
}

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup(){
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  char wifi_ssid[32]= {0};
  char wifi_password[32]= {0};
  char author_email[AUTHOR_EMAIL_ADDR]= {0};
  char author_password[AUTHOR_PASSWORD_ADDR]= {0};
  char recipient_email[RECIPIENT_EMAIL_ADDR]= {0};

  // Decrypt and read credentials from EEPROM
  readAndDecrypt(WIFI_SSID_ADDR, wifi_ssid);
  readAndDecrypt(WIFI_PASSWORD_ADDR, wifi_password);
  readAndDecrypt(AUTHOR_EMAIL_ADDR, author_email);
  readAndDecrypt(AUTHOR_PASSWORD_ADDR, author_password);
  readAndDecrypt(RECIPIENT_EMAIL_ADDR, recipient_email);

  // Begin UART1 communication with Arduino
  MySerial.begin(9600, SERIAL_8N1, 16, 17); // TX=16, RX=17

  Serial.println();
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

}

void loop(){
  if (MySerial.available()) {
    String receivedData = MySerial.readStringUntil('\n'); // Read the message from Arduino
    receivedData.trim();
    Serial.println("Received: " + receivedData); // Print the received message to the Serial Monitor

    // Perform an action if the specific message is received
    if (receivedData == "SendAlertMail") {
        /*  Set the network reconnection option */
      MailClient.networkReconnect(true);


      smtp.debug(1);

      /* Set the callback function to get the sending results */
      smtp.callback(smtpCallback);

      /* Declare the Session_Config for user defined session credentials */
      Session_Config config;

      /* Set the session config */
      config.server.host_name = SMTP_HOST;
      config.server.port = SMTP_PORT;
      config.login.email = author_email;
      config.login.password = author_password;
      config.login.user_domain = "";

      config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
      config.time.gmt_offset = 7;
      config.time.day_light_offset = 0;

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = F("ESP");
      message.sender.email = AUTHOR_EMAIL;
      message.subject = F("ESP Alert");
      message.addRecipient(F("Minh"), recipient_email);
        
      //Send raw text message
      String textMsg = "Someone enter the house!!";
      message.text.content = textMsg.c_str();
      message.text.charSet = "us-ascii";
      message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
      
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
      message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


      /* Connect to the server */
      if (!smtp.connect(&config)){
        ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
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

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

    }
  }
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}
