#include <Wire.h>
#include "MAX30105.h"
#include <math.h>
#include <SPI.h>
//#include <TFT_eSPI.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include "time.h"
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

//define Firebase
#define WIFI_SSID "grieentea"
#define WIFI_PASSWORD "manggagoreng"
#define API_KEY "AIzaSyBK7e2gTo508Ql4-k3oA7bfQGQ19wLHmn4"
#define USER_EMAIL "pasien_mvs1@gmail.com"
#define USER_PASSWORD "12345678"
#define DATABASE_URL "https://monitoring-vital-sign-957a5-default-rtdb.asia-southeast1.firebasedatabase.app/"
/*//tft and button setup
#define umurbtn D0
#define enterbtn D1
#define WIDTH 240
#define HEIGHT 240*/

//TFT_eSPI tft = TFT_eSPI();
MAX30105 particleSensor;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;  // Variable to save USER UID
String databasePath; // Database main path (to be updated in setup with the user UID)

// Database child nodes
String hrPath = "/hr";
String rrPath = "/rr";
String spoPath = "/spo";
String sysPath = "/sys";
String dysPath = "/dys";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

//setup display tft
int rrBoxX = 20;
int rrBoxY = 70;
int rrBoxWidth = 80;
int rrBoxHeight = 70;
int bpBoxX = 130;
int bpBoxY = 70;
int bpBoxWidth = 90;
int bpBoxHeight = 70;
int hrBoxX = 20;
int hrBoxY = 160;
int hrBoxWidth = 80;
int hrBoxHeight = 70;
int spo2BoxX = 130;
int spo2BoxY = 160;
int spo2BoxWidth = 90;
int spo2BoxHeight = 70;

// Variabel untuk mengukur umur
int duration, umur = 0;
bool umurTersimpan = false;

//Variabel untuk button umur
unsigned long lastUmurTime = 0;
const unsigned long umurTimeout = 3000;

/*String server = "http://maker.ifttt.com";
String eventName = "DATA_MVS";
String IFTTT_Key = "c1tNcPFeE4cuLmSiwbVz0G";
String IFTTTUrl="https://maker.ifttt.com/trigger/DATA_MVS/with/key/c1tNcPFeE4cuLmSiwbVz0G";*/

unsigned long currentTime, startTime;

bool flag, flagRR = false;

//coeff fc 0.5-3 orde 3
float a_filt[] = {-4.5287, 8.7324, -9.2347, 5.6724, -1.9190, 0.2781};
float b_filt[] = {0.0181, 0, -0.0543, 0, 0.0543, 0, -0.0181};
float hr, hrn1, hrn2, hrn3, hrn4, hrn5, hrn6; //sinyal asli
float hrFilt, hrFilt1, hrFilt2, hrFilt3, hrFilt4, hrFilt5, hrFilt6; //sinyal filter
float filtSignal[] = {0,0,0,0,0,0};
float a, b, c, d, RR, red, SpO;

int numPeaks = 0;
int RRint, SpOint, sistolik, diastolik;

unsigned long sendDataPrevMillis = 0;
const int ARRAY_SIZE = 1500;
int signalValues[ARRAY_SIZE];
int currentIndex = 0;

