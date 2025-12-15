/* ESP32 A: ASR -> ChatAnywhere -> forward reply to ESP32 B (TTS)
   Press touch (or button) to START recording (VAD stops it).
   When starting recording this sketch sends POST /stop to the TTS ESP
   so any playing audio is interrupted immediately.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoASRChat.h>



// === WiFi / ChatAnywhere ===
const char* ssid     = "Netline broadband";
const char* password = "netline@863";
const char* chatServer = "https://api.chatanywhere.org/v1/chat/completions";
String apiKey = "sk-Cx3CGtNDjTLuwuZcQBsMcypGzR609zocNfZbNFdqsrJ0ndwB";

// System prompt
String personality =
  "You are a supportive, slightly playful AI friend. Keep replies friendly."
  "You are a playful and friendly voice assistant."
  "Use light humor and very gentle sarcasm occasionally."
  "Sound warm, relaxed, and natural."
  "Keep replies short and clear for speech."
  "Stay respectful and non-romantic."
  "Don't use emojis."
;

// Forwarding: set IP of ESP32 B (TTS server)
const char* ttsHostIP = "192.168.1.5"; // CHANGE to ESP32 B IP
uint16_t ttsPort = 8000;

// I2S mic pins (INMP441)
#define I2S_MIC_SERIAL_CLOCK      33
#define I2S_MIC_LEFT_RIGHT_CLOCK  32
#define I2S_MIC_SERIAL_DATA       35

// Touch sensor pin (or change to button)
#define TOUCH_PIN 4
#define LED_PIN   2

// ASR params
#define SAMPLE_RATE 16000
#define MAX_RECORD_SECONDS 30
#define SILENCE_MS 1000

// ASR creds
const char* asr_api_key = "07fcb4a5-b7b2-45d8-864a-8cc0292380df";
const char* asr_cluster = "volcengine_input_en";
ArduinoASRChat asrChat(asr_api_key, asr_cluster);

enum ASRState { IDLE, LISTENING, PROCESSING };
ASRState currentState = IDLE;

bool wifiWasConnected = true;

unsigned long lastButtonChange = 0;
const unsigned long DEBOUNCE_MS = 50;
bool lastRaw = HIGH;
bool lastStable = HIGH;
unsigned long blinkLast = 0;
const unsigned long BLINK_INTERVAL = 350;
bool touchIdleLevel = HIGH;
bool touchActiveLevel = LOW;

String jsonEscape(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\n", "\\n");
  s.replace("\r", "\\r");
  return s;
}

void connectWifiOnce() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
  }
}

bool initASR() {
  if (!asrChat.initINMP441Microphone(I2S_MIC_SERIAL_CLOCK, I2S_MIC_LEFT_RIGHT_CLOCK, I2S_MIC_SERIAL_DATA)) return false;
  asrChat.setAudioParams(SAMPLE_RATE, 16, 1);
  asrChat.setSilenceDuration(SILENCE_MS);
  asrChat.setMaxRecordingSeconds(MAX_RECORD_SECONDS);
  return asrChat.connectWebSocket();
}

void ledIdle()  { digitalWrite(LED_PIN, LOW); }
void ledListen(){ 
  digitalWrite(LED_PIN, HIGH); 
  sendStatusToB("busy");
}
void ledBlink() {
  unsigned long now = millis();
  if (now - blinkLast >= BLINK_INTERVAL) {
    blinkLast = now;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}
void sendStatusToB(const char* state) {
  WiFiClient client;
  HTTPClient http;
  String url = String("http://") + ttsHostIP + ":8000/" + state;

  if (http.begin(client, url)) {
    http.POST("{}");
    http.end();
  }
}


// send POST /stop to TTS ESP to interrupt playback immediately
void stopTTSPlayback() {
  if (WiFi.status() != WL_CONNECTED) return;
  String url = String("http://") + ttsHostIP + ":" + String(ttsPort) + "/stop";
  WiFiClient client;
  HTTPClient http;
  if (!http.begin(client, url)) return;
  int code = http.POST("{}");
  http.end();
  Serial.println("[FORWARD] Sent /stop to TTS ESP");
}

String sendToChatbotAndGetReply(const String &userText) {
  if (WiFi.status() != WL_CONNECTED) return "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  if (!https.begin(client, chatServer)) return "";
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", "Bearer " + apiKey);
  String payload = "{\"model\":\"gpt-3.5-turbo\",\"messages\":[{\"role\":\"system\",\"content\":\"" + jsonEscape(personality) + "\"},{\"role\":\"user\",\"content\":\"" + jsonEscape(userText) + "\"}]}";
  int httpCode = https.POST(payload);
  String reply = "";
  if (httpCode > 0) {
    String response = https.getString();
    int idx = response.indexOf("\"content\":\"");
    if (idx >= 0) {
      idx += 11;
      for (int i = idx; i < response.length(); ++i) {
        char c = response[i];
        if (c == '"' && response[i-1] != '\\') break;
        reply += c;
      }
      reply.replace("\\n", "\n");
      reply.replace("\\\"", "\"");
      reply.replace("\\\\", "\\");
      Serial.println("\nAI Friend:");
      Serial.println(reply);
    } else {
      Serial.println("[CHAT] Could not parse reply");
    }
  } else {
    Serial.printf("[CHAT] HTTP error: %d\n", httpCode);
  }
  https.end();
  return reply;
}

void forwardReplyToTTS(const String &reply) {
  if (reply.length() == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;
  String url = String("http://") + ttsHostIP + ":" + String(ttsPort) + "/play";
  WiFiClient client;
  HTTPClient http;
  if (!http.begin(client, url)) return;
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"text\":\"" + jsonEscape(reply) + "\"}";
  int code = http.POST(payload);
  if (code > 0) {
    String resp = http.getString();
    Serial.printf("[FORWARD] TTS server response (%d): %s\n", code, resp.c_str());
  } else Serial.printf("[FORWARD] HTTP POST failed: %d\n", code);
  http.end();
}

void startSpeechRecognition() {
  // interrupt any playback on TTS ESP before recording
     // OLED: Listening & Processing

  stopTTSPlayback();

  currentState = LISTENING;
  Serial.println("\n--- ASR STARTED (waiting for speech end) ---");
  if (asrChat.startRecording()) {
    ledListen();
    Serial.println("[ASR] Listening... speak now");
  } else {
    Serial.println("[ASR] startRecording() failed -> IDLE");
    currentState = IDLE;
    ledIdle();
  }
}

void stopSpeechRecognitionNow() {
  if (asrChat.isRecording()) asrChat.stopRecording();
  currentState = PROCESSING;
  blinkLast = millis();
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[ASR] End of speech -> PROCESSING");
}

void handleASRResultAndForward() {

  String transcribed = asrChat.getRecognizedText();
  asrChat.clearResult();
  if (transcribed.length() > 0) {
    Serial.println("\n--- TRANSCRIBED ---");
    Serial.println(transcribed);
    String reply = sendToChatbotAndGetReply(transcribed);
    forwardReplyToTTS(reply);
  } else {
    Serial.println("[ASR] No text recognized.");
  }
  currentState = IDLE;
  ledIdle();
  Serial.println("\nSystem idle. Touch sensor ready for next recording.");
}

void detectTouchIdleLevel(int samples = 40, int delayMs = 10) {
  int countHigh = 0;
  for (int i = 0; i < samples; ++i) {
    int v = digitalRead(TOUCH_PIN);
    if (v == HIGH) ++countHigh;
    delay(delayMs);
  }
  touchIdleLevel = (countHigh >= (samples/2));
  touchActiveLevel = !touchIdleLevel;
  lastRaw = touchIdleLevel;
  lastStable = touchIdleLevel;
  Serial.print("[TOUCH] idle level detected as ");
  Serial.println(touchIdleLevel ? "HIGH" : "LOW");
}

void readTouchDebounced() {
  bool raw = digitalRead(TOUCH_PIN);
  unsigned long now = millis();
  if (raw != lastRaw) {
    lastButtonChange = now;
    lastRaw = raw;
  }
  if (now - lastButtonChange > DEBOUNCE_MS) {
    if (raw != lastStable) {
      if (lastStable == touchIdleLevel && raw == touchActiveLevel) {
        if (currentState == IDLE) startSpeechRecognition();
        else Serial.println("[TOUCH] Ignored touch (not idle).");
      }
      lastStable = raw;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  ledIdle();
  detectTouchIdleLevel();
  connectWifiOnce();
  if (!initASR()) Serial.println("ASR init failed");
  Serial.println("System ready.");
}

void loop() {
  asrChat.loop();
  readTouchDebounced();
  if (currentState == LISTENING) {
    if (asrChat.hasNewResult()) {
      stopSpeechRecognitionNow();
      handleASRResultAndForward();
    }
    yield();
  } else if (currentState == PROCESSING) {
    ledBlink();
    yield();
  } else {
    ledIdle();
    yield();
  }
  static unsigned long lastReconnectTry = 0;
  if (millis() - lastReconnectTry > 10000) {
    lastReconnectTry = millis();
    if (!asrChat.isWebSocketConnected()) {
      if (!asrChat.connectWebSocket()) Serial.println("[ASR] reconnect failed");
    }
  }
  delay(5);
}
