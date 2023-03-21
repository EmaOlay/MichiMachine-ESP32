#define DEBUG
#include "debugUtils.h"
#include "func.h"

//Extern variables
extern WebServer server;
extern String degrees;
extern String datetime;
extern String WaterState;
extern String current_light_on;
extern String current_light_off;
extern HardwareSerial Sender;
extern clock_t t_ini_rx;
extern clock_t t_ini_tx;
extern clock_t t_ini_api;
extern String msg;


//Por lo pronto voy a tener NTWRK_CANT conexiones guardadas
const char * ssid_arr[NTWRK_CANT]={SSID_celu,SSID_depto,SSID_casa,SSID_mtz,SSID_tincho};//,SSID_facu};
const char* pswrd_arr[NTWRK_CANT]={PSWRD_celu,PSWRD_depto,PSWRD_casa,PSWRD_mtz,PSWRD_tincho};//,PSWRD_facu};

void WiFiSetup(void){
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("[*] Scan done");
  if (n == 0) {
      Serial.println("[-] No WiFi networks found");
  } else {
      Serial.println((String)"[+] " + n + " WiFi networks found\n");
      for (int j=0;j<NTWRK_CANT;j++){
        if(WiFi.status() == WL_CONNECTED){//si ya estoy conectado, break...
          break;
        }
        for (int i = 0; i < n; ++i) {
          // Comparo SSID para cada red hayada
          Serial.println(WiFi.SSID(i));
          if(WiFi.SSID(i)==ssid_arr[j]){//Si coinciden
          // Connect to Wi-Fi network with SSID and password
          Serial.print("Connecting to ");
          Serial.println(ssid_arr[j]);
          WiFi.begin(ssid_arr[j], pswrd_arr[j]);
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);//medio segundo
            Serial.print(".");
            }
          break;
          }
        }
      }
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

//Codigo necesario para obtener el unix time y pasarlo al LPC
String GET_Request(const char* server) {
  HTTPClient http;  
  http.begin(server);
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}
extern String degrees;
long http_get_date2(void){
  String json_array;
  if(WiFi.status()== WL_CONNECTED){
    String server = "http://api.weatherapi.com/v1/current.json?key=bc10b22900494465aa7140415232102&q=-34.5983,-58.420217&aqi=no";
    json_array = GET_Request(server.c_str());
    //Serial.println(json_array);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, json_array);
    //long aux2=doc["location"]["localtime_epoch"];
    float temp=doc["current"]["temp_c"];
    degrees=temp;

    String server2 = "http://worldtimeapi.org/api/timezone/America/Argentina/Buenos_Aires";
    json_array = GET_Request(server2.c_str());
    //Serial.println(json_array);
    deserializeJson(doc, json_array);
    long aux2=doc["unixtime"];
    return aux2;
  }
  return-1;
}
//Fin del codigo para el unix time

