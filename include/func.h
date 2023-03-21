#define Rx_TOUT 2500 //RX Timeout
#define Tx_TOUT 40000 //TX timeout
#define DELTA_T_API 30000 //Refresh time from API(auto)
#define MSG_LEN 100 //Max length of USART message
#define NTWRK_CANT 6 //Amount of configured networks
//Para el servidor WEB
#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
//libreria para la arduino ide
#include <Arduino.h>
//Para info de la API
#include <HTTPClient.h>
//Para manejo derespuestas de las API
#include <ArduinoJson.h>


int is_valid_msg(String s);
//Estas implementaciones no le gustan al driver de la UART
//void process_msg(String s,String temperature,HardwareSerial Sender, String datetime);
//void process_msg(String s,HardwareSerial Sender, String datetime);

void WiFiSetup(void);

String GET_Request(const char* server);

long http_get_date2(void);

String SendHTML();

void handle_OnConnect();

void handle_WaterON();

void handle_WaterOFF();

void handle_RefreshTime();

void handle_light_time();

void handle_unix_time();

void handle_temp();

void handle_NotFound();

void SetDegrees(void);

//void LPC845_Communication(String* msg,String * degrees,String * datetime);