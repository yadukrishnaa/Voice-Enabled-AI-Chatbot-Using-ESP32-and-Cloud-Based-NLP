# 🤖 Voice-Enabled-AI-Chatbot-Using-ESP32-and-Cloud-Based-NLP
## Project Introduction
This project implements a real-time, voice-interactive chatbot system using dual ESP32 microcontrollers, designed to provide a smooth conversational experience with visual feedback. The system supports push-to-talk voice input, speech-to-text processing, AI chatbot interaction, and text-to-speech audio output, accompanied by animated OLED expressions for better user engagement.

To overcome the resource limitations of a single microcontroller, the system is divided into two dedicated modules:

- ESP32-A handles voice recording, speech recognition, and chatbot communication.

- ESP32-B manages text-to-speech audio playback and OLED display animations.
# Key Features
- Dual-ESP32 Architecture
    - Separate controllers for ASR/chat processing and TTS/OLED ensure stability and real-time performance.
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


