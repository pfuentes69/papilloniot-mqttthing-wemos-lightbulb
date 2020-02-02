#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//IPAddress server(192,168,2,10); // Casa

IPAddress server(35,178,36,205); // Solace

const int relayPin = D1;
const int ledPin = D4;
const int buttonPin = D5;
const int LDRPin = A0;

const char* ssid     = "Papillon"; //Naviter"; //
const char* password = "70445312"; //N4v1t3rWIFI2015"; //

const char* devID = "dev02";
const char* devUS = "dev02";
const char* devPW = "dev02";

const char* devTopic = "PapillonIoT/dev02/cmd/+";

const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

const int thresholdLDR = 700; // Light level to turn on Lamp
const int delayLDR = 200; //ms

int status = WL_IDLE_STATUS;   // the Wifi radio's status

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

bool estadoLampara = false;
bool viejoEstadoLampara = false;
bool forzarEstado = false;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  Serial.println("<<<<< SETUP START >>>>>");
  // initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  bool connectingWIFI = true;
  int tries = 0;

  Serial.println("Trying Main WiFi");
  while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
    
  if (tries >= 10) {
    Serial.println("Too many trials, no WiFi connection was possible");
  } else {
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
    //connect to MQTT server
    client.setServer(server, 1883);
    client.setCallback(callback);
  
  //  client.connect("dev02", "dev02", "dev02");
    reconnect();
    Serial.println("Network OK");
  }

  // Set Relay
  pinMode(relayPin, OUTPUT);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);

  // Set Button
  pinMode(buttonPin, INPUT_PULLUP);

  // Estado inicial
  controlLampara(estadoLampara);
  Serial.println("<<<<< SETUP END >>>>>");
}

void loop() {
  viejoEstadoLampara = estadoLampara;

  // First we check if the WiFi is still OK, or we reconnect
  if (WiFi.status() != WL_CONNECTED) {
    bool connectingWIFI = true;
    int tries = 0;
  
    Serial.println("Trying Main WiFi");
    while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
      delay(500);
      Serial.print(".");
      tries++;
    }
    Serial.println();
      
    if (tries >= 10) {
      Serial.println("Too many trials, no WiFi connection was possible");
    } else {
      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    
      //connect to MQTT server
      client.setServer(server, 1883);
      client.setCallback(callback);    
      reconnect();
      Serial.println("Network OK");
    }
  } else {
    // WiFi is OK, we loop the MQTT client
    if (!client.connected()) {
      reconnect();
    } 
    client.loop();  
  }

  // Update Lampara Status and sensors
  updateLampara();

  // Check new status
  if (estadoLampara != viejoEstadoLampara) {
     // set the Lampara:
    controlLampara(estadoLampara);
  }
}

/////////////////////////////////////
//
// MQTT FUNCTIONS
//
/////////////////////////////////////

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  String sTopic = topic;
  String sCommand = sTopic.substring(sTopic.lastIndexOf("/") + 1);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String sPayload = "";
  for (int i=0;i<length;i++) {
    sPayload += (char)payload[i];
  }
  Serial.println(sPayload);

  Serial.println("Command: " + sCommand);

  if (sCommand == "ledoff") {
      if (sPayload == "ON") {
        controlLED(true);
      } else {
        controlLED(false);
      }
  } else if (sCommand == "relay") {
      forzarEstado = true;
      if (sPayload == "ON") {
        estadoLampara = true;
      } else {
        estadoLampara = false;
      }
  } else if (sCommand == "cmd1off") {
    Serial.println("Comando 1, Payload: " + sPayload);
    controlLED(true);
    delay(100);    
    controlLED(false);
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else if (sCommand == "cmd2off") {
    Serial.println("Comando 2, Payload: " + sPayload);
    controlLED(true);
    delay(100);    
    controlLED(false);
    delay(200);
    controlLED(true);
    delay(100);    
    controlLED(false);
  } else if (sCommand == "cmd3off") {
    Serial.println("Comando 3, Payload: " + sPayload);
    controlLED(true);
    delay(100);    
    controlLED(false);
    delay(200);    
    controlLED(true);
    delay(100);    
    controlLED(false);
    delay(200);    
    controlLED(true);
    delay(100);    
    controlLED(false);
  } else {    
      Serial.println("Invalid command!");
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect(devID, devUS, devPW)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(devTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////////////////////////////////////
//
// CUSTOM HW FUNCITONS
//
/////////////////////////////////////

void controlLED (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender LED");
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (LOW is the voltage level)
  }
  else {
    Serial.println("Apagar LED");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off (HIGH is the voltage level)    
  }
}

void controlRelay (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender Relay");
    digitalWrite(relayPin, HIGH);   // turn the RELAY on (HIGH is the voltage level)    
  } 
  else {
    Serial.println("Apagar Relay");
    digitalWrite(relayPin, LOW);   // turn the RELAY off (LOW is the voltage level)
  }
}

void controlLampara (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender Lampara");
    controlLED(false);
    controlRelay(true);
  }
  else {
    Serial.println("Apagar Lampara");
    controlLED(true);
    controlRelay(false);
  }
}

void controlButton() 
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == LOW) {
        estadoLampara = !estadoLampara;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void controlLDR()
{
  if (forzarEstado) {
    // We terminate here
    return;
  }
  
  int sensorValue = analogRead(LDRPin);

  if (estadoLampara && sensorValue < thresholdLDR) {
    // Hay luz, apagamos
    estadoLampara = false;
  } else if (!estadoLampara && sensorValue >= thresholdLDR) {
    // No hay luz, encendemos
    estadoLampara = true;
  }

  delay(delayLDR);
}

void updateLampara()
{
  // Read analog
//  controlLDR();
  
  // Read button
  controlButton();

}

