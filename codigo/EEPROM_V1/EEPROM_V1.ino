#include <EEPROM.h>

char endBroker[40] = "200.143.198.220";
char portBroker[40] = "1883";
char userBroker[40] = "picg";
char passBroker[40] = "i2spicgIFF";
char topicoMQTT[40] = "paraibasul";
char ssidwifi[40] = "linus";
char passwifi[40] = "lapela593";
char staticIP[40] = "192.168.15.200";
char subnetWiFi[40] = "255.255.255.0";
char gatewayWiFi[40] = "192.168.15.1";
char dnsWiFi[40] = "4.2.2.2";
char ssidAp[40] = "TroqueSSID";
char passAp[40] = "12345678";
char userConf[40] = "admin";
char passConf[40] = "123456";
char tag[40] = "LT100";
char name[40] = "Paraiba do Sul";
char lat[40] = "-21.62018027266234";
char lon[40] = "-41.031522464095616";
char extrainfo[40] = "sensor_Paraiba_do_Sul_Foz";

unsigned int endereco = 0;

void lerEEPROM() {

  endereco = 0;
  EEPROM.get(endereco, endBroker);
  endereco += 40;
  EEPROM.get(endereco, portBroker);
  endereco += 40;
  EEPROM.get(endereco, userBroker);
  endereco += 40;
  EEPROM.get(endereco, passBroker);
  endereco += 40;
  EEPROM.get(endereco, topicoMQTT);
  endereco += 40;
  EEPROM.get(endereco, ssidwifi);
  endereco += 40;
  EEPROM.get(endereco, passwifi);
  endereco += 40;
  EEPROM.get(endereco, staticIP);
  endereco += 40;
  EEPROM.get(endereco, subnetWiFi);
  endereco += 40;
  EEPROM.get(endereco, gatewayWiFi);
  endereco += 40;
  EEPROM.get(endereco, dnsWiFi);
  endereco += 40;
  EEPROM.get(endereco, ssidAp);
  endereco += 40;
  EEPROM.get(endereco, passAp);
  endereco += 40;
  EEPROM.get(endereco, userConf);
  endereco += 40;
  EEPROM.get(endereco, passConf);
  endereco += 40;
  EEPROM.get(endereco, tag);
  endereco += 40;
  EEPROM.get(endereco, name);
  endereco += 40;
  EEPROM.get(endereco, lat);
  endereco += 40;
  EEPROM.get(endereco, lon);
  endereco += 40;
  EEPROM.get(endereco, extrainfo);
  endereco += 40;
  endereco = 0;
  char dado[40];
  for ( byte i = 0; i < 24; i++)
  {
    EEPROM.get(endereco, dado);
    Serial.println(dado);
    endereco += 40;
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(2048);

  endereco = 0;

  EEPROM.put(endereco, endBroker);
  endereco += 40;
  EEPROM.put(endereco, portBroker);
  endereco += 40;
  EEPROM.put(endereco, userBroker);
  endereco += 40;
  EEPROM.put(endereco, passBroker);
  endereco += 40;
  EEPROM.put(endereco, topicoMQTT);
  endereco += 40;
  EEPROM.put(endereco, ssidwifi);
  endereco += 40;
  EEPROM.put(endereco, passwifi);
  endereco += 40;
  EEPROM.put(endereco, staticIP);
  endereco += 40;
  EEPROM.put(endereco, subnetWiFi);
  endereco += 40;
  EEPROM.put(endereco, gatewayWiFi);
  endereco += 40;
  EEPROM.put(endereco, dnsWiFi);
  endereco += 40;
  EEPROM.put(endereco, ssidAp);
  endereco += 40;
  EEPROM.put(endereco, passAp);
  endereco += 40;
  EEPROM.put(endereco, userConf);
  endereco += 40;
  EEPROM.put(endereco, passConf);
  endereco += 40;
  EEPROM.put(endereco, tag);
  endereco += 40;
  EEPROM.put(endereco, name);
  endereco += 40;
  EEPROM.put(endereco, lat);
  endereco += 40;
  EEPROM.put(endereco, lon);
  endereco += 40;
  EEPROM.put(endereco, extrainfo);
  endereco += 40;
  EEPROM.commit();

  lerEEPROM();

}

void loop() {
}
