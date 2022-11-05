#include "BluetoothSerial.h"

BluetoothSerial ESP_BT;
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define TZ (7 * 60 * 60) /*JST*/
#define leftmotor1 25    //control forward and backward
#define rightmotor1 26

#define leftmotor2 14  //control door
#define rightmotor2 27

#define leftmotor3 18  //control left and right
#define rightmotor3 19
bool isOpen = false;
bool isDoSomeThing = false;
QueueHandle_t commandQueue;

char ssid[] = "Hihi-Hao";
char pass[] = "12345678@";
void setup() {
  Serial.begin(115200);
  ESP_BT.begin("Car2");

  pinMode(leftmotor1, OUTPUT);
  pinMode(rightmotor1, OUTPUT);

  pinMode(leftmotor2, OUTPUT);
  pinMode(rightmotor2, OUTPUT);

  pinMode(leftmotor3, OUTPUT);
  pinMode(rightmotor3, OUTPUT);

  digitalWrite(leftmotor1, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor2, LOW);
  digitalWrite(leftmotor3, HIGH);
  digitalWrite(rightmotor3, LOW);
  commandQueue = xQueueCreate(1, sizeof(int));
  TimerHandle_t xAutoReloadTimer = xTimerCreate("AutoReload", pdMS_TO_TICKS(6000), pdTRUE, 0, ScheduleCallback);
  if ((xAutoReloadTimer != NULL)) {
    BaseType_t xTimer2Started = xTimerStart(xAutoReloadTimer, 0);
    if ((xTimer2Started == pdPASS)) {
      Serial.println("initialization success");
    }
  }
  xTaskCreatePinnedToCore(command, "command", 1024, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(start, "start", 1024, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  digitalWrite(leftmotor3, LOW);
}

void loop() {
}
void start(void *pvParameters) {
  while (1) {
    if (commandQueue != 0) {
      int cmd;
      if (xQueuePeek(commandQueue, &cmd, (TickType_t)0)) {
        switch (cmd) {
          Serial.println(cmd);
          case 8:
            {
              front();
              delay(2000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case 2:
            {
              back();
              delay(2000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case 4:
            {
              left();
              break;
            }
          case 6:
            {
              right();
              break;
            }
          case 12:
            {
              front();
              delay(500);
              left();
              delay(3000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case 14:
            {
              front();
              delay(500);
              right();
              delay(3000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case -6:
            {
              back();
              delay(500);
              left();
              delay(3000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case -8:
            {
              back();
              delay(500);
              right();
              delay(3000);
              cmd = 0;
              xQueueOverwrite(commandQueue, &cmd);
              break;
            }
          case 7:
            {
              if (isOpen == false) {
                opendoor();
                isOpen = true;
                break;
              } else {
                Serial.println("Door Opening");
                break;
              }
            }
          case 9:
            {
              if (isOpen == true) {
                closedoor();
                isOpen = false;
                break;
              } else {
                Serial.println("Door Closing");
                break;
              }
            }
          case 0:
            {
              stop();
              break;
            }
          default:
            {
              break;
            }
        }
      }
    }
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }
}



void command(void *pvParameters) {
  for (;;) {
    if (ESP_BT.available()) {
      int cmd2;
      String cmd;
      cmd = ESP_BT.readStringUntil('\n');
      cmd.remove(0, 5);
      cmd.remove(cmd.length() - 1, 1);
      Serial.println(cmd);
      if (cmd == "up" || cmd == "tiến" || cmd == "tiến lên") {
        cmd2 = 8;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "down" || cmd == "lùi" || cmd == "lùi lại") {
        cmd2 = 2;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "left" || cmd == "rẽ trái" || cmd == "trái") {
        cmd2 = 4;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "right" || cmd == "phải" || cmd == "rẽ phải") {
        cmd2 = 6;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "up and left" || cmd == "turn left" || cmd == "đi thẳng rẽ trái") {
        cmd2 = 12;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "up and right" || cmd == "đi thẳng rẽ phải") {
        cmd2 = 14;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "down and left") {
        cmd2 = -6;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "down and right" || cmd == "lùi rẽ phải" || cmd == "đi lùi rẽ phải") {
        cmd2 = -8;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "open door" || cmd == "open" || cmd == "mở cửa") {
        cmd2 = 7;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "close door" || cmd == "close" || cmd == "đóng cửa") {
        cmd2 = 9;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "stop" || cmd == "dừng lại") {
        cmd2 = 0;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      if (cmd == "time" || cmd == "software time" || cmd == "đặt lịch" || cmd == "Đặt lịch") {
        isDoSomeThing = true;
        cmd2 = 99;
        xQueueOverwrite(commandQueue, &cmd2);
      }
      vTaskDelay(40 / portTICK_PERIOD_MS);
    }
  }
}

void ScheduleCallback(TimerHandle_t xTimer) {
  if (isDoSomeThing) {
    left();
    front();
    if (isOpen) {
      closedoor();
    } else {
      opendoor();
    }
    isOpen = !isOpen;
    delay(2000);
    stop();
    isDoSomeThing = false;
  }
}

void back() {
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(rightmotor1, LOW);
  Serial.println("backWard");
}
void front() {
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(leftmotor1, LOW);
  Serial.println("forward");
}
void right() {
  digitalWrite(rightmotor3, HIGH);
  digitalWrite(leftmotor3, LOW);
  Serial.println("right");
}
void left() {
  digitalWrite(rightmotor3, LOW);
  digitalWrite(leftmotor3, HIGH);
  Serial.println("left");
}

void stop() {
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, LOW);
  digitalWrite(rightmotor3, LOW);
  Serial.println("stop");
}

void opendoor() {
  digitalWrite(leftmotor2, HIGH);
  digitalWrite(rightmotor2, LOW);
  delay(2000);
  digitalWrite(leftmotor2, LOW);
  Serial.println("open door");
}
void closedoor() {
  digitalWrite(rightmotor2, HIGH);
  digitalWrite(leftmotor2, LOW);
  Serial.println("close door");
  delay(1850);
  digitalWrite(rightmotor2, LOW);
}