/*void button1Pressed() {
  // Tambahkan umur
  umur++;
  clrScrHdr();
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print(">Set umur: ");
  tft.println(umur);
}

void clrScrHdr(){
  tft.fillScreen(TFT_BLACK); 
  tft.fillRect(0, 0, 240, 50, TFT_NAVY);
  tft.setTextSize(2);
  tft.setTextPadding(240);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Pengukuran TTV", 120, 10, 2);
}

void displayOneMinsWaiting(){
  clrScrHdr();
  tft.setTextSize(2);
  tft.setCursor(55, 80);
  tft.printf("Mohon Untuk");
  tft.setCursor(15, 100);
  tft.printf("Tetap Relax Selama");
  tft.setTextSize(3);  
  tft.setCursor(100, 150);
  tft.print((duration-(currentTime - startTime))/1000); //countdown 60s
  tft.printf("s");
}

void putFinger(){
  clrScrHdr();  
  tft.setTextSize(2);
  tft.setCursor(70,80);
  tft.printf("Letakkan");
  tft.setCursor(35,100);
  tft.printf("Telunjuk Kanan");
}

//keadaan normal
void tampilkanttvnormal(){
  tft.fillScreen(TFT_BLACK); 
  tft.fillRect(0, 0, 240, 50, TFT_NAVY);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Pengukuran TTV", 120, 10, 2);
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan semua TTV error
void tampilkanttverror(){
  tft.fillScreen(TFT_BLACK); 
  tft.fillRect(0, 0, 240, 50, TFT_NAVY);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Pengukuran TTV", 120, 10, 2);
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

void dibawahUmur(){
  tft.fillScreen(TFT_BLACK); 
  tft.fillRect(0, 0, 240, 50, TFT_NAVY);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Pengukuran TTV", 120, 10, 2);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Maaf, Umur anda", 120, 100, 2);
  tft.drawString("diluar klasifikasi", 120, 130, 2);
}

//tampilan RR abnormal
void tampilkanrrerror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan BP abnormal
void tampilkanbperror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan HR abnormal
void tampilkanhrerror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan SpO2 abnormal
void tampilkanspoerror(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan rr bp error
void tampilkanrrbperror(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan rr hr error
void tampilkanrrhrerror(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan rr spo error
void tampilkanrrspoerror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"

  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan bp hr error
void tampilkanbphrerror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan bp spo error
void tampilkanbpspoerror(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan hr spo error
void tampilkanhrspoerror(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan rr normal
void tampilkanrrnormal(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan bp normal
void tampilkanbpnormal(){
 clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_GREEN);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan hr normal
void tampilkanhrnormal(){
  clrScrHdr();

  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_GREEN);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_RED);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

//tampilan spo normal
void tampilkansponormal(){
  clrScrHdr();
  // Menggambar kotak hijau di sekitar "RR"
  tft.drawRect(rrBoxX, rrBoxY, rrBoxWidth, rrBoxHeight, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(rrBoxX + 10, rrBoxY + 10);
  tft.print("RR: ");
  tft.setCursor(rrBoxX + 10, rrBoxY + 40);
  tft.println(RRint);

  // Menggambar kotak hijau di sekitar "BP"
  tft.drawRect(bpBoxX, bpBoxY, bpBoxWidth, bpBoxHeight, TFT_RED);
  tft.setCursor(bpBoxX + 10, bpBoxY + 10);
  tft.print("BP: ");
  tft.setCursor(bpBoxX + 10, bpBoxY + 40);
  tft.print(sistolik);
  tft.print("/");
  tft.println(diastolik);
  
  // Menggambar kotak hijau di sekitar "HR"
  tft.drawRect(hrBoxX, hrBoxY, hrBoxWidth, hrBoxHeight, TFT_RED);
  tft.setCursor(hrBoxX + 10, hrBoxY + 10);
  tft.print("HR: ");
  tft.setCursor(hrBoxX + 10, hrBoxY + 40);
  tft.println(numPeaks);

  // Menggambar kotak hijau di sekitar "SpO2"
  tft.drawRect(spo2BoxX, spo2BoxY, spo2BoxWidth, spo2BoxHeight, TFT_GREEN);
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 10);
  tft.print("SpO2: ");
  tft.setCursor(spo2BoxX + 10, spo2BoxY + 40);
  tft.print(SpOint);
  tft.println("%");
}

void tampilkanHalamanPengukuran(){
  if(umur >= 14 && umur <= 18){
    if((numPeaks >=60 && numPeaks <= 90) && (RRint >= 12 && RRint <= 16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanttvnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint < 12 || RRint > 16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint >= 12 && RRint <= 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbperror();
    }
    else if((numPeaks < 60 ||numPeaks > 90) && (RRint >= 12 && RRint <=16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanhrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint >= 12 && RRint <=16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint < 95)){
      tampilkanspoerror();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint < 12 || RRint > 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrbperror();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint < 12 || RRint > 16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrhrerror();
    }
    else if((numPeaks >= 60 && numPeaks <= 90) && (RRint < 12 || RRint > 16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint < 95)){
      tampilkanrrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint < 12 || RRint > 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbphrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint >= 12 && RRint <= 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint < 95)){
      tampilkanbpspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint >= 12 && RRint <=16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint < 95)){
      tampilkanhrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint >= 12 && RRint <=16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint < 95)){
      tampilkanrrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint < 12 || RRint > 16) && (sistolik >=80 && sistolik <= 120 || diastolik >= 40 && diastolik <= 80) && (SpOint < 95)){
      tampilkanbpnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 90) && (RRint < 12 || RRint > 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint < 95)){
      tampilkanhrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 90) && (RRint < 12 || RRint > 16) && (sistolik <80 || sistolik > 120 || diastolik < 40 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkansponormal();
    }
    else{
    tampilkanttverror();
    }
  }
  else if(umur >= 19 && umur <= 40){
    if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanttvnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbperror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanhrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <=20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint < 95)){
      tampilkanspoerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrbperror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrhrerror();
    }
    else if((numPeaks >= 60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint < 95)){
      tampilkanrrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbphrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint < 95)){
      tampilkanbpspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint < 95)){
      tampilkanhrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint < 95)){
      tampilkanrrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik >=95 && sistolik <= 135 || diastolik >= 60 && diastolik <= 80) && (SpOint < 95)){
      tampilkanbpnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint < 95)){
      tampilkanhrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik <95 || sistolik > 135 || diastolik < 60 || diastolik > 80) && (SpOint >= 95 && SpOint <=100)){
      tampilkansponormal();
    }
    else{
    tampilkanttverror();
    }
  }
  else if(umur >= 41 && umur <= 60){
    if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanttvnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbperror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanhrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <=20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint < 95)){
      tampilkanspoerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrbperror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanrrhrerror();
    }
    else if((numPeaks >= 60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint < 95)){
      tampilkanrrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkanbphrerror();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint >= 12 && RRint <= 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint < 95)){
      tampilkanbpspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint < 95)){
      tampilkanhrspoerror();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint >= 12 && RRint <=20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint < 95)){
      tampilkanrrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik >=110 && sistolik <= 145 || diastolik >= 70 && diastolik <= 90) && (SpOint < 95)){
      tampilkanbpnormal();
    }
    else if((numPeaks >=60 && numPeaks <= 100) && (RRint < 12 || RRint > 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint < 95)){
      tampilkanhrnormal();
    }
    else if((numPeaks < 60 || numPeaks > 100) && (RRint < 12 || RRint > 20) && (sistolik <110 || sistolik > 145 || diastolik < 70 || diastolik > 90) && (SpOint >= 95 && SpOint <=100)){
      tampilkansponormal();
    }
    else{
    tampilkanttverror();
    }
  }
  else{
    dibawahUmur();
  }
}*/

