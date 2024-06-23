#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <Crypto.h>
#include <AES.h>
#include <Base64.h>

#define EEPROM_SIZE 512

#define WIFI_SSID_ADDR 0
#define WIFI_PASSWORD_ADDR 64
#define AUTHOR_EMAIL_ADDR 128
#define AUTHOR_PASSWORD_ADDR 192
#define RECIPIENT_EMAIL_ADDR 256

// AES key - 16 bytes for AES-128
byte aes_key[16] = { /* 16-byte key */ };

AES128 aes;

void encryptAndStore(int addr, const char* data) {
  byte encrypted[64];
  aes.setKey(aes_key, sizeof(aes_key));
  aes.encryptBlock(encrypted, (byte*)data);
  for (int i = 0; i < 64; i++) {
    EEPROM.write(addr + i, encrypted[i]);
  }
}

void readAndDecrypt(int addr, char* data) {
  byte encrypted[64];
  for (int i = 0; i < 64; i++) {
    encrypted[i] = EEPROM.read(addr + i);
  }
  aes.setKey(aes_key, sizeof(aes_key));
  aes.decryptBlock((byte*)data, encrypted);
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);

  // Replace these with your actual credentials
  const char* wifi_ssid = "*****";
  const char* wifi_password = "*******";
  const char* author_email = "********";
  const char* author_password = "*******";
  const char* recipient_email = "********";

  // Encrypt and store credentials
  encryptAndStore(WIFI_SSID_ADDR, wifi_ssid);
  encryptAndStore(WIFI_PASSWORD_ADDR, wifi_password);
  encryptAndStore(AUTHOR_EMAIL_ADDR, author_email);
  encryptAndStore(AUTHOR_PASSWORD_ADDR, author_password);
  encryptAndStore(RECIPIENT_EMAIL_ADDR, recipient_email);

  // Commit changes to EEPROM
  EEPROM.commit();
  Serial.println("Credentials written to EEPROM");

  char read_ssid[32];
  char read_password[32];
  char read_author_email[32];
  char read_author_password[32];
  char read_recipient_email[32];

  readAndDecrypt(WIFI_SSID_ADDR, read_ssid);
  readAndDecrypt(WIFI_PASSWORD_ADDR, read_password);
  readAndDecrypt(AUTHOR_EMAIL_ADDR, read_author_email);
  readAndDecrypt(AUTHOR_PASSWORD_ADDR, read_author_password);
  readAndDecrypt(RECIPIENT_EMAIL_ADDR, read_recipient_email);

  Serial.print("WiFi SSID: ");
  Serial.println(read_ssid);
  Serial.print("WiFi Password: ");
  Serial.println(read_password);
  Serial.print("Author Email: ");
  Serial.println(read_author_email);
  Serial.print("Author Password: ");
  Serial.println(read_author_password);
  Serial.print("Recipient Email: ");
  Serial.println(read_recipient_email);
}

void loop() {
}

