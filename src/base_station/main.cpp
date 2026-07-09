#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


struct telemetry{};


struct command{};



void Command_Task(void *pCom_params);
void Telemetry_Task(void *pTelem_params);

TaskHandle_t Command_Task_ID=nullptr;
TaskHandle_t Telemetry_Task_ID=nullptr;

void setup() {
  Serial.begin(9600);
  
  xTaskCreate(
    Command_Task, 
    "Command Task",
    4096,
    nullptr,
    3,
    &Command_Task_ID
  );

  xTaskCreate(
    Telemetry_Task,
    "Telemetry Task",
    4096,
    nullptr,
    4,
    &Telemetry_Task_ID
  );




}



void loop() {

}

void Command_Task(void *pCom_Params){
  for(;;){
    Serial.print("Command Task");
    vTaskDelay(1000);
  }
}

void Telemetry_Task(void *pTelem_Params){
  for(;;){
    Serial.print("Telemetry Task");
    vTaskDelay(1000);

  }
}