void hrFilter(){
  //orde 3 25hz
  hrFilt = b_filt[0]*hr + b_filt[1]*hrn1 + b_filt[2]*hrn2 + b_filt[3]*hrn3 + b_filt[4]*hrn4 + b_filt[5]*hrn5 + b_filt[6]*hrn6 - a_filt[0]*hrFilt1 - a_filt[1]*hrFilt2 - a_filt[2]*hrFilt3 - a_filt[3]*hrFilt4 - a_filt[4]*hrFilt5 - a_filt[5]*hrFilt6;

  hrn6 = hrn5; hrn5 = hrn4; hrn4 = hrn3; hrn3 = hrn2; hrn2 = hrn1; hrn1 = hr;
  hrFilt6 = hrFilt5; hrFilt5 = hrFilt4; hrFilt4 = hrFilt3; hrFilt3 = hrFilt2; hrFilt2 = hrFilt1; hrFilt1 = hrFilt;
}

void rrCalculate(){
  filtSignal[4] = filtSignal[3];
  filtSignal[3] = filtSignal[2];
  filtSignal[2] = filtSignal[1];
  filtSignal[1] = filtSignal[0];
  filtSignal[0] = hrFilt;

  a = max(filtSignal[4], filtSignal[3]);
  b = max(a, filtSignal[2]);
  c = max(b, filtSignal[1]);
  d = max(c, filtSignal[0]);
  
  int threshold = 0;  
  if(d > threshold && filtSignal[0] > threshold){
    if(!flagRR){
    numPeaks++;
    flagRR = true;
    while (d < threshold) {
      a=b=c=d=0;
      }      
    }
  }
  else {
    flagRR = false;
  }
  RR = (numPeaks/4.5) *60/59.96;
  RRint = round(RR);
}

