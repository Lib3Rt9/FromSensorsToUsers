# Smart home monitoring and security system

This project integrates an Intruder Detection System with Security Camera. It also has additional features: an automatic LED that turn on based on light level and motion, and a system to view temperature/humidity of the house in real time. 
## Components:

### 1. intruderDetect_smartLED

- **Description**: Arduino code for controlling LEDs based on light levels and motion detection. It also has all the sensory code for the Intruder detection system.
- **Functionality**:
  - Turns on/off LEDs based on sensor input of light level and motion.
  - Detect intruder based on magnet sensor and alarm by buzzer. This system can be turn on/off by a IR remote
  
### 2. save_mail_cred

- **Description**: ESP32 code to securely save encrypted email and WiFi credentials.
- **Functionality**:
  - Stores sensitive credentials securely on ESP32 EPROM memory.
  - Implements encryption to protect stored data.
  
### 3. send_mail_via_esp32_commu

- **Description**: ESP32 code to communicate with Arduino and send email alerts.
- **Functionality**:
  - Facilitates communication between ESP32 and Arduino modules.
  - Sends email alerts triggered by the Intruder Detection System code with Arduino.

### 4. motion_detected_camera

- **Description**: Implementation for a microcontroller to communicate with a sensor and send an email alert.
- **Functionality**:
  - Store captured footage locally on SD card.
  - Sends email alerts triggered by the microwave sensor with Doppler's effect.
  
## Demo
[https://drive.google.com/drive/folders/1mDV5Wj-wY_zIdS-Fqm_uwAzlQeinl-GS]

## Contributors:

- [Ta Quang Minh]([https://github.com/mnhqut])
- [Pham Gia Phuc]([https://github.com/Lib3Rt9])


