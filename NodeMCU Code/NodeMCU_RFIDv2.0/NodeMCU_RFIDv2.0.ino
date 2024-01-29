//******************************libraries*******************************
//RFID-----------------------------
#include <SPI.h>
#include <MFRC522.h>
//NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);


//************************************************************************
#define SS_PIN  D4  //D4
#define RST_PIN D3  //D3
const int LED = D0;
const int buzzerPin = D8;
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "WIFI_NAME";
const char *password = "PASSWORD";
const char* device_token  = "TOKEN DEVEICE";
//************************************************************************
String URL = "http://WIFI_IP/rfidattendance/getdata.php"; //computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
//************************************************************************
void setup() {
  delay(1000);
  
  Serial.begin(115200);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi();
   pinMode(LED, OUTPUT);
   pinMode(buzzerPin, OUTPUT); 
    lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight 
  lcd.print(" VIT ATTENDANCE "); // Start Printing

  // digitalWrite(LED, HIGH);
}
//************************************************************************
void loop() {
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
    connectToWiFi();    //Retry to connect to Wi-Fi
  }
  //---------------------------------------------
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID="";
  }
  delay(50);
  //---------------------------------------------
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  String CardID ="";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }


  //---------------------------------------------
  if( CardID == OldCardID ){
    return;
  }
  else{
    OldCardID = CardID;
  }
  //---------------------------------------------
//  Serial.println(CardID);
  SendCardID(CardID);
  delay(1000);
}
//***********send the Card UID to the website************
void SendCardID( String Card_uid ){
  Serial.println("Sending the Card ID");
    digitalWrite(D1, LOW);
  if(WiFi.isConnected()){
    WiFiClient client;
    HTTPClient http;
    //GET Data
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token);
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); // use client as lvalue reference
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload

//    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(Card_uid);     //Print Card ID
    Serial.println(payload);    //Print request response payload
    digitalWrite(LED, HIGH);
    tone(buzzerPin, 100); delay(100); 
    noTone(buzzerPin); 

       lcd.clear();
delay(100);

    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
        lcd.setCursor(0,0);
     lcd.print(user_name);
     lcd.setCursor(0,1);  
     lcd.print("LogIn");

      }
      else if (payload.substring(0, 6) == "logout") {
        String user_name = payload.substring(6);
    //  Serial.println(user_name);
      lcd.setCursor(0,0);
     lcd.print(user_name);
     lcd.setCursor(0,1);  
     lcd.print("LogOut");
  
      }
      else if (payload == "succesful") {
    
      }
      else if (payload == "available") {

      }
      delay(100);
      http.end();  //Close connection
      delay(500);
        digitalWrite(LED, LOW);
    }
  }
}
//*******************connect to the WiFi*****************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    
    delay(1000);
}
//=======================================================================
