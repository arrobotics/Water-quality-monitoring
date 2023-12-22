

#include <WiFi.h>
#define USE_WROVER_BOARD
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 


int DSPIN = 25; //  connect Dallas Temperature Sensor to 13 no pin of ESP32 board
int TDS_Sensor = 36; // connect TDS Sensor to 34 no pin of ESP32 board
float Aref = 3.3;

float ec_Val = 0;
unsigned int tds_value = 0;
float ecCal = 1;
double wTemp;


String apiKey = "JPDBS7AW2L6R4D6Z";     //  Enter your Write API key from ThingSpeak

const char *ssid =  "AR RoboTics";     // replace with your wifi ssid and wpa2 key
const char *pass =  "#G25Rana";
const char* server = "api.thingspeak.com";

WiFiClient client;

void sendSensor()
{
   wTemp = TempRead()* 0.0625;  // conversion accuracy is 0.0625 / LSB
  float V_level= Aref / 1024.0;
  float rawEc = analogRead(TDS_Sensor) * V_level;  // Raw  data of EC
  float T_Cof = 1.0 + 0.02 * (wTemp - 25.0);  // Temperature Coefficient
  
  ec_Val = (rawEc / T_Cof) * ecCal;// temperature and calibration compensation
  tds_value = (133.42 * pow(ec_Val, 3) - 255.86 * ec_Val * ec_Val + 857.39 * ec_Val) * 0.5; 
  double Far= (((wTemp *9)/5) + 32); // Temp in Far*

  Serial.print("TDS:"); Serial.println(tds_value);
  Serial.print("EC:"); Serial.println(ec_Val, 2);
  Serial.print("Temperature (oC):"); Serial.println(wTemp,2);   
  Serial.print("Temperature (oF):"); Serial.println(Far,2);
  Serial.print("");
delay(2000);

}

void setup()
{
  Serial.begin(9600);
  delay(100);
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  delay(500);
  display.clearDisplay();
  display.setCursor(25, 25);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" TDS Sensor");
  display.setCursor(25, 40);
  display.setTextSize(1);
  display.print("Initializing");
  display.setCursor(30, 5);
  display.setTextSize(1);
  display.print("Group P2");
  display.display();
  delay(3000);

   Serial.println("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
 
  sendSensor();
   display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
    
    display.setCursor(10, 10);
    if(wTemp < 100)
    {
    display.print("TDS: ");
    display.print(tds_value);
    display.print(" PPM");

    display.setCursor(10, 30);
    display.print("EC: ");
    display.print(ec_Val, 2);
    display.print("");
 
    display.setCursor(10, 50);
    display.print("Temp: ");
    display.print(wTemp,2);
    display.print(" C");
    display.display();
    delay(1500);
    
    display.clearDisplay();}


    if (client.connect(server, 80))  //   "184.106.153.149" or api.thingspeak.com
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(wTemp,2);
    postStr += "&field2=";
    postStr += String(tds_value);
    postStr += "\r\n\r\n";
    delay(500);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    delay(500);
  }
  client.stop();
}
