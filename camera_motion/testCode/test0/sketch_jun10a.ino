#include "esp_camera.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// OLED I2C address
#define OLED_ADDRESS 0x3C

// OLED pins (using ESP32-CAM GPIOs)
#define OLED_SDA 15
#define OLED_SCL 13

// LED pin
#define LED_BUILTIN 4 // Change this if your LED is connected to another pin

// Camera model
#define CAMERA_MODEL_AI_THINKER

// Include camera configuration corresponding to your model
#include "camera_pins.h"

// Initialize the OLED display using Wire library
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  
  // Set up the LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Initialize camera
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

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Initialize the OLED display
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on
  // Capture a photo with the camera
  camera_fb_t * fb = esp_camera_fb_get();
  digitalWrite(LED_BUILTIN, LOW); // Turn the LED off
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Display the image on the OLED here
  // Convert fb->buf (image data) to a format suitable for the OLED display
  // This will depend on the resolution and color capabilities of your OLED

  // Example of displaying text on the OLED
  display.clearDisplay();
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.println(F("ESP32-CAM Image"));
  
  // TODO: Add your image processing and display code here
  
  display.display();
  // Return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);

  // Delay before the next capture
  delay(5000);
}
