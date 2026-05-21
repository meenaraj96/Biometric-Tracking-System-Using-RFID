
#define HELTEC_POWER_BUTTON // Must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <ESP_Google_Sheet_Client.h>

/**********************Excel Sheet for End Device 1*****************************/
// Google Project ID
#define PROJECT_ID "enddevice1sensorsdata"
// Service Account's client email
#define CLIENT_EMAIL "iotsensordatalogging@enddevice1sensorsdata.iam.gserviceaccount.com"
// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC1qP+y47cmhtAW\nP3YMVy5hvGcu+tdL4XPvznj154Hh0J83t/69/tqvJp403l1fm4egZm3CDLOBiLsa\ntNsvBW5zQ1eFuky2XRNOBfgTwhBQnA4Nfq6Fh1sDCq49hE2tWro9nPH8M07sCWXX\nIiEod0aVTzX/B6fxexEc6YeU8kWI11R5Ih6JsRq1ncSW1Bh1edmtXt9v/YYMm5TS\n8u3hOkapz5bJNsFj5PMcoK9ReD7LZPbkULlR/cCwexcmSWygYQwLsOej4gsHnVuj\nSJ/o6OC79hd7adlfnvMiCkT67xu0B6MtlMaxYl0YgWYJPOc0iXZtsUGo+0om5Q8H\nmzu1IMs5AgMBAAECggEAKgfUQ5Re1aXnAuLDgibwtxHTkOHeSvL6s18jCwg5u6Dc\nmdRzNwv+lnxZqfNSCNMwLetNFVMBfGTBn7P9K5O24qnP2APLCH2gABMu+wlRY4k7\n0iD/qarv4pQHzPltzIB7q1JDBd5sMf4Zp01A/T8doGGvpVZk4+EczEL3CP4GGxsN\nISloztRq3LbWLIeVlsyQ74RhkGEz6NTw+BgA1rgTT+/usVjWZaIjS6pscWTN8dcY\nUQP8Gtq19WEd5gCil6iy9oC2M7bdgrnLONbcm2GJIYqaoyxcKzn1YSQGriaC2EGv\ncScuPkpd22cp2wWn4kSeNC6PClL6MxmVd1+rBCq7FQKBgQDoZQ+IG3d+QmeuHfdS\nn34hi76Tfb+cSNYYtio8sLKKz57+q3tv0xpB7O8mETw6mF1MULOpedzJwhXxiy3r\nZnZZDrf0NNjoDP0c/iN+RNGxDCjh/T4MSFIJQZUEnTEqz/680ktRyYYXybA/hiKU\n9yelQZv5qNF5FqskKUipsFWe9QKBgQDIHLDQnRAa/mhatxR+LaMFVvvHlpJOVfJA\n1Oja8htna6uJpaBGyDjXfRByJT9DzrpSr7xCbrPF8I8fIRgPH0vC4l7XEljorBtV\ns2FR57idyqiKWrrLMcd3BllfT0Hkzndg2nbEmQpnsZ+75cVJqzOGSpch7FWEiCL7\nGzeMAcjItQKBgQDcOZDbYXP+UT1hUgJi6MOlVIFX0a13k8pXhqFF/Ahl5NR979E8\nqidS3A3Q3DpgKK0ZOczXyKTDCUq0KDluvx1dzcXjBQWb+VsPU4IdEWhJSAWiCtL2\n3r3nCaAGzFOgh5SWKQ65Rajt9/IucJVq50BJqX/+uc0v25x42yqBq5mn1QKBgFaV\n+3e4jhBwBi1CTG6wRnp4AaeBojqc+55LwZmC6pKcRvPfI3OdDONIrQYG0dIAMCS4\n/2drSQQbq4HXqvZYH0rq8ghzWt8KISW64SA0eHBmEPkUewCSwHDi4WbY/3UGVScm\nP/+fDJ6Df88og2310uyHd89o2DPpR+GJDEbbOsqFAoGBAKKaWoqrZ0vi8WL7oG3u\nlAcOmF/SMJah+L3DN+7E9iw2EdKgLvZz3lo67HG49QiKKHnj9FJfSpsRd+Y4JjPS\nLMZXoZZ9rdyjnTC8KwB+A8YLB9q3jEmZwfefsh4KnVg8QBxbzqqD32lDOnZn1dCY\nUbyOiHs+0IgEf1Aee/1zf+qV\n-----END PRIVATE KEY-----\n";
// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1z7mz4GxF9_nD8Lksh1ZvHfTDTECu4MvPGuLvRDZLUuE";
bool headerAdded = false; // Flag to ensure header is only sent once per session

