#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "7SPOT";
// char pass[] = "misakamikoto";
unsigned int localPort = 2390;
int hour, min, sec;

IPAddress timeServerIP;
const char* ntpServerName = "ntp.nict.jp";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;

void setup()
{
  pinMode(D7, OUTPUT);     // boot beep
  tone(D7, 349, 50);
  delay(50);
  tone(D7, 523, 50);
  delay(50);
  
  Serial.begin((9600));
  delay(500);
  Serial.println();
  Serial.println();
  
//   modeset();
  
  clockset();
  
}

// void modeset()
// {
//   
// }

void clockset()
{
  connect2wifi();          //  wifiに接続
  warten();                //  正時まで待つ
  
  Serial.println("正時");   //  正時をお知らせ
  tone(D7, 2000, 50);
  delay(100);
  tone(D7, 1000, 50);
  delay(100);
}


void connect2wifi()
{
  Serial.print("Connecting");
  unsigned long ctime1 = millis();
  WiFi.begin(ssid, pass);  //try to connect
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");     //wait for connection
    delay(100);
  }
  unsigned long ctime2 = millis();
  Serial.print("IP address obtained. (");
  Serial.print(ctime2 - ctime1);
  Serial.println(" ms)");
  udp.begin(localPort);
}

unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void getepoch()
{
  Serial.print("NTP packet sending");
  unsigned long stime1 = millis();
  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP);
  int cb = udp.parsePacket();
  while ( udp.parsePacket() == 0 ) {
    Serial.print(".");     //wait for packet
    delay(10);
  }
  unsigned long stime2 = millis();
  Serial.print("received. (");
  Serial.print(stime2 - stime1);
  Serial.println(" ms)");
  udp.read(packetBuffer, NTP_PACKET_SIZE);

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1970 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = secsSince1970 - seventyYears;
  
  hour = (epoch % 86400L) / 3600;
  min  = (epoch % 3600) / 60;
  sec  = (epoch % 60);
  
  timeprint();    //  デバッグ用
}

void timeprint()  //  デバッグ用
{
  Serial.print("The JST time is ");
  if ( hour   < 10 ) { Serial.print('0'); }
  Serial.print(hour);
  Serial.print(':');
  if ( min < 10 ) { Serial.print('0'); }
  Serial.print(min);
  Serial.print(':');
  if ( sec < 10 ) { Serial.print('0'); }
  Serial.println(sec);
}

void warten()
{
  getepoch();
  
  if (min <= 30)               //  00:sec~30:sec
  {
    Serial.println("00:sec~30:sec");
    delay((55 - min) * 60 * 1000);   //  wait untill 55:sec
    getepoch();                      //  時刻を取得
  }
  
  if (30 < min && min <= 55)   //  31:sec~55:sec
  {
    Serial.println("31:sec~55:sec");
    delay((57 - min) * 60 * 1000);   //  wait untill 57:sec
    getepoch();                      //  時刻を取得
  }
  
  if (55 < min && min <= 58)   //  56:sec~58:sec
  {
    Serial.println("56:sec~58:sec");
    delay((58 - min) * 60 * 1000);   //
    delay((90 - sec) * 1000);        //  59:30まで待って
    getepoch();                      //  時刻を取得
  }
  
  if (59 == min && sec <= 50)  //  59:sec~59:50
  {
    Serial.println("59:sec~59:50");
    delay((60 - sec) * 1000);        //  正分まで待って
    getepoch();                      //  時刻を取得
  }
  
}

void loop()
{
  
}