/*
   Aqui esta definido todo el HTML y el CSS del servidor WEB con ESP32
*/
String SendHTML() {
  // Cabecera de todas las paginas WEB
  String ptr = "<!DOCTYPE html> <html>\n";
  
  // <meta> viewport. Para que la pagina se vea correctamente en cualquier dispositivo
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  ptr += "<title>MichiMachine</title>\n";
/*
 * El estilo de la pagina WEB, tipo de letra, tamaño, colores, 
 * El estilos de los botones (las clases en CSS) y la definicion de como van a cambiar dependiendo de como
 * cambien los estado de los LEDs, color fondo etc
 */
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;"
  "color:#FFFFFF}\n";
  ptr += ".button { border-radius: 30px; background-color: #4CAF50; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\n";
  ptr += "body {height: 99vh;width: 99vw;overflow: hidden;"
    "background-image: url('https://i.pinimg.com/564x/df/61/50/df6150df51bdda5f6066d4a5b62f24ec.jpg'); background-size: cover;"
  "background-repeat: no-repeat;}";
  ptr += ".input-class{border-radius: 10px; background-color: #d3d3d3;}";
  ptr += ".button2 {background-color: #555555;}\n";
  ptr += ".log {color: #FF0000; font-size: 1.5rem;}\n";
  ptr += "</style>\n";
  ptr += "<link rel=\"icon\" href=\"https://img.icons8.com/ios-glyphs/512/cat-pot.png\" />";
  ptr += "<script src=\"https://kit.fontawesome.com/657d94f99b.js\" crossorigin=\"anonymous\"></script>\n";
  ptr += "<script>"
  "setTimeout(function(){"
    "window.location.reload();"
  "},30000);"
"</script>";
  ptr += "</head>\n";
  ptr += "<body>\n";
  /*
   * Encabezados de la pagina
   */
  /*Fondos que me parecieron bonitos*/
  //https://momentum.photos/img/8164bc6a-f876-4ea7-a828-9a4ae7a492a8.jpg&quot
  //https://i.pinimg.com/564x/df/61/50/df6150df51bdda5f6066d4a5b62f24ec.jpg
  ptr += "<h1><i class=\"fa-solid fa-house\" style=\"color:#d3d3d3\"></i> MichiMachine <i class=\"fa-solid fa-shield-cat\" style=\"color:#d3d3d3\"></i></h1>\n";
  //Last updated date time
  ptr += "<p>Last updated time: "+ datetime + "   <a href=\"/RefreshTime\"><i class=\"fa-solid fa-rotate\" style=\"color:#d3d3d3\"></i></a></p>\n";
  //Current LED STATE
  //ptr += "<p>Built in LED - State " + builtinState;

/*
 * Dependiento de los parametros de la funcion SendHTML
 * modificararemos la vista de la pagina WEB, llamando a las distintas clases que cambian como
 * se muestran los datos en la pagina WEB 
 */
  ptr += "<p>Water - State " + WaterState;
  if (WaterState=="off") {
    //cierro el <p> aca abajo ya que el icono que muestro depende del estado del agua
    ptr += "<i class=\"fa-solid fa-droplet-slash\" style=\"color:#d3d3d3\"></i></p>";
    ptr += "<p><a href=\"/Water/on\"><button class=\"button\">ON</button></a></p>";
  } else {
    //cierro el <p> aca abajo ya que el icono que muestro depende del estado del agua
    ptr += "<i class=\"fa-solid fa-droplet\" style=\"color:#d3d3d3\"></i></p>";
    ptr += "<p><a href=\"/Water/off\"><button class=\"button button2\">OFF</button></a></p>";
  }
  ptr += "<p>Temperature " + degrees + " C <a href=\"/Query_temp\"><button class=\"input-class\">Query</button></a></p>";
  //ptr += "<p>Temperature 30 C</p>";
  ptr += "<form action=\"/form-light\" method=\"get\">\n"
  "<div>"
    "<label for=\"light-on-time\">"
    "  Set a lights on time(current="+current_light_on+"):"
    "</label>"
    "<input id=\"light-on-time\" type=\"time\" name=\"light-on-time\" class=\"input-class\" value=\"08:30\" required/>"
  "</div>\n"
  "<div>\n"
    "<label for=\"light-off-time\">"
    "  Set a lights off time(current="+current_light_off+"):"
    "</label>\n"
    "<input id=\"light-off-time\" type=\"time\" name=\"light-off-time\" class=\"input-class\" value=\"23:30\" required/>\n"
  "</div>\n";
  if(server.arg("light-off-time")<server.arg("light-on-time")){
    ptr +="<div class=\"log\">No se enviaron las horas ya que estaban invertidas...</div>";
  }
  ptr += "<div>\n"
    "<input type=\"submit\" value=\"Update light time\" class=\"input-class\"/>\n"
  "</div>\n"
"</form>\n"
"<form action=\"/form-time\">\n"
  "<label for=\"unixtime\">Current datetime (in case of no internet):</label>\n"
  "<input type=\"datetime-local\" id=\"unixtime\" name=\"unixtime\" value=\"2023-02-19T11:30\" class=\"input-class\">\n"
  "<input type=\"submit\" value=\"Update unix time\" class=\"input-class\">"
"</form>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  http_get_date2();
  return ptr;
}
/*
   Todos las siguientes funciones ejecutan tres tareas:
   1 Cambian de estado las variables
   2 Muestran por el monitor Serial de Arduino IDE, informacion relevante al estado de los LED
   3 Actualizan la vista de la pagina del servidor WEB con ESP32, envia al navegador un codigo 200 indican el exito
   de la conexion y llama a otra funcion SendHTML con dos parametros que modificaran la pagina 
   del servidor WEB con Arduino.
*/
extern char cycle;
void handle_OnConnect() {//podria utilizar el root para reiniciar todo...
  switch(cycle){
    case 0:
      // ask for temp
      Sender.print("#Temperatura:"+degrees+"$");
      //Sender.print("#Temperatura$");
      cycle+=1;
      break;
    case 1:
      // updates time
      long segs;
      segs=http_get_date2();
      //ESP32 manda la hora
      Sender.print("#Hora:");
      segs-=10800;//le quito 3 horas porque local time es GMT
      //Serial.println(segs);
      Sender.print(segs);
      Sender.print("$");
      cycle+=1;
      break;
    case 2:
      // Ask pump state
      Sender.print("#Pump$");
      cycle+=1;
      break;
    default:
      // default statements
      Serial.print("Default\n");
      cycle=0;
}
  //Serial.println("Reseteo todo."); //2
  server.send(200, "text/html", SendHTML()); // 3
}