/**********************Excel Sheet for End Device 3*****************************/
// The ID of the spreadsheet where you'll publish the data
const char RFID_spreadsheetId[] = "1P3xE1qQPpH6xJNdjfUjF5rP4o5GiW42vI4lB_S23GPc";
bool RFID_headerAdded = false; // Flag to ensure header is only sent once per session

/*******************************************************************************/
void tokenStatusCallback(TokenInfo info);

//#define BAND 470E6 
char Gateway_timeString[100] = ""; 
String LoRa_Tx_DeviceID;
String EndDevice1_timeString;
float c;
float h;
float PPFD;
float DS18B20_Temp;
float flowRate;
float TotalWaterUsedLitres;
String EndDevice2_timeString;
//String EndDevice3_timeString;
String DeviceID3;
String EmployeeName;
String Log_Date;
String Log_Time;
String EmployeeUID;
String Status;
int counterTime = 0;

#define FREQUENCY 433.0 //radio frequency band 433 MHz, 470 MHz
#define BANDWIDTH 125.0 //bandwidth 125 KHz, 250 KHz
#define SPREADING_FACTOR 10 //spreading factor 7,8,10
#define CODING_RATE 5
#define SYNCWORD 0x12
#define CRC false
#define OUTPUT_POWER 20
#define CURRENT 100
#define PREAMBLE 8

String incomingLoRaData;
//String EndDevice1_LoRaData;
//String EndDevice2_LoRaData;
//String EndDevice3_LoRaData;
volatile bool rxFlag = false;

// WiFi network credentials
const char* ssid = "Crestclimber";
const char* password = "Admin@123";

//const char* ssid = "Meena_Samsung";
//const char* password = "PER24@68fect";

const char* ntpServer1 = "pool.ntp.org";
// India Standard Time (IST) is UTC +5:30. 
// This is handled by the TZ string, so the standard offsets are set to 0.
const long  gmtOffset_sec = 0; // The TZ string handles the offset
const int   daylightOffset_sec = 0; // India does not observe Daylight Saving Time
// Timezone string for India (Asia/Kolkata is a common representation)
// The TZ string format is "TZname offset DSTname [offset],start[/time],end[/time]"
// For IST, a simple string can be used if supported by the ESP8266 core's time library
const char* time_zone = "IST-5:30"; // Using standard POSIX format for simplicity
const long  indiaOffset_sec = 19800; // 5 hours and 30 minutes

const unsigned long sixHours = 6UL * 60UL * 60UL * 1000UL; //To use as delay
const unsigned long tenMinutes = 10UL *60UL * 1000UL;  //To use as delay

// MQTT broker details
const char* mqtt_server = "test.mosquitto.org"; //"test.mosquitto.org" // e.g., "m16.cloudmqtt.com"
const int mqtt_port = 1883; // Use 1883 for TCP, or 8883 for secure TLS connection
//const char* mqtt_user = "YOUR_MQTT_USERNAME";
//const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_topic_1 = "heltec/esp32/data/ED1"; // The topic to publish data to Cloud
const char* mqtt_topic_2 = "heltec/esp32/data/ED2"; // The topic to publish data to Cloud
const char* mqtt_topic_3 = "heltec/esp32/data/ED3"; // The topic to publish data to Cloud
const char* mqtt_topic_other = "heltec/esp32/data/other"; // The topic to publish data to Cloud
//const char* mqtt_client_id = "ESP32Client"; // Unique client ID

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
int value = 0;
char str[300];  

void EndDevice1(String EndDevice1_LoRaData);
void EndDevice2(String EndDevice2_LoRaData);
void EndDevice3(String EndDevice3_LoRaData);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

// Interrupt callback function for when a packet is received
// Can't do Serial or display things here, takes too much time for the interrupt
void rx() {
  rxFlag = true;
}

// Optional callback function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  // ... handle incoming messages here if needed ...

}

//if (client.connect("HeltecV3Client")) {
//if (client.connect(mqtt_client_id.c_str(), mqtt_user, mqtt_password))
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String mqtt_client_id = "ESP32Client-";   // Create a random client ID
    mqtt_client_id += String(random(0xffff), HEX);
    if (client.connect(mqtt_client_id.c_str())) {
      Serial.println("connected");
      //Optional: Subscribe to a topic after connecting
      //client.subscribe("esp8266/control"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(", retrying in 5 seconds");
      delay(5000);
    }
    Serial.println();
  }
}

