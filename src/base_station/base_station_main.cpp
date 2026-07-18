#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//Base Station MAC address: 30:76:F5:B9:E2:94
//UGV1 MAC address (ESP32-S3): A4:CB:8F:D9:2D:D8


struct packet{
  uint8_t mac_addr[6];
  uint8_t command_type;
  uint8_t battery_life;
  uint8_t roll_angle;
  uint8_t pitch_angle;
  uint8_t yaw_angle;
};


void Send_Command_Task(void *pvParams);
void Display_Telemetry_Task(void *pvParams);
void Mode_Check_Task(void *pvParams);
void Telemetry_Received_Handler(const uint8_t* mac_address, const uint8_t* data, int len);

TaskHandle_t Send_Command_Task_ID = nullptr;
TaskHandle_t Display_Telemetry_Task_ID = nullptr;
TaskHandle_t Mode_Check_ID = nullptr;

enum Mode{
COMMAND_MODE = 'c',
TELEMETRY_MODE = 't',
EXIT = 'e'
};

enum Command{
START_COMMAND = 's',
HALT_COMMAND = 'h'
};

enum Packet_Type{
COMMAND,
TELEMETRY
};

bool TELEM_MODE = false;

packet start_command = {};
packet halt_command = {};
packet command_sent = {};
packet telemetry_received = {};


uint8_t UGV1_MAC_ADDR[6] = {0xA4,0xCB,0x8F,0xD9,0x2D,0xD8};

QueueHandle_t Command_Queue_ID = xQueueCreate(10 , sizeof(packet));
QueueHandle_t Telemetry_Queue_ID = xQueueCreate(10, sizeof(telemetry_received));

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_register_recv_cb(Telemetry_Received_Handler);
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, UGV1_MAC_ADDR, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;


  esp_now_add_peer(&peerInfo);
  

  Serial.println("=====================");
  Serial.println("Base Station MAC Address"); 
  Serial.println(WiFi.macAddress());
  Serial.println("=====================");
  
  
  
  constexpr configSTACK_DEPTH_TYPE STACK_SIZE_BYTES = 4096;
  constexpr void *NO_TASK_PARAMS = nullptr;
  constexpr UBaseType_t SEND_COMMAND_TASK_PRIORITY = 5;
  constexpr UBaseType_t DISPLAY_TELEMETRY_TASK_PRIORITY = 5;
  constexpr UBaseType_t MODE_CHECK_TASK_PRIORITY = 3;

  
  start_command.command_type = START_COMMAND;
  halt_command.command_type = HALT_COMMAND;
  
  xTaskCreate(
    Send_Command_Task, 
    "Send Command Task",
    STACK_SIZE_BYTES,
    NO_TASK_PARAMS,
    SEND_COMMAND_TASK_PRIORITY,
    &Send_Command_Task_ID
  );

  xTaskCreate(
    Display_Telemetry_Task,
    "Display Telemetry Task",
    STACK_SIZE_BYTES,
    NO_TASK_PARAMS,
    DISPLAY_TELEMETRY_TASK_PRIORITY,
    &Display_Telemetry_Task_ID
  );

  xTaskCreate(
    Mode_Check_Task,
    "Mode Check Task",
    STACK_SIZE_BYTES,
    NO_TASK_PARAMS,
    MODE_CHECK_TASK_PRIORITY,
    &Mode_Check_ID
  );

}


void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void Send_Command_Task(void *pvParams){
  for(;;){
    xQueueReceive(Command_Queue_ID, &command_sent, portMAX_DELAY);
    esp_now_send(UGV1_MAC_ADDR, (uint8_t *)&command_sent, sizeof(command_sent));
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

void Display_Telemetry_Task(void *pvParams){
  for(;;){
    xQueueReceive(Telemetry_Queue_ID, &telemetry_received, portMAX_DELAY);
    
    if(memcmp(telemetry_received.mac_addr, UGV1_MAC_ADDR, 6) && TELEM_MODE == true){
      Serial.println("UGV 1 Telemetry:");
      Serial.println(telemetry_received.battery_life);
      Serial.println(telemetry_received.roll_angle);
      Serial.println(telemetry_received.pitch_angle);
      Serial.println(telemetry_received.yaw_angle);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void Mode_Check_Task(void *pvParams){
  for(;;){
    if(Serial.available()>0){
      switch(Serial.read()){
        case COMMAND_MODE:{
          TELEM_MODE = false;
          Serial.println("Command Mode");
          int exit_status=false;
          while(!exit_status){
            if(Serial.available()>0){  
              switch(Serial.read()){
                case START_COMMAND:{
                  Serial.println("Sending Start Command");
                  xQueueSend(Command_Queue_ID, &start_command, portMAX_DELAY);
                  break;
                }
                case HALT_COMMAND:{
                  Serial.println("Sending Halt Command");
                  xQueueSend(Command_Queue_ID, &halt_command, portMAX_DELAY);
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
          TELEM_MODE = true;
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

void Telemetry_Received_Handler(const uint8_t* mac_address, const uint8_t* data, int len){

if(len!=sizeof(telemetry_received)){
  return;
}

memcpy(telemetry_received.mac_addr, mac_address, sizeof(telemetry_received.mac_addr));
memcpy(&telemetry_received, data, sizeof(telemetry_received));
xQueueSend(Telemetry_Queue_ID, &telemetry_received, 0);

}

