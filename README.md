# 🤖 Voice-Enabled-AI-Chatbot-Using-ESP32-and-Cloud-Based-NLP
## Project Introduction
This project implements a real-time, voice-interactive chatbot system using dual ESP32 microcontrollers, designed to provide a smooth conversational experience with visual feedback. The system supports push-to-talk voice input, speech-to-text processing, AI chatbot interaction, and text-to-speech audio output, accompanied by animated OLED expressions for better user engagement.

To overcome the resource limitations of a single microcontroller, the system is divided into two dedicated modules:

- ESP32-A handles voice recording, speech recognition, and chatbot communication.

- ESP32-B manages text-to-speech audio playback and OLED display animations.
## Key Features
- Dual-ESP32 Architecture
    - Separate controllers for ASR/chat processing and TTS/OLED ensure better handling.
- Complete Voice Interaction:
  - Voice input via INMP441 microphone
  - Real-time speech recognition using ByteDance ASR API
  - AI processing through OpenAI API
  - Voice output via MAX98357A I2S audio amplifier

- Push-to-Talk Voice Interaction
  - Touch-based speech input with immediate audio interruption support.

- GPT-Based Conversational AI
  - Uses ChatGPTGPT-3.5-Turbo

- Interruptible Text-to-Speech (TTS)
  - Speech playback can be stopped instantly when new input is detected.
    
- OLED-based UI state animation
  - Visual state feedback using procedural eye animations and bitmap frames.
- Wi-Fi Based Inter-Device Communication
  - ESP32 boards communicate over Wi-Fi using TCP/IP and HTTP REST APIs. ESP32 A acts as the HTTP client, while ESP32 B runs an HTTP server handling control and TTS requests.

## 🧩 System Architecture

### ESP32-A (ASR & Control Node)

- I²S audio acquisition via INMP441 (16 kHz, 16-bit PCM)

- Real-time ASR using WebSocket streaming
- Performs real-time speech-to-text (ASR)

- Handles chatbot interaction and response processing

- Manages system states and forwards commands to ESP32-B

## ESP32-B (TTS & UI Unit)
- HTTP server for control and data reception
- Converts text responses to speech via MAX98357A (I²S amplifier)
- Renders animated OLED UI using SSD1306
- Interruptible TTS playback with audio lifecycle reset
- Shows UI state animation based on the HTTP POST commands from ESP32-A
## 🧰 Hardware Used

- Controller: ESP32 development board (×2)

- Microphone: INMP441 digital I²S microphone

- Audio Amplifier: MAX98357A I²S Class-D amplifier or similar I2S amplifier

- Display: 0.96" SSD1306 OLED (I²C)

- Input: TTP223 capacitive touch sensor

- Audio Output:  4Ω 3W Speaker
## .🔄Communication
### ESP32-A → ESP32-B via HTTP (Wi-Fi)
The system uses Wi-Fi–based HTTP communication to coordinate actions between two ESP32 devices.
ESP32-A acts as the controller and AI processing unit, while ESP32-B acts as the audio output and UI unit.
ESP32-B runs an embedded HTTP server, and ESP32-A sends REST requests to control its behavior.
Communication is unidirectional (ESP32-A → ESP32-B) and occurs over the local network with low latency.
- Communication Protocol

    - Medium: Wi-Fi (LAN)

    - Protocol: HTTP (REST)

    - Method: POST

    - Server: ESP32-B (port 8000)

    - Client: ESP32-A
#### Endpoints Used

1. **`/busy`**
   Called when the user starts speaking.
   ESP32-B switches to the **Listening & Processing animation** on the OLED.

2. **`/stop`**
   Called immediately after `/busy`.
   Any ongoing audio playback is **stopped** so the new voice input is not interrupted.

3. **(ASR + Chatbot processing happens on ESP32-A)**
   Speech is recorded, converted to text, and sent to the chatbot.

4. **`/play`**
   Called after the chatbot reply is received.
   ESP32-B converts the text to speech and **plays the audio** through the speaker.

5. **`/idle`**
   Called after playback finishes or when the system is ready again.
   ESP32-B returns to the **idle eye animation**, waiting for the next interaction.


`/busy` → `/stop` → `/play` → `/idle`
## 🛠️ Software & Technologies

- ESP32 Arduino Framework

- I²S (Inter-IC Sound) Protocol

- Wi-Fi (HTTP & WebSocket)

- ByteDance / Volcengine ASR

- ChatAnywhere (OpenAI-compatible GPT API)

- Google Translate TTS

- Adafruit_GFX & SSD1306 Libraries