void setup() {
  heltec_setup();  
  // heltec_setup() initializes LoRa, OLED, and serial communication automatically
  //initializes Serial at 115200 baud. initializes the OLED and sets up the SX1262 LoRa radio.

  while (!Serial);
  Serial.println("System Initialized.....LoRa Receiver Initializing...");

  setup_wifi();

  // Initialize and get time from NTP Server
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ...);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
  // Set the Timezone
  setenv("TZ", time_zone, 1); // Set the TZ environment variable
  tzset(); // Apply the timezone settings
  Serial.println("Time synchronized.");

  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(1024);
  client.setCallback(callback); // Optional: for receiving messages

  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);

  // Initialize RadioLib-based radio instance
  RADIOLIB_OR_HALT(radio.begin());

  // Set the receive callback
  radio.setDio1Action(rx);

  // Configure radio frequency
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));  
  RADIOLIB_OR_HALT(radio.setCodingRate(CODING_RATE));
  RADIOLIB_OR_HALT(radio.setSyncWord(SYNCWORD));
  RADIOLIB_OR_HALT(radio.setCRC(CRC)); 
  //RADIOLIB_OR_HALT(radio.setOutputPower(OUTPUT_POWER));
  //RADIOLIB_OR_HALT(radio.setCurrentLimit(CURRENT));
  //RADIOLIB_OR_HALT(radio.setPreambleLength(PREAMBLE));

  // Start in continuous receive mode
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  
  display.println("LoRa Receiver Ready");  
  display.println("Waiting for Data..."); 

  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void sendTimestamp() {
  // Format the time as a readable string: "Day of week, Month Day, Year Hour:Minute:Second IST"
  char timeStringSync[300];
  String SyncTimestampStr;

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  strftime(timeStringSync, sizeof(timeStringSync), "%H:%M:%S IST %d %B %Y %A ", &timeinfo);
  Serial.println(timeStringSync);   

  // Display on built-in OLED
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);               
  sprintf(str, "Time: %s", timeStringSync);
  display.drawString(0, 15, str);              
  display.display();

  time_t now;
  time(&now); // Get current epoch time  
  
  Serial.print("UTC time: ");   Serial.print(ctime(&now)); // Print the UTC time for verification

  // --- ADD THE OFFSET ---
  time_t indiaTime = now + indiaOffset_sec;
  
  Serial.print("GMT+05:30 time: "); // Print the GMT+05:30 time  
  Serial.print(ctime(&indiaTime)); // ctime() converts epoch time_t to a human-readable string

  SyncTimestampStr = "TimeDate," + String(indiaTime); // Convert time_t to String
  Serial.print("Time Sent: ");
  Serial.print(SyncTimestampStr); Serial.print(" => ");  Serial.print(ctime(&indiaTime));

  radio.clearDio1Action();
  RADIOLIB(radio.transmit(SyncTimestampStr.c_str()));

  int state = radio.transmit(SyncTimestampStr);

  if (state == RADIOLIB_ERR_NONE) 
  {
    Serial.println("Transmission successful!");
  } 
  else 
  {
    Serial.printf("Transmission failed, code: %i\n", state);
  }

  Serial.println();

  radio.setDio1Action(rx);
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF)); // Restart receive mode
}

