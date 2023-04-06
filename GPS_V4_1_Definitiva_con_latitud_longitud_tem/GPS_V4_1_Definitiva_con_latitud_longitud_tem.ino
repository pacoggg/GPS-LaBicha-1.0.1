#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


// Definir la clave API y el número de canal de ThingSpeak
const char* apiKey = "DEQILIFKWUF5UIF5";
const char* serverUrl = "http://api.thingspeak.com/update";

// Configura los pines GPIO5 y GPIO4 como pines de comunicación serial para el GPS
SoftwareSerial gpsSerial(5, 4);

// Crea un objeto TinyGPS++ para manejar los datos del GPS
TinyGPSPlus gps;

// Configura las credenciales de la red WiFi a la que se conectará el ESP8266
const char* ssid1 = "LaBicha";
const char* password1 = "123456789";
const char* ssid2 = "PacoMov";
const char* password2 = "123456789";
const char* ssid3 = "MiFibra-B460";
const char* password3 = "WuSAp49X";

void setup() {
  // Inicializa la comunicación serial a 115200 baudios para el puerto serie de la placa
  Serial.begin(115200);

  // Inicializa la comunicación serial a 9600 baudios para el GPS
  gpsSerial.begin(9600);

  // Inicializa el dht
  dht.begin();

  // Conecta el ESP8266 a la red WiFi
  connectToWiFi();
  
  
  // Espera a que el GPS se inicie y envíe datos
  delay(1000);
}

void loop() {
  // Comprobar conexión WIFI si no hay reconectar  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexión perdida, intentando reconectar...");
    connectToWiFi();
  }
  // Lee los datos del GPS y los almacena en el objeto TinyGPS++
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Si el objeto TinyGPS++ ha recibido una nueva posición válida del GPS
  if (gps.location.isValid()) {
    // Obtiene la posición actual del GPS
    float lat = gps.location.lat();
    float lon = gps.location.lng();

    // Imprime la posición actual del GPS en el monitor serial
    Serial.print("Latitud: ");
    Serial.println(lat, 6);
    Serial.print("Longitud: ");
    Serial.println(lon, 6);
    // Temperatura y Humedad
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    Serial.print("Temperatura: ");
    Serial.println(temp, 2);
    Serial.print("Humedad: ");
    Serial.println(hum, 2);


    // Envía la posición actual del GPS y la temperatura y humedad al servidor web
    enviarPosicion(lat, lon, temp, hum);
  }
}

void enviarPosicion(float latitud, float longitud, float temp, float hum) {
  // Crear un objeto HTTPClient
  WiFiClient client;
  HTTPClient http;

  // Construir la URL de solicitud
  char latitudStr[15];
  char longitudStr[15];
  char tempStr[15];
  char humStr[15];
  dtostrf(latitud, 8, 6, latitudStr);
  dtostrf(longitud, 8, 6, longitudStr);
  sprintf(tempStr, "%+06.2f", temp);  
  //dtostrf(temp, 5, 2, tempStr);
  sprintf(humStr, "%05.2f", hum);
  //dtostrf(hum, 5, 2, humStr);
  String url = String(serverUrl) + "?api_key=" + String(apiKey) + "&field1=" + String(latitudStr) + "&field2=" + String(longitudStr) + "&field3=" + String(tempStr) + "&field4=" + String(humStr);
  Serial.println(url);
  // Realizar la solicitud HTTP GET
  http.begin(client, url);
  int httpResponseCode = http.GET();

  // Comprobar el código de respuesta HTTP
  if (httpResponseCode == 200) {
    Serial.println("Posición enviada correctamente a ThingSpeak");
  } else {
    Serial.print("Error al enviar la posición a ThingSpeak. Código de respuesta: ");
    Serial.println(httpResponseCode);
  }

  // Liberar el objeto HTTPClient
  http.end();
}
void connectToWiFi() {
  Serial.print("Conectando a la red Wi-Fi...");
  int numSsid = 3; // Número de redes Wi-Fi conocidas
  const char* ssidList[] = { ssid1, ssid2, ssid3 };
  const char* passwordList[] = { password1, password2, password3 };
  WiFi.disconnect(); // Desconectar de la red actual (si se estaba conectado)
  for (int i = 0; i < numSsid; i++) {
    WiFi.begin(ssidList[i], passwordList[i]);
    Serial.print("Intentando conectar a ");
    Serial.println(ssidList[i]);
    unsigned long timeout = millis() + 10000; // Esperar máximo 10 segundos por cada red
    while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nConectado a %s, dirección IP: %s\n", ssidList[i], WiFi.localIP().toString().c_str());
      break; // Salir del ciclo si se logra conectar a una red
    } else {
      Serial.println();
      Serial.println("No se pudo conectar a la red");
    }
  }
}