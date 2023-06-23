#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp8266.h>

 
#define SS_PIN 3
#define RST_PIN D3
#define RELAY 16 //relay pin
#define ACCESS_DELAY 2000
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C //OLED display’s I2C address

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6OgT2LsDt"
#define BLYNK_TEMPLATE_NAME "Smartdoor with RFID"
#define BLYNK_AUTH_TOKEN "IFCe2YopbALLE24q_cG6vBzi2JN3UQy4"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

// LED
#define LEDG 1
#define LEDR 15

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "MAMORU";
char pass[] = "12345678";

BlynkTimer timer;
WidgetTerminal terminal(V2);

//Adafruit_SSD1306 constructor (screen width, screen height, ESP8266 pin number, Reset Pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
// //position of center of display
int16_t centerX = SCREEN_WIDTH / 2; // Calculate x position of center of display
int16_t centerY = SCREEN_HEIGHT / 2; // Calculate y position of center of display

// Default Status
int doorStatus = 0;
int doorKey = 0;
int securityOpt = 0;
int stat = 0;
String Card = "C3 54 2D AB";

// Motion Sensor
const int motionSensor = 2 ; // PIR Motion Sensor
bool motionDetected = false;

// Check for Door Status
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  doorStatus = param.asInt();
}

// Check for Door Key
BLYNK_WRITE(V1)
{
  // Set incoming value from pin V1 to a variable
  doorKey = param.asInt();
}

BLYNK_WRITE(V3)
{
  // Set incoming value from pin V1 to a variable
  securityOpt = param.asInt();
}

// Buatan Untuk Print di Oled
void centerText(String text, int size, int disp = 1) {
  int16_t x,y; // Coordinates of top-left corner of bounding box
  uint16_t  w, h;// Width and height of bounding box
  display.clearDisplay(); //Clear Display
  display.setTextSize(size); //Set Countdown Number Size so it can calculate based on the modified size
  display.getTextBounds(text, 0, 0, &x, &y, &w, &h); // Calculate bounding box of Countdown Number
  int16_t X = centerX - w / 2; // Calculate x position of Countdown Number
  int16_t Y = centerY - h / 2; // Calculate y position of Countdown Number
  display.setTextSize(size); //Set Countdown Text Size so it can calculate based on the modified size
  display.setCursor(X, Y); //Set cursor to the Countdown Number position
  display.println(text); //Displaying Countdown Number

  if (disp == 1){
    display.display(); // Display updated image
    display.clearDisplay(); //Clear Display
  }
}

void alertAlarm(){
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
}

void RFIDController()
{
  if (doorKey == 0 && doorStatus == 0){
    centerText("Tap your card", 1);
    display.clearDisplay(); //Clear Display
    }

  if (doorStatus == 1 && doorKey == 0){
    centerText("Door Is Locked!", 1);
    display.clearDisplay(); //Clear Display
  }


   // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  if (content.substring(1) == Card && doorStatus == 0) //change here the UID of the card/cards that you want to give access
  {
    centerText("Authorised", 2);
    Blynk.virtualWrite(V2, "Hi! Welcome, Card Authorised \n" );
    // delay(500);
    digitalWrite(RELAY, HIGH);
    digitalWrite(LEDG, HIGH);
    delay(ACCESS_DELAY);
    digitalWrite(RELAY, LOW);
    digitalWrite(LEDG, LOW);
    display.clearDisplay(); //Clear Display
  }
 
 else {
    centerText("Declined", 2);
    if (doorStatus != 1) {
      Blynk.virtualWrite(V2, "Card Declined \n" );
    } else{
      Blynk.virtualWrite(V2, "Door is Restricted!, Turn off Door Status by The App! \n" );
    }
    digitalWrite(LEDR, HIGH);
    delay(2000);
    digitalWrite(LEDR, LOW);
    display.clearDisplay(); //Clear Display
  }
  display.clearDisplay(); //Clear Display
}


void progressBar(int i, int stat = 1){
  int totalIterations = 7;  // Set the total number of iterations
  int progressWidth = i * SCREEN_WIDTH / totalIterations; //Calculating Progress Bar Width

  if (stat == 0){
    centerText("Connecting to WIFI..", 1, 0);
  } 
  
  if (stat == 1){
    centerText("Loading...", 1, 0);
  }
  // Draw the progress bar
  display.fillRect(0, SCREEN_HEIGHT - 4 , progressWidth, 4, SSD1306_WHITE); //Creating Progress Bar
  display.display(); // Display updated image
  display.clearDisplay(); //Clear Display
  delay(500);
}

void setup() 
{
  Serial.begin(115200);   // Initiate a serial communication
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextColor(WHITE);
  display.clearDisplay();
  centerText("Welcome Back", 1);  
  delay(1500);
  progressBar(1);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  
  progressBar(2);
  digitalWrite(RELAY, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDR, LOW);


  progressBar(3, 0);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  progressBar(4);
  // Setup a function to be called every second
  timer.setInterval(1000L, RFIDController); 
  
  progressBar(5);
  SPI.begin();          // Initiate  SPI bus

  progressBar(6);
  mfrc522.PCD_Init();   // Initiate MFRC522

  progressBar(7);
  // Clear the buffer.
  centerText("Device is Ready", 1);
  delay(2000);
}
void loop() 
{  
  Blynk.run();
  timer.run();

  motionDetected = digitalRead(motionSensor);

  if (doorKey == 1) {
    digitalWrite(RELAY, HIGH);
    centerText("Door Opened Via App", 1);
    doorStatus = 1;
    stat = 1;
    digitalWrite(LEDG, HIGH);
  } else if (doorKey == 0 && doorStatus == 1 && stat == 1) {
    doorStatus = 0;
    stat = 0;
    digitalWrite(LEDG, LOW);
  } else {
    digitalWrite(RELAY, LOW);
  }

  if (doorStatus == 1 && securityOpt == 1 && motionDetected == HIGH){
    Blynk.virtualWrite(V2, "Someone's Break Your Door! \n" );
    alertAlarm();
    motionDetected = false;
  }
}