void loop() {  
  // heltec_loop() handles the power button and deep sleep logic
  // heltec_loop() manages button presses and background tasks
  heltec_loop();
  unsigned long currentMillis1 = millis();
  int seconds1 = currentMillis1/1000;
  int minutes1 = seconds1/60;
  int hours1 = minutes1/60;

  heltec_delay(1000); 
  if (!client.connected()) {
    reconnect();
    //sendTimestamp();
  }
  client.loop(); // Keeps the connection alive

  if (rxFlag) 
  {
    rxFlag = false;
    
    // Read the received packet into a String
    RADIOLIB_OR_HALT(radio.readData(incomingLoRaData));
    int state = radio.readData(incomingLoRaData);
    Serial.print(F("\nData Status: "));  
    Serial.println(state);    
    Serial.print(F("Recieved Data: "));
    Serial.println(incomingLoRaData);

    if (state == RADIOLIB_ERR_NONE) 
    {
      if ( (incomingLoRaData == "REQUEST_DATA") || (incomingLoRaData == "REQUEST_DATA2") || (incomingLoRaData == "REQUEST_DATA3") )  // Check if the received packet is a specific request
      {
        Serial.println("Request received, sending data response...");

        // Display on built-in OLED
        display.clear();

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, 0, "Request Received!");
        display.drawString(0, 15, "Sending Time Response...");
        sprintf(str, "RSSI: %.2f dBm", radio.getRSSI());
        display.drawString(0, 30, str);
        sprintf(str, "SNR: %.2f dB", radio.getSNR());
        display.drawString(0, 45, str);

        display.display();

        sendTimestamp();
        //radio.transmit("ACK");
        Serial.println("Response sent");
      }
      else
      {
        int fc = incomingLoRaData.indexOf(',');
        String D_ID = incomingLoRaData.substring(0, fc);
        Serial.print(F("Device ID: "));   Serial.print(D_ID);

        if(D_ID == "3")
        {
          // Print to Serial
          Serial.printf("\nReceived: [%s]", incomingLoRaData.c_str());
          Serial.printf("\nRSSI: %.2f dBm, SNR: %.2f dB \n", radio.getRSSI(),radio.getSNR());
          
          EndDevice3(incomingLoRaData.c_str());
          //heltec_delay(3000); //add in EndDevice3() function defenition and adjust delay according
        }
        else
        {
          // Print to Serial
          Serial.printf("\nReceived: [%s]", incomingLoRaData.c_str());
          Serial.printf("\nRSSI: %.2f dBm, SNR: %.2f dB \n", radio.getRSSI(),radio.getSNR());

          //(3) To send Data to cloud
          // *** Create JSON Object ***
          // Use the ArduinoJson Assistant to determine the buffer size
          JsonDocument doc;       //StaticJsonDocument<512> doc;

          doc["Received_Other_Data"] = incomingLoRaData;

          // *** Serialize JSON to String ***
          char jsonBuffer[1024];
          serializeJson(doc, jsonBuffer);  //, sizeof(jsonBuffer)
          Serial.printf("Publishing message to %s: %s\n", mqtt_topic_other, jsonBuffer);

          // *** Publish to MQTT ***
          // Publish the message to the MQTT topic
          bool success = client.publish(mqtt_topic_other, jsonBuffer);
          if (!success) {
            Serial.println("Publish failed! Buffer might still be too small.");
          }   else {
            Serial.println("MQTT Published Successfully");
          }
          heltec_delay(1000);
          //heltec_delay(5000);

        }
      }
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      Serial.println("CRC mismatch (ignored)");
      Serial.print("CRC but RSSI: ");
      Serial.println(radio.getRSSI());
    }
    else if (state != RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.print("Receive failed: ");
    Serial.println(state);
    }
    else 
    {
      Serial.print(F("[LoRa] Read failed, code: "));
      Serial.println(state);  
    }
    delay(10);
    radio.setDio1Action(rx);
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF)); // Restart receive mode
  }

}