/*void sendArrayRTDB(){
  FirebaseJsonArray arr;

  int signal_len = sizeof(signalValues)/sizeof(signalValues[0]);

  tft.setTextSize(2);
  tft.setCursor(10,80);
  tft.printf("Send Array ... ");

  for (int i = 0; i < 750; i++) {
    Serial.print(signalValues[i]); Serial.print("\t");
    arr.add(signalValues[i]);
  }
  
  Serial.println("");
  Serial.printf("Send array... %s\n", Firebase.RTDB.setArray(&fbdo, "/raw/array", &arr) ? "ok" : fbdo.errorReason().c_str()); 

  arr.clear();

  for (int i = 751; i < 1500; i++) {
    Serial.print(signalValues[i]); Serial.print("\t");
    arr.add(signalValues[i]);
  }

  Serial.println("");
  Serial.printf("Send array... %s\n", Firebase.RTDB.setArray(&fbdo, "/raw/array1", &arr) ? "ok" : fbdo.errorReason().c_str());
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);
  tft.printf("OK!");                  
}

void storeValue(int value){
  signalValues[currentIndex] = value;
  currentIndex = (currentIndex + 1) % ARRAY_SIZE;
}

void getdata(){
  tft.setTextColor(TFT_GREEN);
  tft.printf("OK!");  
  if (Firebase.RTDB.getInt(&fbdo, "/BP/2023-07-12 12:30:11/Sistolik")) {
    if (fbdo.dataType() == "int") {
      sistolik = fbdo.intData();
      Serial.println(sistolik);
    }
  }
  else {
    Serial.println(fbdo.errorReason());
  }

  if (Firebase.RTDB.getInt(&fbdo, "/BP/2023-07-12 12:30:11/Diastolik")) {
    if (fbdo.dataType() == "int") {
      diastolik = fbdo.intData();
      Serial.println(diastolik);
      tft.setCursor(40, 140);
      tft.printf("All Done"); 
      tft.setTextColor(TFT_WHITE);
    }
  }
  else {
    Serial.println(fbdo.errorReason());
  }
}*/

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup()
{
  Serial.begin(115200);
  
  //button setup
  /*pinMode(umurbtn, INPUT_PULLUP);
  pinMode(enterbtn, INPUT_PULLUP);

  // Inisialisasi TFT ST7789
  tft.begin();
  tft.setRotation(4);

  // Tampilkan halaman awal
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 240, 50, TFT_NAVY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Pengukuran TTV", 120, 10, 2);*/

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting to Wi-Fi"));
  /*tft.setCursor(10,80);
  tft.printf("Menyambungkan Wi-Fi");*/
  int dot = 35;
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    /*tft.setCursor(dot,110);
    tft.print(".");*/
    delay(300);
    dot = dot + 10;
  }
  Serial.println();
  Serial.print(F("Connected with IP: "));
  /*tft.setTextColor(TFT_GREEN);
  tft.setCursor(60,140);
  tft.printf("Connected!");*/
  Serial.println(WiFi.localIP());
  /*tft.setTextColor(TFT_WHITE);*/

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  config.max_token_generation_retry = 5; // Assign the maximum retry of token generation

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

    // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  
  Serial.println(F("Initializing Sensor..."));
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power. "));
    while (1);
  }
  // Serial.println("Place your index finger on the sensor with steady pressure.");

  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 2; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

void loop()
{
  red = particleSensor.getRed();
  hr = particleSensor.getIR(); //get raw signal from IR
  
  /*// button - tft working
  int umurState = digitalRead(umurbtn);
  int enterState = digitalRead(enterbtn); */

  /*if (umurState == LOW) {
    button1Pressed();
    delay(200);
    Serial.println(umur);
    lastUmurTime = millis();
    umurTersimpan = true;
  }*/

  /*if (millis() - lastUmurTime > umurTimeout && umurTersimpan == true){
    putFinger();
    Serial.println("Put finger");
    umurTersimpan = false;
  }*/

    currentTime = millis();    
    if(hr > 50000 && umurTersimpan == false){
      if(!flag){
        flag = true;
        startTime = millis();
      } 
      duration = 5000;
      if(currentTime - startTime >= duration) {
        duration = 60000 + duration + 1000;
        if(currentTime - startTime <= duration + 200){
          //displayOneMinsWaiting(); 
          //////Heart rate process//////
          Serial.print(hr); Serial.print(F("\t"));
          int signal = hr;  
          //storeValue(signal);
        
          //////Filtering signal///////
          hrFilter(); 
          // Serial.print(hrFilt); Serial.print(F("\t"));
          // Serial.print(F(" HR=")); Serial.print(numPeaks); Serial.print(F("\t"));
          
          //////Extraction RR//////
          rrCalculate();
          // Serial.print(F(" RR=")); Serial.print(RRint); Serial.print(F("\t"));

          float ratio = red/hr;
          SpO = 104 - 17 * ratio;
          SpOint = round(SpO) + 10;
          // Serial.print(F(" SpO2=")); Serial.print(SpOint); Serial.print(F("\t"));
          Serial.println(); 
        } else {
          if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
            sendDataPrevMillis = millis();
            /*clrScrHdr();
            sendArrayRTDB();
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.setCursor(20,120);
            tft.printf("Get BP ... ");
            delay(5000);
            getdata();
            delay(2000);*/

          //Get current timestamp
          timestamp = getTime();
          Serial.print ("time: ");
          Serial.println (timestamp);

          parentPath= databasePath + "/" + String(timestamp);

          json.set(hrPath.c_str(), String(numPeaks));
          json.set(rrPath.c_str(), String(RRint));
          json.set(spoPath.c_str(), String(SpOint));
          //json.set(presPath.c_str(), String(dht.readPressure()/100.0F));
          json.set(timePath, String(timestamp));
          Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

          }
          //tampilkanHalamanPengukuran();
          // Serial.printf("Get array... %s\n", Firebase.RTDB.getArray(&fbdo, "/raw/array") ? fbdo.to<FirebaseJsonArray>().raw() : fbdo.errorReason().c_str());
          // Serial.printf("Get array... %s\n", Firebase.RTDB.getArray(&fbdo, "/raw/array1") ? fbdo.to<FirebaseJsonArray>().raw() : fbdo.errorReason().c_str());
        }
      }
    } else {
      // Serial.println("No Finger");
      flag = false;
    } 

}