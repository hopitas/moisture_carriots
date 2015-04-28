#include <SPI.h>
#include <Ethernet.h>
#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  8
#define clockPin 9
SHT1x sht1x(dataPin, clockPin);

// Soil sensor variables
float t;
float h;
float dewpoint;

const String APIKEY = "11a7724be71bb72976f5939612609412d564b7756b034e618953d3ca03f3b9f3"; // Replace with your Carriots apikey
const String DEVICE = "SoilSensor@esperanza.esperanza"; // Replace with the id_developer of your device

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0x96, 0xDD };
IPAddress ip(192, 168, 100, 107);    // Your IP Address
IPAddress server(82, 223, 244, 60); // api.carriots.com IP Address

EthernetClient client; // Initialize the library instance

// The setup routine runs once when you press reset
void setup() {

  Serial.begin(115200);             // Start serial port
  Serial.println(F("Starting"));

  Ethernet.begin(mac, ip);        // Start the Ethernet connection
  delay(1000);                    // Give the Ethernet shield a second to initialize
}

// The loop routine runs over and over again forever
void loop() {

  // Read values from the sensor
  t = sht1x.readTemperatureC();
  h = sht1x.readHumidity();

  // Convert data to String
  String temperature = doubleToString(t, 2);
  String humidity = doubleToString(h, 2);

  // Prepare JSON for Carriots & get length
  int length = 0;

  String data = "{\"protocol\":\"v2\",\"device\":\"" + String(DEVICE) + "\",\"at\":\"now\",\"data\":{\"Temperature\":" + String(temperature) + ",\"Humidity\":" + String(humidity) + "}}";

  length = data.length();
  Serial.print("Data length");
  Serial.println(length);
  Serial.println();

  // Print request for debug purposes
  Serial.println("POST /streams HTTP/1.1");
  Serial.println("Host: api.carriots.com");
  Serial.println("Accept: application/json");
  Serial.println("User-Agent: Arduino-Carriots");
  Serial.println("Content-Type: application/json");
  Serial.println("carriots.apikey: " + String(APIKEY));
  Serial.println("Content-Length: " + String(length));
  Serial.print("Connection: close");
  Serial.println();
  Serial.println(data);

  // Send request
  if (client.connect(server, 80)) {   // If there's a successful connection
    Serial.println("Connected!");
    client.println("POST /streams HTTP/1.1");
    client.println("Host: api.carriots.com");
    client.println("Accept: application/json");
    client.println("User-Agent: Arduino-Carriots");
    client.println("Content-Type: application/json");
    client.println("carriots.apikey: " + String(APIKEY));
    client.println("Content-Length: " + String(length));
    client.println("Connection: close");
    client.println();

    client.println(data);

  } else {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  Serial.println(F("-------------------------------------"));


  Serial.println(F("\n\nDisconnecting"));
  client.stop();


  // Wait 10 seconds until next update
  delay(3600000);

}

// Convert double to string
String doubleToString(float input, int decimalPlaces) {
  if (decimalPlaces != 0) {
    String string = String((int)(input * pow(10, decimalPlaces)));
    if (abs(input) < 1) {
      if (input > 0)
        string = "0" + string;
      else if (input < 0)
        string = string.substring(0, 1) + "0" + string.substring(1);
    }
    return string.substring(0, string.length() - decimalPlaces) + "." + string.substring(string.length() - decimalPlaces);
  }
  else {
    return String((int)input);
  }
}