void handle_WaterON() {
  WaterState = "on"; //1
  Sender.println("#Water:on$");
  //Serial.println("Water Estado: ON");//2
  server.send(200, "text/html", SendHTML());//3
}

void handle_WaterOFF() {
  WaterState = "off"; //1
  Sender.println("#Water:off$");
  //Serial.println("Water Estado: OFF");//2
  server.send(200, "text/html", SendHTML());//3
}

void handle_RefreshTime() {
  time_t segs=http_get_date2();
  //ESP32 manda la hora
  Sender.print("#Hora:");
  segs-=10800;//le quito 3 horas porque local time es GMT
  Sender.print(segs);
  Sender.print("$");
  // Format time, "yyyy-mm-dd hh:mm:ss zzz"
  struct tm ts = *localtime(&segs); //Local time es GMT
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
  datetime = buf;
  Serial.println("Updated datetime Manualy");
  server.send(200, "text/html", SendHTML());
}
void handle_light_time(){
  Serial.println("Light span handler");
  //Serial.println(server.hasArg("light-on-time"));
  //Serial.println(server.arg("light-on-time"));
  if(server.arg("light-off-time")>server.arg("light-on-time")){
  current_light_on=server.arg("light-on-time");
  current_light_off=server.arg("light-off-time");
  Sender.print("#light-span=");
  Sender.print(current_light_on);
  Sender.print(";");
  Sender.print(current_light_off);
  Sender.print("$");
  }
  // Serial.print("current light time on: ");
  // Serial.println(current_light_on);
  // Serial.print("Current light off time: ");
  // Serial.println(current_light_off);
  server.send(200, "text/html", SendHTML());//3
}
void handle_unix_time(){
  Serial.println("Unix time handler");
  //Serial.println(server.hasArg("unixtime"));
  //Serial.println(server.arg("unixtime").c_str());
  struct tm tm{};
  std::string s(server.arg("unixtime").c_str());
  if (strptime(s.c_str(), "%Y-%m-%dT%H:%M", &tm)) {
      int d = tm.tm_mday,
          m = tm.tm_mon,
          y = tm.tm_year;
      // Serial.println(y);
      // Serial.println(m);
      // Serial.println(d);
      // Serial.println(tm.tm_hour);
      // Serial.println(tm.tm_min);
      time_t timeSinceEpoch = mktime(&tm);
      //Serial.println(timeSinceEpoch);
      // Format time, "yyyy-mm-dd hh:mm:ss zzz"
      struct tm ts = *localtime(&timeSinceEpoch); //Local time es GMT
      char buf[80];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
      datetime = buf;
      Serial.println("Set time Manualy");
      Sender.print("#Hora:");
      Sender.print(timeSinceEpoch);
      Sender.print("$");
  }
  server.send(200, "text/html", SendHTML());//3
}

void handle_temp(){
  Sender.print("#Temperatura$");
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "La pagina no existe");
}

int is_valid_msg(String s){
	//Flexibilizo el mensaje que me llega
	if((s.indexOf('#')>=0 && s.indexOf('$')>0)&&(s.indexOf('#') < s.indexOf('$'))){
        //Contiene los caracteres terminadores y estan en el orden correcto
        return 0;
	}
	return -1;
}

void SetDegrees(void){
  //Serial.println(msg.substring(6,8));
  //degrees=msg.substring(6,9);
  //Serial.println(degrees);
  Sender.print("#Temperatura:"+degrees+"$");
}

