/******************************************************** SM PRoject ***************************************************/

/*
* Code to recieve data from
* and send the data to thingspeak cloud.
* 
* Using MQTT Protocol
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

#include <Servo.h>

#define ir1 16 //D0
#define ir2 5  //D1
#define ultra1_echo 4 //D2
#define ultra1_trig 0 //D3
#define ultra2_echo 2 //D4
#define ultra2_trig 14 //D5
#define servopin 12 //D6
#define servopin2 13 //D7

Servo myservo;
Servo myservo2;
int sensor1, ulsensor1;
int sensor2, ulsensor2;

String slot1;
String slot2;
String cv, book1, book2;

// Ensure correct credentials to connect to your WiFi Network.
char ssid[] = "Abbas-mi";
char pass[] = "12345678";

// Ensure that the credentials here allow you to publish and subscribe to the ThingSpeak channel.

#define channelID2 1009739

#define channelID 1678742
const char mqttUserName[] = "NgoHJB42MhwvMBsQDTkcPC4"; 
const char clientID[] = "NgoHJB42MhwvMBsQDTkcPC4";
const char mqttPass[] = "DLsufBWOnUxjPgDACe6AGYLk";

#define mqttPort 1883
WiFiClient client;

const char* server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 15;
PubSubClient mqttClient( client );

void connectWifi()
{
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  // Loop until connected.
  while ( !mqttClient.connected() )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    } else {
      Serial.print( "MQTT connection failed, rc = " );
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}

void setup() {
  Serial.begin( 115200 );
  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
  pinMode(ultra1_echo, INPUT);
  pinMode(ultra1_trig, OUTPUT);
  pinMode(ultra2_echo, INPUT);
  pinMode(ultra2_trig, OUTPUT);
  myservo.attach(servopin);
  myservo2.attach(servopin2);

  myservo.write(0);
  myservo2.write(0);
  
  // Delay to allow serial monitor to come up.
  delay(500);
  
  // Connect to Wi-Fi network.
  connectWifi();
  
  // Configure the MQTT client
  mqttClient.setServer( server, mqttPort ); 
  
  // Set the MQTT message handler function.
  mqttClient.setCallback( mqttSubscriptionCallback );
  
  // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize( 2048 );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Code to send and get data from cloud using MQTT
// Function to handle messages from MQTT subscription.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the details of the message that was received to the serial monitor.
  char myString[length];
  if(strcmp(topic,"channels/1009739/subscribe/fields/field1")==0) {
    for (int i = 0; i < length; i++) {
      myString[i] = (char)payload[i];
    }
    cv = String(myString);
    Serial.print("Model detection :: ");
    Serial.println(cv);
    Serial.println();
  }
  else if(strcmp(topic,"channels/1009739/subscribe/fields/field2")==0) {
    for (int i = 0; i < length; i++) {
      myString[i] = (char)payload[i];
    }
    book1 = String(myString);
    Serial.print("Number Booked Slot1 :: ");
    Serial.println(book1);
    Serial.println();
  }
  else if(strcmp(topic,"channels/1009739/subscribe/fields/field3")==0) {
    for (int i = 0; i < length; i++) {
      myString[i] = (char)payload[i];
    }
    book2 = String(myString);
    Serial.print("Number Booked Slot2 :: ");
    Serial.println(book2);
    Serial.println();
  }
}

// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID ){
  String myTopic = "channels/"+String( subChannelID )+"/subscribe/fields/field1";
  String myTopic1 = "channels/"+String( subChannelID )+"/subscribe/fields/field2";
  String myTopic2 = "channels/"+String( subChannelID )+"/subscribe/fields/field3";
  mqttClient.subscribe(myTopic.c_str());
  mqttClient.subscribe(myTopic1.c_str());
  mqttClient.subscribe(myTopic2.c_str());
}

// Publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Code to send and get status of slots from sensors
long ultrafunct(int trig, int echo){
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  return pulseIn(echo, HIGH);
}

void slot1_status(){
  float l = ultrafunct(ultra1_trig,ultra1_echo);
  ulsensor1 = 0.0173*l;
  delay(10);
  if(ulsensor1 <= 5){
    slot1 = "Filled";
  }
  else{
    slot1 = "Empty";
  }  
}

void slot2_status(){
  float l = ultrafunct(ultra2_trig,ultra2_echo);
  ulsensor2 = 0.0173*l;
  delay(10);
  if(ulsensor2 <= 5){
    slot2 = "Filled";
  }
  else{
    slot2 = "Empty";
  }
}

void check_slots(){
  sensor1 = digitalRead(ir1);
  slot1_status();
  slot2_status();

  if(sensor1 == 0){
    if(slot1 == "Empty" or slot2 == "Empty"){
      // open servo
      myservo.write(180);
      delay(3000);
      myservo.write(0);
    }
  }

  slot1_status();
  slot2_status();
}

void out(){
  sensor2 = digitalRead(ir2);

  if(sensor2 == 0){
    myservo2.write(180);
    delay(3000);
    myservo2.write(0);
    slot1_status();
    slot2_status();
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() {
  // Reconnect to WiFi if it gets disconnected.
  slot1 = "Empty";
  slot2 = "Empty";
  
  check_slots();
  
  if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
  }
  
  // Connect if MQTT client is not connected and resubscribe to channel updates.
  if (!mqttClient.connected()) {
     mqttConnect();
     mqttSubscribe( channelID2 );
  }
  
  // Call the loop to maintain connection to the server.
  mqttClient.loop(); 

  out();        // Checking IR sensor at out-let

  if(book1 != ""){
    slot1 = "Booked";
  }
  if(book2 != ""){
    slot2 = "Booked";
  }

  if(book1 == cv | book2 == cv){
    myservo.write(180);
    delay(3000);
    myservo.write(0);
  }
  
  // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
  mqttPublish( channelID, (String("field1=")+slot1+"&"+String("field2=")+slot2) );

  Serial.println("Slot -1 :"+slot1+", Slot -2 :"+slot2);
  delay(5000);
}
