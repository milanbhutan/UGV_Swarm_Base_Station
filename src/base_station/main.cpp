#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


struct telemetry{};


struct command{};


void Command_Task(void *pCom_Params);
void Telemetry_Task(void *pTelem_Params);
void Mode_Check_Task(void *pMode_Check_Params);

TaskHandle_t Command_Task_ID = nullptr;
TaskHandle_t Telemetry_Task_ID = nullptr;
TaskHandle_t Mode_Check_ID = nullptr;

//char CURRENT_MODE='x';

enum Mode{
COMMAND_MODE = 'c',
TELEMETRY_MODE = 't',
EXIT = 'e'
};

enum Command{
START_COMMAND = 's',
HALT_COMMAND = 'h'
};



void setup() {
  Serial.begin(115200);
  
  constexpr configSTACK_DEPTH_TYPE STACK_SIZE_BYTES = 4096;
  
  constexpr UBaseType_t COMMAND_TASK_PRIORITY = 5;
  constexpr UBaseType_t TELEMETRY_TASK_PRIORITY = 5;
  constexpr UBaseType_t MODE_CHECK_TASK_PRIORITY = 3;

  xTaskCreate(
    Command_Task, 
    "Command Task",
    STACK_SIZE_BYTES,
    nullptr,
    COMMAND_TASK_PRIORITY,
    &Command_Task_ID
  );

  xTaskCreate(
    Telemetry_Task,
    "Telemetry Task",
    STACK_SIZE_BYTES,
    nullptr,
    TELEMETRY_TASK_PRIORITY,
    &Telemetry_Task_ID
  );

  xTaskCreate(
    Mode_Check_Task,
    "Mode Check Task",
    STACK_SIZE_BYTES,
    nullptr,
    MODE_CHECK_TASK_PRIORITY,
    &Mode_Check_ID
  );

}


void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void Command_Task(void *pCom_Params){
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}

void Telemetry_Task(void *pTelem_Params){
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void Mode_Check_Task(void *pMode_Check_Params){
 
  
  for(;;){
    if(Serial.available()>0){
      switch(Serial.read()){
        case COMMAND_MODE:{
          Serial.println("Command Mode");
          int exit_status=false;
          while(!exit_status){
            if(Serial.available()>0){  
              switch(Serial.read()){
                case START_COMMAND:{
                  Serial.println("Sending Start Command");
                  break;
                }
                case HALT_COMMAND:{
                  Serial.println("Sending Halt Command");
                  break;
                }
                case EXIT:{
                  Serial.println("Exiting Command Mode");
                  exit_status=true;
                  break;
                }
                default:{
                  Serial.println("Invalid Command");
                }
              }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
          }
          break;
        }
        case TELEMETRY_MODE:{
          Serial.println("Telemetry Mode");
          break;
        }
        default:{
          Serial.println("Invalid Mode");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

