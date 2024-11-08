//usar el serial monitor en 115200

#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <Servo.h>
#include <FastLED.h>

#define NUM_LEDS  8
#define LED_PIN   2

#define  CLOUD_MALE   0
#define  CLOUD_X      1

#define CLOUD CLOUD_X


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//              CONSTANTS USING #define
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*#define DEBUG_OFF  0
#define DEBUG_ON   1
#define debug DEBUG_ON
#define debug_message(fmt)          \
  do {              \
    if (debug)          \
       Serial.print(fmt);     \
  } while (0)


#define serial_message(fmt)          \
  do {              \
      if (debug)          \
      Serial.print(">>> "); \
        Serial.print(fmt);     \
      if (debug)          \
        Serial.print(" <<<<\n"); \
  } while (0)*/

#define DATA_LENGTH 5

#define Board_LED D0  
#define External_LED D1
#define Board_LED_OFF  1
#define Board_LED_ON   0
#define External_LED_OFF  0
#define External_LED_ON   1
#define SERIAL_TERM  "\n"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//              GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long  lastMillis;
long now = millis();
long lastMeasure = 0;
char node_red_data[DATA_LENGTH];
byte data_index = 0;
float accelerometer [3] = {0, 0, 0};
int potentiometer = 0;

Servo myservo;
CRGB leds[NUM_LEDS];


#if CLOUD==CLOUD_MALE
char ssid[] = "";
char password[] = "";
IPAddress MqttServer(192,168,1,102); 
const unsigned int MqttPort=1883; 
const char MqttUser[]="";
const char MqttPassword[]="";
const char MqttClientID[]="";

#elif CLOUD==CLOUD_X
char ssid[] = "Nuria"; //COMPLETAR
char password[] = "patitobb";                         //COMPLETAR
IPAddress MqttServer(192,168,1,107);     //COMPLETAR IP donde esta corriendo MQTT. IP separada por comas.
//IPAddress MqttServer(127,0,0,1);     //COMPLETAR IP donde esta corriendo MQTT. IP separada por comas.
const unsigned int MqttPort=1883; 
const char MqttUser[]="";
const char MqttPassword[]="";
const char MqttClientID[]="";
#endif



// Create Cloud sockets
WiFiClient wclient;
PubSubClient client(wclient);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//              FUNCTION DECLARATIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup_gpios(void);
void setup_wifi(void);    
void setup_mqtt(void);
void reconnect(void); 
void publish_init_state(void);
void callback(char* topic, byte* payload, unsigned int length);
void ParseTopic(char* topic, byte* payload, unsigned int length);

void setup_gpios(void); // esto esta 2 veces???
void setup_wifi(void);    
void setup_mqtt(void);
void reconnect(void); 
void publish_init_state(void);
void callback(char* topic, byte* payload, unsigned int length);
void ParseTopic(char* topic, byte* payload, unsigned int length);

void handle_brightness(byte* brightness, unsigned int length);
void handle_serial();
void handle_RGB(byte* data, unsigned int length);
void cycle_color(CRGB color);

void setup() 
{
  Serial.begin(9600, SERIAL_8N1);
  Serial.swap();                   // Use alternate serial pins (disconect from USB-uart pins and connect to alternate pins) 
  //Serial.begin(115200);
  setup_gpios();         // initialize used GPIOS
  setup_wifi();          // initialize WIFI and connect to network
  setup_mqtt();          // initialize temperature mqtt server
  //myservo.attach(2);    // attaches the servo on GIO2 to the servo object

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  
}

void loop() {
  if (!client.connected()) 
  {
      reconnect();    
  }
  client.loop();  //This should be called regularly to allow the client 
                  //to process incoming messages and maintain its connection to the server
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 500) {
    lastMeasure = now;
    //publishAccelerometer();
  }
  //handle_serial();
}


void setup_gpios(void )
{
  pinMode(Board_LED, OUTPUT);
  digitalWrite(Board_LED, Board_LED_OFF);
  pinMode(External_LED, OUTPUT);
  digitalWrite(External_LED,External_LED_OFF);
}


void setup_mqtt(void) 
{
 client.setServer(MqttServer, MqttPort);
 client.setCallback(callback);
}

void setup_wifi(void) 
{
  /*debug_message("\n\n");
  debug_message("Connecting to ");
  debug_message(ssid);
  debug_message("\n");*/
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    //debug_message(".");
  }
 /*debug_message("\nWiFi connected");  
 debug_message("IP address: ");
 debug_message(WiFi.localIP());
 debug_message("\n\n");*/
 digitalWrite(External_LED,External_LED_ON); // WIFI is OK
}