void EndDevice3(String EndDevice3_LoRaData)
{  
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
  Serial.println("\nFailed to obtain time");
    return;
  }
  
  char timevalue3[100];
  strftime(timevalue3, sizeof(timevalue3), "%H:%M:%S IST %d %B %Y %A ", &timeinfo);
  strftime(Gateway_timeString, sizeof(Gateway_timeString), "%H:%M:%S IST %d %B %Y", &timeinfo);

  heltec_delay(500); 

  // Parse the data
  int firstComma = EndDevice3_LoRaData.indexOf(',');
  int secondComma = EndDevice3_LoRaData.indexOf(',', firstComma + 1);
  int thirdComma = EndDevice3_LoRaData.indexOf(',', secondComma + 1);  
  int fourthComma = EndDevice3_LoRaData.indexOf(',', thirdComma + 1);
  int fifthComma = EndDevice3_LoRaData.indexOf(',', fourthComma + 1);
  int sixthComma = EndDevice3_LoRaData.indexOf(',', fifthComma + 1);
  counterTime++;
  
  // Extract substrings and convert to appropriate types (e.g., float)
  LoRa_Tx_DeviceID = EndDevice3_LoRaData.substring(0, firstComma);
  Log_Date = EndDevice3_LoRaData.substring(firstComma + 1, secondComma);
  Log_Time = EndDevice3_LoRaData.substring(secondComma + 1, thirdComma); 
  EmployeeName = EndDevice3_LoRaData.substring(thirdComma + 1, fourthComma);
  EmployeeUID = EndDevice3_LoRaData.substring(fourthComma + 1, fifthComma);
  Status = EndDevice3_LoRaData.substring(fifthComma + 1, sixthComma);
  
  //(1) Print to Serial Monitor
  Serial.print(F("S.No: "));          Serial.println(counterTime);
  Serial.print(F("Date: "));          Serial.println(Log_Date);
  Serial.print(F("Time: "));          Serial.println(Log_Time);
  Serial.print(F("Employee Name: ")); Serial.println(EmployeeName);
  Serial.print(F("Card ID: "));       Serial.println(EmployeeUID);
  Serial.print(F("Status: "));        Serial.println(Status);
  Serial.print(F("Gateway TimeStamp: "));  Serial.println(Gateway_timeString);
  heltec_delay(500);

  //(2) To send Data to Google Sheet
  // Call ready() repeatedly in loop for authentication checking and processing
  bool ready = GSheet.ready();
  while(!ready)
  {
    Serial.println(ready);
    Serial.println("Preparing Google Sheet");
    ready = GSheet.ready();
    heltec_delay(500);
  }
  if (ready)
  {
    FirebaseJson response;
    Serial.println("Append spreadsheet values...");
    Serial.println("----------------------------");

    FirebaseJson valueRange;
    if (!RFID_headerAdded) {
      // 1. Add Header (Only once)
      valueRange.add("majorDimension", "COLUMNS"); //"ROWS" or "COLUMNS"
      valueRange.set("values/[0]/[0]", "S.No."); //set[Column][Row]
      valueRange.set("values/[1]/[0]", "Date");
      valueRange.set("values/[2]/[0]", "Time");
      valueRange.set("values/[3]/[0]", "Employee Name");
      valueRange.set("values/[4]/[0]", "Access card UID");
      valueRange.set("values/[5]/[0]", "Status");
      
      bool success = GSheet.values.append(&response, RFID_spreadsheetId, "Sheet1!A1", &valueRange);
      if (success) {
        RFID_headerAdded = true; 
        Serial.println("Header added successfully.");
      }
      else
      {
        Serial.println(success);
        Serial.println("Header Not Added.");
      }
      valueRange.clear(); // Clear for next use
    }

    valueRange.add("majorDimension", "COLUMNS");  //"ROWS" or "COLUMNS"
    valueRange.set("values/[0]/[0]", counterTime); //set[Column][Row]
    valueRange.set("values/[1]/[0]", Log_Date);
    valueRange.set("values/[2]/[0]", Log_Time); 
    valueRange.set("values/[3]/[0]", EmployeeName);
    valueRange.set("values/[4]/[0]", EmployeeUID);
    valueRange.set("values/[5]/[0]", Status);

    // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
    // Append values to the spreadsheet
    bool success = GSheet.values.append(&response , RFID_spreadsheetId , "Sheet1!A1" , &valueRange );
    if (success){
        Serial.println("Data appended.");
        response.toString(Serial, true);
        valueRange.clear();
    }
    else{
        Serial.println(success);
        Serial.println("Data Not appended.");
        Serial.println(GSheet.errorReason());
    }
    Serial.println();
    Serial.println(ESP.getFreeHeap());
  }
  else
  {
    Serial.println(ready);
    Serial.println("Google Sheet Not Ready");
  }
  heltec_delay(500);

  //(3) To send Data to cloud
  // *** Create JSON Object ***
  // Use the ArduinoJson Assistant to determine the buffer size
  JsonDocument doc;       //StaticJsonDocument<512> doc;

  doc["S.No"] = counterTime;
  doc["LogDate"] = Log_Date;
  doc["LogTime"] = Log_Time;
  doc["Employee_Name"] = EmployeeName;
  doc["Card_ID"] = EmployeeUID;
  doc["Status"] = Status;
  doc["Gateway_TimeStamp"] = Gateway_timeString;
  // *** Serialize JSON to String ***
  char jsonBuffer3[1024];
  serializeJson(doc, jsonBuffer3);  //, sizeof(jsonBuffer3)
  Serial.printf("Publishing message to %s: %s\n", mqtt_topic_3, jsonBuffer3);

  // *** Publish to MQTT ***
  // Publish the message to the MQTT topic
  bool success = client.publish(mqtt_topic_3, jsonBuffer3);
  if (!success) {
    Serial.println("Publish failed! Buffer might still be too small.");
  }   else {
    Serial.println("MQTT Published Successfully");
  }
  heltec_delay(500);

  //(4) Display on built-in OLED 
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  display.drawString(0, 0, "Received Data");  
  sprintf(str, "Time: %s", Gateway_timeString);
  display.drawString(0, 15, str);                
  sprintf(str, "RSSI: %.2f dBm", radio.getRSSI());
  display.drawString(0, 30, str);                  
  sprintf(str, "SNR: %.2f dB", radio.getSNR());
  display.drawString(0, 45, str);    

  display.display(); 
}


void tokenStatusCallback(TokenInfo info){
  if (info.status == token_status_error){
      GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
      GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  }
  else{
      GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}
