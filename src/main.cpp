#define DEBUG
#include "debugUtils.h"
#include "func.h"

/*
    Declaramos un objeto de la libreria WebServer para poder acceder a sus funciones
    Y como parametro 80, que es el puerto estandar de todos los servicios WEB HTTP
*/
WebServer server(80);
//Declaro otra USART para que se comuniquen entre el LPC845 y el ESP32
HardwareSerial Sender(2);
String msg;
//char trama[TRAMA_LEN];//Donde guardo la trama serie
//int i=0;//indice de la trama
clock_t t_ini_rx=clock();
clock_t t_ini_tx=clock();
clock_t t_ini_api=clock();
/*
   Declaramos el estado inicial de las variables compartidas con el LPC845
*/
String degrees = "0";
String datetime = "0";
String WaterState = "off";
String current_light_on = "";
String current_light_off = "";
char cycle = 0;

void setup() {
  /*
   * Declaracion de la velocidad de comunicacion entre Arduino IDE y ESP32
   * Configura el comportamiento de los pines
   */
  Serial.begin(9600);
//Init del serial para la com con el LPC845
  Sender.begin(115200,SERIAL_8N1,16,17);
/*
 * Configuracion de la conexion a la Wifi de tu casa
 */
  WiFiSetup();
/*
 * Para procesar las solicitudes HTTP necesitamos definir el codigo que debe de ejecutar en
 * cada estado. Para ello utilizamos el metodo "on" de la libreria WebServer.
 * 1 El primero se ejecuta cuando te conectas al Servidor WEB con ESP32 http://la_ip_del_esp32/
 * 2 Los siguientes procesan los comandos que querramos mandar.
 * 3 El ultimo gestiona los errores por ejemplo si pones http://la_ip_del_esp32/holaquetal
 * esta pagina no existe, por lo tanto actualizara la pagina WEB con un mensaje de error
 */
  server.on("/", handle_OnConnect); // 1
  server.on("/Water/on", handle_WaterON); // 2
  server.on("/Water/off", handle_WaterOFF); // 2
  server.on("/RefreshTime", handle_RefreshTime); // 2
  server.on("/form-light", handle_light_time); // 2
  server.on("/form-time", handle_unix_time); // 2
  server.on("/Query_temp", handle_temp); // 2
  server.onNotFound(handle_NotFound); // 3
/*
 * Arrancamos el Servicio WEB
 */
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}
/*
 * Para gestionar las la peticiones HTTP es necesario llamar al metodo "handleClient"
 * de la libreria WebServer que se encarga de recibir las peticiones y lanzar las fuciones
 * de callback asociadas.
 */
void loop() {
  server.handleClient();
  //levanto msg del LPC845
  while (Sender.available())
  {
    msg += char(Sender.read());
  }
  //pregunto si ya paso tiempo suficiente como para leer la msg armada
  if ((clock()-t_ini_rx)> Rx_TOUT)
  { 
    //Serial.println("Rx_TOUT");
    //pregunto si es valida
    if (is_valid_msg(msg)==0)
    {
      if(msg.indexOf("IP")>0){
      Sender.print("#IP:");
      Sender.print(WiFi.localIP());
      Sender.print("$");
	    }
      if(msg.indexOf("Hora")>0){
      time_t segs=http_get_date2();
      //ESP32 manda la hora
      Sender.print("#Hora:");
      segs-=10800;//le quito 3 horas porque local time es GMT
      //Serial.println(segs);
      Sender.print(segs);
      Sender.print("$");
      // Format time, "yyyy-mm-dd hh:mm:ss zzz"
      struct tm ts = *localtime(&segs); //Local time es GMT
      char buf[80];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
      (datetime).clear();
      (datetime)+=buf;
	    }
      if(msg.indexOf("Temp")>0){
        SetDegrees();
      }
      if(msg.indexOf("Pump")>0){
        //Serial.println(msg.substring(6,7));
        if(msg.substring(6,7)=="0"){
          WaterState="off";
        }else{
          WaterState="on";
        }
        
      }
    }
    //actualizo el tiempo
    t_ini_rx=clock();
    //borro lo que tengo almacenado
    msg.clear();
  }
}