#include <Wire.h>
#include <Adafruit_ICM20948.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

// Crear instancia del sensor
Adafruit_ICM20948 icm;

// Pines I2C para ESP32
#define SDA_PIN 5
#define SCL_PIN 4

// Dirección I2C del sensor (0x68 o 0x69 dependiendo de ADO)
#define ICM20948_I2C_ADDRESS 0x68 

// Credenciales WiFi
const char* ssid = "Red";
const char* password = "Password";

// Instancia del servidor web y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket en la ruta /ws

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Esperar a que se abra la consola serie

  // Configurar I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Iniciando prueba del ICM20948 con I2C...");

  // Inicializar el sensor con la dirección I2C correcta
  if (!icm.begin_I2C(ICM20948_I2C_ADDRESS, &Wire)) {
    Serial.println("No se encontró el chip ICM20948. Verifica las conexiones.");
    while (1) {
      delay(10);
    }
  }

  Serial.println("ICM20948 detectado con éxito!");

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configurar WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor WebSocket iniciado en /ws");
}

// Función para manejar eventos de WebSocket
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Cliente conectado");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("Cliente desconectado");
  }
}

// Bucle principal
void loop() {
  sensors_event_t accel, gyro, temp, mag;
  
  // Obtener datos de cada sensor
  icm.getTemperatureSensor()->getEvent(&temp);
  icm.getAccelerometerSensor()->getEvent(&accel);
  icm.getGyroSensor()->getEvent(&gyro);
  icm.getMagnetometerSensor()->getEvent(&mag);

  // Mostrar datos en el monitor serial
  Serial.print("Temperatura: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Aceleración [m/s^2]: X=");
  Serial.print(accel.acceleration.x);
  Serial.print(" Y=");
  Serial.print(accel.acceleration.y);
  Serial.print(" Z=");
  Serial.println(accel.acceleration.z);

  Serial.print("Giroscopio [rad/s]: X=");
  Serial.print(gyro.gyro.x);
  Serial.print(" Y=");
  Serial.print(gyro.gyro.y);
  Serial.print(" Z=");
  Serial.println(gyro.gyro.z);

  Serial.print("Magnetómetro [uT]: X=");
  Serial.print(mag.magnetic.x);
  Serial.print(" Y=");
  Serial.print(mag.magnetic.y);
  Serial.print(" Z=");
  Serial.println(mag.magnetic.z);
  
  Serial.println(); // Espacio entre bloque de datos

  // Crear cadena de datos para enviar por WebSocket
  String data = "T:" + String(temp.temperature) + ",";
  data += "A:" + String(accel.acceleration.x) + "," + String(accel.acceleration.y) + "," + String(accel.acceleration.z) + ",";
  data += "G:" + String(gyro.gyro.x) + "," + String(gyro.gyro.y) + "," + String(gyro.gyro.z) + ",";
  data += "M:" + String(mag.magnetic.x) + "," + String(mag.magnetic.y) + "," + String(mag.magnetic.z);

  // Enviar datos a todos los clientes conectados
  ws.textAll(data);

  delay(100); // Ajusta la frecuencia de muestreo y transmisión
}
