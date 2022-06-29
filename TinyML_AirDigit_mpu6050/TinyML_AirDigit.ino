//ngrok tcp 192.168.56.184:80 --authtoken 28vncXA65LijfQB6dNyAYfTbaMJ_7KyoGvqvzrTMSi1jevgtg
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>
#include "tinyml_airdigit.h"
#include "x_test.h"

#include <WiFi.h>
int A;
const char* ssid = "asd1234";
const char* password = "j1030287";

WiFiServer server(80);
String header;


#define N_INPUTS  90
#define N_OUTPUTS 3
// preallocate a certain amount of memory for input, output, and intermediate arrays.
#define TENSOR_ARENA_SIZE 16*1024 

#include "I2Cdev.h" 
#include "Wire.h"
#include <MPU6050.h>

String name_[3]={"門","手機震動","拖鞋"};
float y_pred[3] ={0};
int k=0;

MPU6050 imu;
float ax, ay, az;
int16_t accX, accY, accZ;

float sumX=0;
float sumY=0;
float sumZ=0;
float offsetX=0;
float offsetY=0;
float offsetZ=0;

#define sampleTime 50 // 50ms = 20Hz
#define periodTime 1500 // 1500ms
long loopTime, lastTime;
int samples = periodTime / sampleTime; // 1500/50 = 5 samples30
int counts;
bool sampleEn = false;

enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};

int Ascale = AFS_2G;
float aRes; // scale resolutions per LSB for the sensors

void getAres() {
  switch (Ascale)
  {
   // Possible accelerometer scales (and their register bit settings) are:
  // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11). 
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
          aRes = 2.0/32768.0;
          break;
    case AFS_4G:
          aRes = 4.0/32768.0;
          break;
    case AFS_8G:
          aRes = 8.0/32768.0;
          break;
    case AFS_16G:
          aRes = 16.0/32768.0;
          break;
  }
}

void getAcc() {
    imu.getAcceleration(&accX, &accY, &accZ);
    getAres();        
    ax = (float)accX * aRes -offsetX;  // get actual g value, this depends on scale being set
    ay = (float)accY * aRes -offsetY;   
    az = (float)accZ * aRes -offsetZ;   
}

  
Eloquent::TinyML::TensorFlow::TensorFlow<N_INPUTS, N_OUTPUTS, TENSOR_ARENA_SIZE> tf;
    
void setup() 
{  
  Wire.begin(); // default I2C clock is 100KHz
   Serial.begin(115200);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  
  
  
  pinMode(LED_BUILTIN, OUTPUT);
    
  // initialize IMU
  Serial.println("Initializing IMU...");
  imu.initialize();
  
  // verify connection
  Serial.println(imu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));  

  // calibration
  for (int i=0; i<100; i++)
  {
    getAcc();
    sumX += ax;
    sumY += ay;
    sumZ += az;
  }
  offsetX = sumX/100;
  offsetY = sumY/100;
  offsetZ = sumZ/100;
  Serial.print("Offset: ");
  Serial.print(offsetX); Serial.print("\t");
  Serial.print(offsetY); Serial.print("\t");
  Serial.print(offsetZ); Serial.println();  

  // pick up IMU to start sampling  
  while(!sampleEn) 
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);   
    getAcc();
    if (abs(az)>0.1) sampleEn=true;
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);      
  }
  
    lastTime = millis();
    
    tf.begin(tinyml_airdigit);
    Serial.println(sizeof(tinyml_airdigit));
}

void loop() 
{ 
  
    MPU_6050();
    
    
    

    uint32_t lastTime_InFer = micros(); 
    
    tf.predict(x_test_dat, y_pred);

    uint32_t inferTime = micros() - lastTime_InFer;

    Serial.print("The inference took ");
    Serial.print(inferTime);
    Serial.println(" us");

    for (int i=0; i<3;i++) 
    {
      Serial.print(y_pred[i]);
      Serial.print(i==2 ? '\n' : ',');
    }
    
    A=tf.probaToClass(y_pred);
    Serial.print("Predicted Class :");
    //Serial.println(tf.probaToClass(y_pred));
    Serial.println(name_[tf.probaToClass(y_pred)]);
    Serial.print("Sanity check:");
    
    //Serial.println(tf.predictClass(x_test_dat));
    Serial.println(name_[tf.predictClass(x_test_dat)]);
    wifi_();
 
}

void MPU_6050() 
{
    
    while(k<91)
    {
      loopTime = millis() - lastTime;
      if (loopTime >= sampleTime) 
      {
        getAcc();
        lastTime = millis();
      
        Serial.print(ax); Serial.print(','); 
        Serial.print(ay); Serial.print(',');
        Serial.print(az); Serial.println();
        
        x_test_dat[k]=ax;
        x_test_dat[k+1]=ay;
        x_test_dat[k+2]=az;
        
        k=k+3;
        counts++;
      }
    }
    
    if (counts>=samples) 
    {
      k=0;
      counts=0;
      Serial.println(); Serial.println(); Serial.println();
       
    }
    
}

void wifi_()
{
  WiFiClient client = server.available();   // Listen for incoming clients
  
    if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) 
    {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html>\n");
            client.println("<html>\n");
            client.println("<body>\n");
            client.println("<center>\n");
            
            A=tf.probaToClass(y_pred);
            
            switch(A+1){
              case 1:
              client.println("<h1 style=\"color:blue;\">door</h1>\n");
              break;
              case 2:
              client.println("<h1 style=\"color:blue;\">phone vibration</h1>\n");
              break;
              case 3:
              client.println("<h1 style=\"color:blue;\">slippers</h1>\n");
              break;
            }
            
            client.println("</center>\n");
            client.println("</body>\n");
            client.println("</html>");
            break;                       
          } 
          else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
}
  
  
  
}