- Finite State Machine Design

# 🚀 Quick Start 
## 🛠 Environment Setup
1. Install Arduino IDE

Arduino IDE 2.0 or later recommended

2. Install ESP32 Board Support

Open Arduino IDE → File → Preferences

Add this URL to Additional Board Manager URLs:

https://espressif.github.io/arduino-esp32/package_esp32_index.json


Go to Tools → Board → Boards Manager

Search “ESP32” and install
esp32 by Espressif Systems
## 📦 Required Libraries

### Install via Library Manager (Tools → Manage Libraries):

- Adafruit_GFX

- Adafruit_SSD1306

- ArduinoJson

- WebServer (ESP32 core)
### Install via ZIP.file
- Download the [ZIP FILE](https://drive.google.com/drive/folders/1cxwlnMrinvA2jZlQyiTGMtgiXTPgrZg3?usp=sharing)
- Open Arduino IDE
- Go to Sketch → Include Library → Add .ZIP Library..
- Select the DAZI-AI.zip and ESP32-audioI2S-master.zip.
- Wait for installation to complete
## 🔑 API & Network Configuration
### Wi-Fi
In both ESP32-A and ESP32-B sketches, set:
```bash

  const char* ssid = "your-wifi-name";
  const char* password = "your-wifi-password";
```
### ASR (ESP32-A)
Provider: ByteDance / Volcengine
Replace:

```bash
const char* asr_api_key = "your-bytedance-asr-api-key";

```
### Chatbot (ESP32-A)
Provider: ChatAnywhere (OpenAI-compatible)
Replace:
```bash
String apiKey = "your-chatanywhere-api-key";
```
Customize system prompt to define chatbot personality:
```bash
String personality =
  "You are a playful, warm, and respectful AI companion with light humor.";
```
## 🔌 Circuit connections
- ###  ESP32-A (ASR + Chatbot Controller)
#### 🎤 INMP441 I²S Microphone
| INMP441    | ESP32-A            |
| ---------- | ------------------ |
| VDD        | 3.3V               |
| GND        | GND                |
| L/R        | GND (Left channel) |
| WS (LRCLK) | GPIO **32**        |
| SCK (BCLK) | GPIO **33**        |
| SD (DATA)  | GPIO **35**        |
#### 👆 TTP223 Touch Sensor 
| TTP223 | ESP32-A    |
| ------ | ---------- |
| VCC    | 3.3V       |
| GND    | GND        |
| OUT    | GPIO **4** |
- ### 🔌 ESP32-B (TTS + OLED + Speaker)
#### 🔊 MAX98357A I²S Amplifier
| MAX98357A | ESP32-B     |
| --------- | ----------- |
| VIN       | 5V          |
| GND       | GND         |
| DIN       | GPIO **26** |
| BCLK      | GPIO **25** |
| LRC       | GPIO **27** |
| SPK+      | Speaker +   |
| SPK−      | Speaker −   |
#### 🖥️ SSD1306 OLED (0.96", I²C)
| OLED | ESP32-B     |
| ---- | ----------- |
| VCC  | 3.3V        |
| GND  | GND         |
| SDA  | GPIO **21** |
| SCL  | GPIO **22** |
## 🔄 Overall System Architecture

### ESP32-A

- Touch-to-talk input

- Speech-to-text (ASR)

- Chatbot processing

- Sends control + text to ESP32-B

### ESP32-B

- OLED animations (Idle / Busy)

- Text-to-speech (Google TTS)

- Audio playback

- Interruptible audio handling
## ▶️ Upload Order
Upload ESP32-B first
Note its IP address from Serial Monitor

Update this in ESP32-A:
```bash
const char* ttsHostIP = "ESP32-B-IP";// like 192.168.1.1

```
Upload ESP32-A
## 🧪 Testing

1. Open Serial Monitor (115200 baud)

2. Wait for Wi-Fi connection

3. Touch the TTP223 sensor

4. Speak your prompt

5. Release → ASR → Chatbot → TTS

6. OLED shows:

    - Idle animation

    - Busy animation during listening & processing

7. Speaker outputs AI response
## 🚀 How It Works (Flow)

1. User touches the touch sensor

2. ESP32-A starts recording audio

3. Speech is converted to text (ASR)

4. Text is sent to the chatbot (GPT)

5. Chatbot response is sent to ESP32-B

6. ESP32-B plays speech + updates OLED
    - OLED shows:
        - Idle animation [eyes blinking]

        - Busy animation during listening & processing [audio signal]

7. User can interrupt at any time