void reconnect() 
{
 // Loop until we're reconnected
 while (!client.connected()) 
 {
 //debug_message("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect(MqttClientID,MqttUser,MqttPassword)) 
 {
  //debug_message("connected \r\n");
  
  cycle_color(CRGB::White);
  // and subscribe to topic
  client.subscribe("from_nodered"); 
  client.subscribe("brightness"); 
  client.subscribe("new_pixel"); 
  client.subscribe("test_button"); 
 } 
 else 
 {
  /*debug_message("failed, rc=");
  debug_message(client.state());
  debug_message(" try again in 3 seconds \r\n");*/
  // Wait before retrying
  cycle_color(CRGB::Red);
  delay(3000);
  }
 }
}


void callback(char* topic, byte* payload, unsigned int length) 
{
  /*debug_message("Message arrived [");
  debug_message("Topic:");
  debug_message(topic);
  debug_message("  Length:");
  debug_message(length);
  debug_message("] ");
  debug_message("  Payload: ");
  for (int i=0;i<length;i++) 
  {
    debug_message((char)payload[i]);
  }
  debug_message("\r\n");*/
  //client.publish("esp_answer_topic","chau",false); //answers
 // ParseTopic(topic,payload,length);
  
  if(strcmp(topic, "brightness") == 0)
    handle_brightness(payload, length);
  if(strcmp(topic, "new_pixel") == 0)
    handle_RGB(payload, length);
}

void cycle_color(CRGB color){
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                            HANDLERS AND PUBLISHERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_brightness(byte* brightness_arr, unsigned int length){
  String brightness_string = "";
  byte brightness;
  for(int i=0; i<length; i++){
    brightness_string += (char)brightness_arr[i];           //Convert to string
  }
  brightness = (byte)brightness_string.toInt();             //Convert string to byte
  /*debug_message("Converted brightness value is: \"");
  debug_message(brightness);
  debug_message("\"\r\n");*/
  //leds[0] = CRGB::White;
  //FastLED.show();
  //delay(300);
  //leds[0] = CRGB::Black;
  FastLED.show();
  Serial.write('B');                                      //Sends the 'B' to indicate that next byte is the brightness
  Serial.write(brightness);                               //Send brightness
  Serial.write(0);
  Serial.write(0);
}

void handle_serial(){
  /*if (Serial.available()>0)
    serin=Serial.read();
  
  if(serin != 0){
    node_red_data[data_index] = (char)serin;
    data_index++;
    if(data_index == DATA_LENGTH)
      publishResponse();
      data_index = 0;
      for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Yellow;
        FastLED.show();
        delay(100);
        leds[i] = CRGB::Black;
      }
  }*/
  char serin[DATA_LENGTH];
  if(Serial.available() == DATA_LENGTH){
    for(int i=0; i<DATA_LENGTH; i++){
      serin[i] = (char)Serial.read(); 
    }
    cycle_color(CRGB::Yellow);
    client.publish("test_response", (char*)serin);
  }
}

void handle_RGB(byte* data, unsigned int length){
  String aux = "";
  byte RGB[3] = {0,0,0};
  int j = 0;
  int i = 0;
  char current_char = 0;
  char last_char = 0;
  if(strcmp((char*)data, "destroy_latest_pixel") == 0){
    //debug_message("Last pixel shall be destroyed\r\n");
  }
  else{
    while(i<length){                                //We know that in this case the format of data is "rgb(r, g, b)"
      current_char = (char)data[i];                 //Save current character
      if(current_char >= '0' && current_char<= '9'){
          aux += current_char;            //If we get a number, we add it to the aux number
      }
      else{
          if(last_char >= '0' && last_char<= '9'){  //If last character was a number, and I now got a non-number
            RGB[j] = (byte)aux.toInt();     //We save current number
            aux = "";                       //Reset aux string
            j++;                            //And move to next number
          }
          //If last char was a non-nuber, and I now got a non-number, we just move to the next character
      }
      last_char = current_char;
      i++;
    }
    /*debug_message("Converted RGB Values are: R=");
    debug_message(RGB[0]);
    debug_message(", G=");
    debug_message(RGB[1]);
    debug_message(", B=");
    debug_message(RGB[2]);
    debug_message(", whorray\r\n");*/

    CRGB color = CRGB(RGB[0], RGB[1], RGB[2]);
    cycle_color(color);
    
    Serial.write('C');                                      //Sends the 'C' to indicate that next 3 bytes are the RGB colour
    Serial.write(RGB[0]);                                   //Send R
    Serial.write(RGB[1]);                                   //Send G
    Serial.write(RGB[2]);                                   //Send B
  }
}
