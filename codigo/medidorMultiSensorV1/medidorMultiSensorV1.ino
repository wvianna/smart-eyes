/*

   D0 - Jump para habilitar/desabilitar o sensor de umidade e temperatura DHT11
   D1 - SCL conversor ADS1015
   D2 - SDA conversor ADS1015
   D3 - pino para led vermelho (nível 0 liga led)
   D4 - Dado do sensor DHT11
   D5 - Jump para habilitar/desabilitar o sensor de presença
   D6 - Entrada para sensor de presença. O ajuste de temporização é feito diretamente no sensor.
   D7 - pino para led verde (nível 0 liga led)
   D8 - pino para reset
   D10

   SSID AP padrão "TroqueSSID" e senha "12345678"  (sem aspas)
   login padrão para configurações "admin" e senha "123456" (sem aspas)

   ACESSO AOS DADOS MEDIDOS VIA WEB

    valorCorrente
    http://IPMEDIDOR/data.txt
    valorTemperatura
    http://IPMEDIDOR/temp.txt
    valorUmidade
    http://IPMEDIDOR/umi.txt
    valorPresenca
    http://IPMEDIDOR/pres.txt
    valorPotencia
    http://IPMEDIDOR/potencia.txt
    valorTensao
    http://IPMEDIDOR/tensao.txt
    valorPotDiaria
    http://IPMEDIDOR/pdia.txt
    valorPotMensal
    http://IPMEDIDOR/pmes.txt
    valorPotAnual
    http://IPMEDIDOR/pmes.txt
    valorPotTotal
    http://IPMEDIDOR//ptotal.txt

*/


#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

//Cliente UDP E NTP para aquisição da hora exata
WiFiUDP Udp;
IPAddress timeServer(200, 160, 7, 186); //a.st1.ntp.br
const int timeZone = -3;  // - 3 horas
unsigned int localPort = 123;  // local port to listen for UDP packets

//Cria instância do cliente MQTTl
WiFiClient espClient;
PubSubClient client(espClient);

//Configuração IP do AP
IPAddress ip(192, 168, 4, 1);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(255, 255, 255, 0);

//Criação do servidor Web
ESP8266WebServer server(80);

//Criação da variável do ads 1115
Adafruit_ADS1115 ads;

//Definição do pino utilizado pelo DHT11
DHTesp dht;

// definição dos pinos de jumpeamento
#define pinJumpDHT D0
#define pinJumpPresenca D5

//definição do pino do sensor de presença
#define pinPresenca D6

// definição do pino de reset
#define pinReset D8

// definição dos les de sinalização
#define pinRedRGB D3
#define pinGreenRGB D7

//declarando variáveis
float cont = 0;
float corrente = 0, potenciaDiaria = 0, potenciaMensal = 0, potenciaAnual = 0, potenciaTotal = 0, tensao = 0;
char endBroker[40], portBroker[40], userBroker[40], passBroker[40], ssidwifi[40], passwifi[40], staticIP[40];
char ssidAp[40], passAp[40], userConf[40], passConf[40], subnetWiFi[40], gatewayWiFi[40], dnsWiFi[40];
char potenciaMensalFlash[40], potenciaAnualFlash[40], potenciaTotalFlash[40], potenciaDiariaFlash[40], topicoMQTT[40];
char coeficienteLinearTensao[40], coeficienteAngularTensao[40], coeficienteACorrente[40], coeficienteBCorrente[40], coeficienteCCorrente[40], shuntCorrente[40];
int vetorIP[4] = {0, 0, 0, 0}, vetorGateway[4] = {0, 0, 0, 0}, vetorSubnet[4] = {0, 0, 0, 0}, vetorDNS[4] = {0, 0, 0, 0};
unsigned long tempo2 = 0, lastReconnectAttempt = 0, tempo3 = 0, tempo4 = 0, tempo5 = 0, tempo6 = 0, tempo7 = 0, tempo8 = 0;
unsigned long tempoAnterior2 = 0, tempoAnterior3 = 0, tempoAnterior4 = 0, tempoAnterior5 = 0, tempoAnterior6 = 0;
unsigned long tempoAnterior7 = 0, tempoAnterior8 = 0, tempoAnterior9 = 0, tempoAnterior10 = 0, tempoAnterior11 = 0, tempoAnterior12 = 0;
String valorCorrente = "0.00", valorTemperatura = "0.00", valorUmidade = "0.00", valorPresenca = "OFF", valorPotencia = "0.00", valorTensao = "0.00";
String valorPotDiaria = "0.00", valorPotMensal = "0.00", valorPotAnual = "0.00", valorPotTotal = "0.00";
byte packetBuffer[48]; //buffer to hold incoming & outgoing packets

void lerDHT(void) {

  float umidade = dht.getHumidity();
  float temperatura = dht.getTemperature();

  valorUmidade = String(umidade, 1);
  valorTemperatura = String(temperatura, 1);
  Serial.print("TEMPERATURA: "); Serial.println(temperatura);
  Serial.print("UMIDADE: "); Serial.println(umidade);

}

//html da página com a introdução do AJAX
void handleRoot(void) {

  String page;
  page = "<html><head><title>Medidor de Energia</title>";
  page += "</head><body>";
  page += "<h1><center>PICG - I2S</center></h1>";
  page += "<h2>MEDIDOR DE ENERGIA</h2>";

  page += "<fieldset><legend><b>Informacoes Device IoT</b></legend>";
  page += "<p>FlashChipID: ";
  page += String(ESP.getFlashChipId());
  page += "</p>";
  page += "<p>SdkVersion: ";
  page += String(ESP.getSdkVersion());
  page += "</p>";
  page += "<p>ChipID: ";
  page += String(ESP.getChipId());
  page += "</p>";
  page += "<p>FlashChipSpeed: ";
  page += String(ESP.getFlashChipSpeed());
  page += "<p>IP rede Wifi: ";
  page += String(staticIP);
  page += "<p>Coeficiente linear calibracao tensao: ";
  page += String(coeficienteLinearTensao);
  page += "<p>Coeficiente angular calibracao tensao: ";
  page += String(coeficienteAngularTensao);
  page += "<p>Coeficiente A calibracao corrente polinomial grau 2: ";
  page += String(coeficienteACorrente);
  page += "<p>Coeficiente B calibracao corrente polinomial grau 2: ";
  page += String(coeficienteBCorrente);
  page += "<p>Coeficiente C calibracao corrente polinomial grau 2: ";
  page += String(coeficienteCCorrente);
  page += "<p>Shunt corrente: ";
  page += String(shuntCorrente);
  page += "</p></fieldset>";

  page += "<fieldset><legend><b>Monitoramento: </b></legend>";
  page += "<table>";
  page += "<header><font size=2> Observacao: Para atualizacao dos parametros, deve-se atualizar a pagina. </font></header>";

  page += "<tr><td Width=200>Corrente Eletrica (A): </td><td Align=Middle width=100 id=\"corrente\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Potencia Eletrica (kW): </td><td Align=Middle width=100 id=\"potencia\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Potencia Consumida Diaria (kWh): </td><td Align=Middle width=100 id=\"pdiaria\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Potencia Consumida Mensal (kWh): </td><td Align=Middle width=100 id=\"pmensal\">" "</td></tr>\r\n";
  page += "<tr><td Width=300>Potencia Consumida Anual (kWh): </td><td Align=Middle width=100 id=\"panual\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Potencia Consumida Total(kWh): </td><td Align=Middle width=100 id=\"pototal\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Tensao (V): </td><td Align=Middle width=100  id=\"tensao\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Temperatura (ºC):</td><td Align=Middle width=100 id=\"temperatura\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Umidade (%): </td><td Align=Middle width=100 id=\"umidade\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Presenca: </td><td Align=Middle width=100 id=\"presenca\">" "</td></tr>\r\n";
  page += "<tr><td Width=200>Contador(teste): </td><td Align=Middle width=100>" + String(cont) + "</td></tr>\r\n";
  page += "</fieldset>";

  page += "<script>\r\n";

  page += "var x = setInterval(function() {loadData(\"data.txt\",updateData)},3150);\r\n";
  page += "function loadData(url, callback){\r\n";
  page += "var xhttp = new XMLHttpRequest();\r\n";
  page += "xhttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(xhttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "xhttp.open(\"GET\", url, true);\r\n";
  page += "xhttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData(){\r\n";
  page += "document.getElementById(\"corrente\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var b = setInterval(function() {loadData6(\"potencia.txt\",updateData6)},4500);\r\n";
  page += "function loadData6(url, callback){\r\n";
  page += "var bhttp = new XMLHttpRequest();\r\n";
  page += "bhttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(bhttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "bhttp.open(\"GET\", url, true);\r\n";
  page += "bhttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData6(){\r\n";
  page += "document.getElementById(\"potencia\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var e = setInterval(function() {loadData8(\"pdia.txt\",updateData8)},4600);\r\n";
  page += "function loadData8(url, callback){\r\n";
  page += "var ehttp = new XMLHttpRequest();\r\n";
  page += "ehttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(ehttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "ehttp.open(\"GET\", url, true);\r\n";
  page += "ehttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData8(){\r\n";
  page += "document.getElementById(\"pdiaria\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var f = setInterval(function() {loadData9(\"pmes.txt\",updateData9)},4600);\r\n";
  page += "function loadData9(url, callback){\r\n";
  page += "var fhttp = new XMLHttpRequest();\r\n";
  page += "fhttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(fhttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "fhttp.open(\"GET\", url, true);\r\n";
  page += "fhttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData9(){\r\n";
  page += "document.getElementById(\"pmensal\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var g = setInterval(function() {loadData10(\"pano.txt\",updateData10)},4600);\r\n";
  page += "function loadData10(url, callback){\r\n";
  page += "var ghttp = new XMLHttpRequest();\r\n";
  page += "ghttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(ghttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "ghttp.open(\"GET\", url, true);\r\n";
  page += "ghttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData10(){\r\n";
  page += "document.getElementById(\"panual\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var k = setInterval(function() {loadData11(\"ptotal.txt\",updateData11)},4600);\r\n";
  page += "function loadData11(url, callback){\r\n";
  page += "var khttp = new XMLHttpRequest();\r\n";
  page += "khttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(khttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "khttp.open(\"GET\", url, true);\r\n";
  page += "khttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData11(){\r\n";
  page += "document.getElementById(\"pototal\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var c = setInterval(function() {loadData7(\"tensao.txt\",updateData7)},12500);\r\n";
  page += "function loadData7(url, callback){\r\n";
  page += "var chttp = new XMLHttpRequest();\r\n";
  page += "chttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(chttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "chttp.open(\"GET\", url, true);\r\n";
  page += "chttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData7(){\r\n";
  page += "document.getElementById(\"tensao\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var y = setInterval(function() {loadData1(\"temp.txt\",updateData1)},7900);\r\n";
  page += "function loadData1(url, callback){\r\n";
  page += "var yhttp = new XMLHttpRequest();\r\n";
  page += "yhttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(yhttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "yhttp.open(\"GET\", url, true);\r\n";
  page += "yhttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData1(){\r\n";
  page += "document.getElementById(\"temperatura\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var z = setInterval(function() {loadData2(\"umi.txt\",updateData2)},7900);\r\n";
  page += "function loadData2(url, callback){\r\n";
  page += "var zhttp = new XMLHttpRequest();\r\n";
  page += "zhttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(zhttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "zhttp.open(\"GET\", url, true);\r\n";
  page += "zhttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData2(){\r\n";
  page += "document.getElementById(\"umidade\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "var a = setInterval(function() {loadData3(\"pres.txt\",updateData3)},1050);\r\n";
  page += "function loadData3(url, callback){\r\n";
  page += "var ahttp = new XMLHttpRequest();\r\n";
  page += "ahttp.onreadystatechange = function(){\r\n";
  page += "if(this.readyState == 4 && this.status == 200){\r\n";
  page += "callback.apply(ahttp);\r\n";
  page += "}\r\n";
  page += "};\r\n";
  page += "ahttp.open(\"GET\", url, true);\r\n";
  page += "ahttp.send();\r\n";
  page += "}\r\n";
  page += "function updateData3(){\r\n";
  page += "document.getElementById(\"presenca\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";

  page += "</script>\r\n";

  page += "</tr>";
  page += "</table></td>";
  page += "</fieldset></center>";

  page += "<p><p><fieldset><legend><b>Para acessar a pagina de configuracoes, faca login:</b></legend>";
  page += "<form method='POST' action='/login'>";
  page += "<p>Usuario: ";
  page += "<input type=text size=10  name=login value=admin /></p>";
  page += "<p>Senha: ";
  page += "<input type=password size=10 name=senha /></p>";
  page += "<p><input type=submit name=enviar value=Enviar /></p>";
  page += "</form></fieldset>";
  page += "</p>";

  page += "</body></html>";
  server.send(200, "text/html", page);

}

void formularioBroker(void)
{
  /*
      HTML do formulario de configuracao MQTT (servidor e topicos), após realizar login na pagina anterior
  */
  char userC[30], passC[30];
  String uconf, pconf;

  //recupera usuário e senha digitados na página raiz para fazer a autenticação
  uconf = server.arg("login");
  pconf = server.arg("senha");

  //Converte String em char array
  uconf.toCharArray(userC, 30);
  pconf.toCharArray(passC, 30);

  String html = "<html><head><title>Medidor de Energia</title>";
  html += "</head>";


  //compara usuário e senha com valor da EEPROM
  if ( strcmp(userC, userConf) == 0 && strcmp(passC, passConf) == 0 )
  {
    html += "<h1><center>Configuracoes</center></h1>";

    html += "<fieldset><legend><b>Configuracao Cliente WiFi</b></legend>";
    html += "<form method='POST' action='/configMqtt'>";
    html += "<p> SSID: ";
    html += "<input type=text size=40 name=ssidw value=" + String(ssidwifi) + " /> </p>";
    html += "<p> Senha: ";
    html += "<input type=password size=40 name=pw value=" + String(passwifi) + " /> </p>";
    html += "<p> Static IP: ";
    html += "<input type=text size=40 name=sIP value=" + String(staticIP) + " /> </p>";
    html += "<p> Gateway: ";
    html += "<input type=text size=40 name=gw value=" + String(gatewayWiFi) + " /> </p>";
    html += "<p> Mascara Subnet :";
    html += "<input type=text size=40 name=sn value=" + String(subnetWiFi) + " /> </p>";
    html += "<p> DNS: ";
    html += "<input type=text size=40 name=dns value=" + String(dnsWiFi) + " /> </p></fieldset>";

    html += "<fieldset><legend><b>Configuracao Access Point</b></legend>";
    html += "<p> SSID: ";
    html += "<input type=text size=40 name=ssidAp value=" + String(ssidAp) + " /> </p>";
    html += "<p> Senha: ";
    html += "<input type=password size=40 name=pAp value=" + String(passAp) + " /> </p></fieldset>";

    html += "<fieldset><legend><b>Servidor MQTT</b></legend>";
    html += "<form method='POST' action='/servidorMqtt'>";
    html += "<header><font size=2> Observacao: Se nao houver usuario e senha, deixar em branco. </font></header>";
    html += "<p>IP ou Nome: ";
    html += "<input type=text size=40  name=eBr value=" + String(endBroker) + " /></p> ";
    html += "<p>Porta: ";
    html += "<input type=text size=40  name=poBr value=" + String(portBroker) + " /></p> ";
    html += "<p>Usuario: ";
    html += "<input type=text size=40  name=uBr value=" + String(userBroker) + " /></p> ";
    html += "<p>Senha: ";
    html += "<input type=password size=40  name=pBr value=" + String(passBroker) + " /></p></fieldset>";

    html += "<fieldset><legend><b>Topico MQTT</b></legend>";
    html += "<header><font size=2> Observacao: Criar topicos com no maximo 60 caracteres. </font></header>";
    html += "<p>Topico: ";
    html += "<input type=text size=40 name=tMQTT value=" + String(topicoMQTT) + " /></p></fieldset> ";

    html += "<fieldset><legend><b>Coeficientes de calibracao</b></legend>";
    html += "<header><font size=2> Observacao: utilizar . ponto para separacao decimal. </font></header>";
    html += "<p>Coeficiente linear calibracao tensao: ";
    html += "<input type=text size=40 name=coefLinearTensao value=" + String(coeficienteLinearTensao) + " /></p> ";
    html += "<p>Coeficiente angular calibracao tensao: ";
    html += "<input type=text size=40 name=coefAngularTensao value=" + String(coeficienteAngularTensao) + " /></p> ";
    html += "<p>Coeficiente A calibracao corrente polinomial grau 2: ";
    html += "<input type=text size=40 name=coefACorrente value=" + String(coeficienteACorrente) + " /></p> ";
    html += "<p>Coeficiente B calibracao corrente polinomial grau 2: ";
    html += "<input type=text size=40 name=coefBCorrente value=" + String(coeficienteBCorrente) + " /></p> ";
    html += "<p>Coeficiente C calibracao corrente polinomial grau 2: ";
    html += "<input type=text size=40 name=coefCCorrente value=" + String(coeficienteCCorrente) + " /></p> ";
    html += "<p>Shunt corrente: ";
    html += "<input type=text size=40 name=shuntCorrente value=" + String(shuntCorrente) + " /></p></fieldset> ";
    html += "<p><p><center><input type=submit name=botao1 value=Salvar /></center>";
    html += "</form>";
    html += "<p><center><a href=/>Pagina Inicial</a></center></p>";
  }
  else
  {
    html += "<h1><center>Falha na autenticacao</center></h1>";
    html += "Usuario ou senha invalidos!";
    html += "<p><center><a href=javascript:window.history.go(-1)><input type=submit name=botao value=Voltar /></a></center></p>";
  }
  html += "</body></html>";

  // Enviando HTML para o servidor
  server.send(200, "text/html", html);
}

void formularioEnviadoMqtt(void)
{
  /*
     HTML APÓS CONFIGURAÇÕES FEITAS
  */
  //cabeçalho html
  String html = "<html><head><title>Medidor de Energia</title>";
  html += "</head>";
  html += "<body>";
  html += "<h1><center>Formulario de Confirmacao das Configuracoes</center></h1>";

  /*
     WiFi
  */

  html += "<p>";
  if (server.hasArg("ssidw"))
  {
    html += "SSID (Cliente WiFi): ";
    html += server.arg("ssidw");
    server.arg("ssidw").toCharArray(ssidwifi, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("pw"))
  {
    html += "Senha (Cliente WiFi): ";
    html += server.arg("pw");
    server.arg("pw").toCharArray(passwifi, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("sIP"))
  {
    html += "Static IP (Cliente WiFi): ";
    html += server.arg("sIP");
    server.arg("sIP").toCharArray(staticIP, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("gw"))
  {
    html += "Gateway (Cliente WiFi): ";
    html += server.arg("gw");
    server.arg("gw").toCharArray(gatewayWiFi, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("sn"))
  {
    html += "Mascara Subnet (Cliente WiFi): ";
    html += server.arg("sn");
    server.arg("sn").toCharArray(subnetWiFi, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("dns"))
  {
    html += "DNS (Cliente WiFi): ";
    html += server.arg("dns");
    server.arg("dns").toCharArray(dnsWiFi, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  /*ler
     Access Point
  */

  html += "<p>";
  if (server.hasArg("ssidAp"))
  {
    html += "SSID (Access Point): ";
    html += server.arg("ssidAp");
    server.arg("ssidAp").toCharArray(ssidAp, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("pAp"))
  {
    html += "Senha (Access Point): ";
    html += server.arg("pAp");
    server.arg("pAp").toCharArray(passAp, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  /*
     Servidor
  */

  html += "<p>";
  if (server.hasArg("eBr"))
  {
    html += "IP ou Nome: ";
    html += server.arg("eBr");
    server.arg("eBr").toCharArray(endBroker, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("poBr"))
  {
    html += "Porta: ";
    html += server.arg("poBr");
    server.arg("poBr").toCharArray(portBroker, 40);
  }
  else
  {
    html += "<b>-----</b>";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("uBr"))
  {
    html += "Usuario: ";
    html += server.arg("uBr");
    server.arg("uBr").toCharArray(userBroker, 40);

  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("pBr"))
  {
    html += "Senha: ";
    html += server.arg("pBr");

    server.arg("pBr").toCharArray(passBroker, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  /*
     Topicos MQTT
  */

  html += "<p>";
  if (server.hasArg("tMQTT"))
  {
    html += "Topico MQTT: ";
    html += server.arg("tMQTT");
    server.arg("tMQTT").toCharArray(topicoMQTT, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  /*
       Coeficientes e constantes
  */

  html += "<p>";
  if (server.hasArg("coefLinearTensao"))
  {
    html += "Coeficiente linear tensao: ";
    html += server.arg("coefLinearTensao");
    server.arg("coefLinearTensao").toCharArray(coeficienteLinearTensao, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("coefAngularTensao"))
  {
    html += "Coeficiente angular tensao: ";
    html += server.arg("coefAngularTensao");
    server.arg("coefAngularTensao").toCharArray(coeficienteAngularTensao, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("coefACorrente"))
  {
    html += "Coeficiente A corrente polinomial grau 2: ";
    html += server.arg("coefACorrente");
    server.arg("coefACorrente").toCharArray(coeficienteACorrente, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("coefBCorrente"))
  {
    html += "Coeficiente B corrente polinomial grau 2: ";
    html += server.arg("coefBCorrente");
    server.arg("coefBCorrente").toCharArray(coeficienteBCorrente, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("coefCCorrente"))
  {
    html += "Coeficiente C corrente polinomial grau 2: ";
    html += server.arg("coefCCorrente");
    server.arg("coefCCorrente").toCharArray(coeficienteCCorrente, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";

  html += "<p>";
  if (server.hasArg("shuntCorrente"))
  {
    html += "Shunt corrente: ";
    html += server.arg("shuntCorrente");
    server.arg("shuntCorrente").toCharArray(shuntCorrente, 40);
  }
  else
  {
    html += "-----";
  }
  html += "</p>";


  html += "<p><center><a href=javascript:window.history.go(-1)><input type=submit name=botao value=Voltar /></a></center></p>";
  html += "</form>";
  html += "<form method='POST' action='/reset'>";
  html += "<p><center><input name=button3 type=submit value=Reset_WiFi_AccessPoint /></center></p>";
  html += "</form>";
  html += "<p><center><a href=/>Pagina Inicial</a></center></p>";
  html += "</body></html>";

  // Enviando HTML para o servidor
  server.send(200, "text/html", html);
  valorPotDiaria.toCharArray(potenciaDiariaFlash, 40);
  valorPotMensal.toCharArray(potenciaMensalFlash, 40);
  valorPotAnual.toCharArray(potenciaAnualFlash, 40);
  valorPotTotal.toCharArray(potenciaTotalFlash, 40);

  gravarEEPROM();

  lerReset();
  setup();
}

void reiniciarSenhas(void) {
  /*
     função para reiniciar: ssid, senha AP, user conf. e senha conf. através do botão virtual do browser.
  */
  String dado;
  dado = "TroqueSSID";
  dado.toCharArray(ssidAp, 40);
  dado = "12345678";
  dado.toCharArray(passAp, 40);
  dado = "admin";
  dado.toCharArray(userConf, 40);
  dado = "123456";
  dado.toCharArray(passConf, 40);

  gravarEEPROM();

  String html = "<html><head><title>Medidor de Energia</title>";
  html += "</head>";
  html += "<p><center>Configuracoes Restauradas</center></p>";
  html += "<p><center>Acesse a Pagina Inicial</center></p>";
  html += "<p><center><a href=/>Pagina Inicial</a></center></p>";
  html += "</html>";

  // Enviando HTML para o servidor
  server.send(200, "text/html", html);

}

void lerEEPROM() {
  unsigned int endereco;
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
  EEPROM.get(endereco, potenciaDiariaFlash);
  endereco += 40;
  EEPROM.get(endereco, potenciaMensalFlash);
  endereco += 40;
  EEPROM.get(endereco, potenciaAnualFlash);
  endereco += 40;
  EEPROM.get(endereco, potenciaTotalFlash);
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
  EEPROM.get(endereco, coeficienteLinearTensao);
  endereco += 40;
  EEPROM.get(endereco, coeficienteAngularTensao);
  endereco += 40;
  EEPROM.get(endereco, coeficienteACorrente);
  endereco += 40;
  EEPROM.get(endereco, coeficienteBCorrente);
  endereco += 40;
  EEPROM.get(endereco, coeficienteCCorrente);
  endereco += 40;
  EEPROM.get(endereco, shuntCorrente);
  endereco += 40;
  endereco = 0;
  char dado[40];
  for ( byte i = 0; i < 19; i++)
  {
    EEPROM.get(endereco, dado);
    Serial.println(dado);
    endereco += 40;
  }
}


void gravarEEPROM() {
  unsigned int endereco;
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
  EEPROM.put(endereco, potenciaDiariaFlash);
  endereco += 40;
  EEPROM.put(endereco, potenciaMensalFlash);
  endereco += 40;
  EEPROM.put(endereco, potenciaAnualFlash);
  endereco += 40;
  EEPROM.put(endereco, potenciaTotalFlash);
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
  EEPROM.put(endereco, coeficienteLinearTensao);
  endereco += 40;
  EEPROM.put(endereco, coeficienteAngularTensao);
  endereco += 40;
  EEPROM.put(endereco, coeficienteACorrente);
  endereco += 40;
  EEPROM.put(endereco, coeficienteBCorrente);
  endereco += 40;
  EEPROM.put(endereco, coeficienteCCorrente);
  endereco += 40;
  EEPROM.put(endereco, shuntCorrente);
  endereco += 40;
  EEPROM.commit();
  endereco = 0;
  char dado[40];
  for ( byte i = 0; i < 19; i++)
  {
    EEPROM.get(endereco, dado);
    Serial.println(dado);
    endereco += 40;
  }

}

void conectarWifi(void) {
  /*
     Função para conectar Wifi
     Tenta conexão durante 6 segundos
  */
  unsigned long tempo1 = 0;
  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration
  splitIP();
  splitGateway();
  splitDNS();
  splitSubnet();
  IPAddress staticIp(vetorIP[0], vetorIP[1], vetorIP[2], vetorIP[3]);
  IPAddress gateway1(vetorGateway[0], vetorGateway[1], vetorGateway[2], vetorGateway[3]);
  IPAddress subnet1(vetorSubnet[0], vetorSubnet[1], vetorSubnet[2], vetorSubnet[3]);
  IPAddress dns1(vetorDNS[0], vetorDNS[1], vetorDNS[2], vetorDNS[3]);
  WiFi.begin(ssidwifi, passwifi);
  WiFi.config(staticIp, gateway1, subnet1, dns1);
  WiFi.mode(WIFI_AP_STA);

  // configurando o Access Point
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssidAp, passAp);

  Serial.println("");
  tempo1 = millis();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Wifi disconnected");
  }

  // Mostrando IP se conectado
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

}

void splitIP (void) {

  //função para fazer a transcrição do vetor char do IP estático para inteiro

  char * sIP;
  int auxiliar = 0;
  char auxiliarIP[40];
  strcpy(auxiliarIP, staticIP)  ;
  sIP = strtok(auxiliarIP, ".");
  while (sIP != NULL)
  {
    vetorIP[auxiliar] = atoi(sIP);
    sIP = strtok(NULL, ".");
    auxiliar++;
  }

}

void splitGateway(void) {

  char * sGW;
  int auxiliar = 0;
  char auxiliarGateway[40];
  strcpy(auxiliarGateway, gatewayWiFi);
  sGW = strtok(auxiliarGateway, ".");
  while (sGW != NULL)
  {
    vetorGateway[auxiliar] = atoi(sGW);
    sGW = strtok(NULL, ".");
    auxiliar++;
  }

}

void splitSubnet(void) {

  char * sSN;
  int auxiliar = 0;
  char auxiliarSubnet[40];
  strcpy(auxiliarSubnet, subnetWiFi);
  sSN = strtok(auxiliarSubnet, ".");
  while (sSN != NULL)
  {
    vetorSubnet[auxiliar] = atoi(sSN);
    sSN = strtok(NULL, ".");
    auxiliar++;
  }

}

void splitDNS(void) {

  char * sDNS;
  int auxiliar = 0;
  char auxiliarDNS[40];
  strcpy(auxiliarDNS, dnsWiFi);
  sDNS = strtok(auxiliarDNS, ".");
  while (sDNS != NULL)
  {
    vetorDNS[auxiliar] = atoi(sDNS);
    sDNS = strtok(NULL, ".");
    auxiliar++;
  }
}

void lerTensao() {
  /*
    Cálculo da tensão a partir do secundário do transformador, o qual também alimenta o sistema
  */
  int16_t adsValor = 0;
  int vTensao = 0;
  int  cont = 0 ;
  uint32_t start_time = millis();

  while ((millis() - start_time) < 34) //sample for 1 Sec 34 para ler durante dois periodos do sinal da rede
  {
    adsValor = ads.readADC_Differential_0_1();
    //shift para direita depois para esquerda para zerar os bits menos significativos
    adsValor = adsValor >> 5;
    adsValor = adsValor << 5;
    vTensao = vTensao + adsValor;
    cont = cont + 1;
  }
  //faz a média do sinal lido
  vTensao =  vTensao / cont;
  //Serial.print("vTensao = "); Serial.println(vTensao);

  if (( abs(tensao - ((strtod(coeficienteLinearTensao, NULL) + strtod(coeficienteAngularTensao, NULL) * vTensao))) > (tensao * 0.01)) || ( abs(tensao - ((strtod(coeficienteLinearTensao, NULL) + strtod(coeficienteAngularTensao, NULL) * vTensao))) < (tensao * 0.01))) {
    tensao =  strtod(coeficienteLinearTensao, NULL) + strtod(coeficienteAngularTensao, NULL) * vTensao;
    //Serial.print("Tensao = "); Serial.println(tensao);
    //Serial.print("CoefLinearTensao = "); Serial.println(strtod(coeficienteLinearTensao, NULL));
    //Serial.print("CoefAnglarTensao = "); Serial.println(strtod(coeficienteAngularTensao, NULL));
  }

  valorTensao = String(tensao, 1);

}


void lerCorrente(void) {
  /*
    Cálculo da corrente do primário
    Sendo:
    100A -> 50 mA dado do sensor
    Valor do resistor = 160 ohms
  */

  int sensorRead;             //valor lido no sensor
  int maxValue = 0;           //valor máximo
  int minValue = 1023;        //valor mínimo
  int deltaValue = 0;
  float correnteAtual;

  uint32_t start_time = millis();

  while ((millis() - start_time) < 34) {// 34 mS para ler durante dois periodos do sinal da rede

    sensorRead = analogRead(A0);

    if (sensorRead > maxValue)
      maxValue = sensorRead;
    if (sensorRead < minValue)
      minValue = sensorRead;

  }
  Serial.print("maxValue : "); Serial.println(maxValue);
  Serial.print("minValue : "); Serial.println(minValue);
  Serial.print("deltaValue : "); Serial.println(maxValue - minValue);
  Serial.print("A : "); Serial.println(coeficienteACorrente);
  Serial.print("B : "); Serial.println(coeficienteBCorrente);
  Serial.print("C : "); Serial.println(coeficienteCCorrente);
  deltaValue = maxValue - minValue;
  //para eliminar ruído
  if (deltaValue <= 10) {
    deltaValue = 0;
  }
  //correnteAtual = strtod(coeficienteCCorrente, NULL) + (strtod(coeficienteBCorrente, NULL)) * ((((((deltaValue) * 3.3) / 1023.0) * sqrt(2)) / 2 ) / (strtod(shuntCorrente, NULL) * 100 / 0.05)) + (strtod(coeficienteACorrente, NULL) * strtod(coeficienteACorrente, NULL) * (((((deltaValue) * 3.3) / 1023.0) * sqrt(2)) / 2 ) / (strtod(shuntCorrente, NULL) * 100 / 0.05));
  //calcula a corrente com base nos parametros do AD e resitor shunt
  correnteAtual = abs(((((((maxValue - minValue) * 3.3) / 1023.0) * sqrt(2)) / 2 ) / 160.0) * 100 / 0.05);
  Serial.print("correnteAtualCrua : "); Serial.println(correnteAtual);
  //corrige a corrente devido a nao linearidade do sensor
  correnteAtual = strtod(coeficienteACorrente, NULL) * strtod(coeficienteACorrente, NULL) * correnteAtual + strtod(coeficienteBCorrente, NULL) * correnteAtual + strtod(coeficienteCCorrente, NULL);
  Serial.print("correnteAtualCorrigida : "); Serial.println(correnteAtual);
  if (correnteAtual > (corrente * 1.01)) {
    corrente = correnteAtual;
  }
  //utiliza funçao lienar abaixo do coeficiente C
  Serial.print("correnteAtual : "); Serial.println(correnteAtual);
  if (correnteAtual <= strtod(coeficienteCCorrente, NULL)) {
    corrente = correnteAtual * 0.8;
  }
  //zera o valor da corrente devido a nao linearidade e erro do inicio da faixa 
   if (correnteAtual <= 0.3) {
    corrente = 0;
  }
  valorCorrente = String(corrente, 1);

}

void lerMovimento(void)
{
  int presenca;
  presenca = digitalRead(pinPresenca);
  if (presenca == HIGH)
  {
    valorPresenca = "ON";
  }
  else if (presenca == LOW)
  {
    valorPresenca = "OFF";
  }
}

void publicarTopicos(void) {

  /*
     Coloque aqui o código para publicar os tópicos
  */

  char msgMQTT[300];

  if (client.connected())
  {
    StaticJsonDocument<500> doc;
    doc["Pot_Inst"] = valorPotencia;
    doc["Pot_Dia"] = valorPotDiaria;
    doc["Pot_Mes"] = valorPotMensal;
    doc["Pot_Ano"] = valorPotAnual;
    doc["Pot_Total"] = valorPotTotal;
    doc["Corrente"] = valorCorrente;
    doc["Tensao"] = valorTensao;
    doc["Temperatura"] = valorTemperatura;
    doc["Umidade"] = valorUmidade;
    doc["Presenca"] = valorPresenca;
    doc["Topico"] = topicoMQTT;
    serializeJson(doc, msgMQTT);
    client.publish(topicoMQTT, msgMQTT);
  }
}

boolean reconnect()
{
  String chipID = String(ESP.getChipId());
  char clientESP[40];
  chipID.toCharArray(clientESP, 40);
  //conecta no broker com identificação chipID
  if (client.connect(clientESP, userBroker, passBroker)) {
    char primeraPublicacao[] = "===================> PRIMEIRA PUBLICACAO =====================> ";
    strcat(primeraPublicacao, topicoMQTT);
    client.publish("inTopic", primeraPublicacao);
    // ... and resubscribe
    client.subscribe("inTopic");
    Serial.println("conectado ao broker mqtt");
  }
  else {
    Serial.println("não conectado");
  }
  return client.connected();
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void lerReset()
{
  /*
    função para o reset manual do sistema
  */
  int valorReset;
  valorReset = digitalRead(pinReset);
  digitalWrite(pinReset, LOW);

}


time_t getNtpTime()
{

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= 48) {
      Udp.read(packetBuffer, 48);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }

  return 0; // return 0 if unable to get the time

}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, 48);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:l
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, 48);
  Udp.endPacket();

}

void lerPotencia(void) {

  float potencia = 0;
  potencia = (corrente * tensao) / 1000; //em kW
  valorPotencia = String(potencia, 2);
  potenciaDiaria = potenciaDiaria + (potencia / 1800); // valor dividido por 1800 devido ao intervalor de 2segundos
  valorPotDiaria = String(potenciaDiaria, 2);
  potenciaMensal = potenciaMensal + (potencia / 1800);
  valorPotMensal = String(potenciaMensal, 2);
  potenciaAnual = potenciaAnual + (potencia / 1800);
  valorPotAnual = String(potenciaAnual, 2);
  potenciaTotal = potenciaTotal + (potencia / 1800);
  valorPotTotal = String(potenciaTotal, 2);

  //zerando o contador de kwh consumido mensalmente
  if (month() == 1 || month() == 3 || month() == 5 || month() == 7 || month() == 8 || month() == 10 || month() == 12)
  {
    if (day() == 31 && hour() == 23  && minute() == 59 && second() == 59)
    {
      potenciaMensal = 0;
    }
  }
  else if (month() == 4 || month() == 6 || month() == 9 || month() == 11)
  {
    if (day() == 30 && hour() == 23  && minute() == 59 && second() == 59)
    {
      potenciaMensal = 0;
    }
  }
  else if (month() == 2)
  {
    if (year() % 4 != 0)
    {
      if (day() == 28 && hour() == 23  && minute() == 59 && second() == 59)
      {
        potenciaMensal = 0;
      }
    }
    else
    {
      if (day() == 29 && hour() == 23  && minute() == 59 && second() == 59)
      {
        potenciaMensal = 0;
      }
    }
  }

  //zerando o contador de kwh consumido anualmente
  if (day() == 31 && month() == 12  && hour() == 23  && minute() == 59 && second() == 59)
  {
    potenciaAnual = 0;
  }

  //zerando o contador de kwh consumido diariamente
  if ((hour() == 23  && minute() == 59 && second() == 59) || (hour() == 12  && minute() == 59 && second() == 59) ) {
    potenciaDiaria = 0;
    valorPotDiaria = String(potenciaDiaria, 2);
    //colocando os valores atualizados das potencias na memória flash ao fim do dia
    valorPotDiaria.toCharArray(potenciaDiariaFlash, 40);
    valorPotMensal.toCharArray(potenciaMensalFlash, 40);
    valorPotAnual.toCharArray(potenciaAnualFlash, 40);
    valorPotTotal.toCharArray(potenciaTotalFlash, 40);
    gravarEEPROM();
    lerReset();
  }

}

void setup(void) {

  Serial.begin(115200);
  EEPROM.begin(2048);
  ESP.wdtEnable(10000);

  //troca velocidade do i2c para 400kHz no esp8266
  Wire.setClock(400000L);

  lerEEPROM();

  //carregando os valores salvos na EEPROM para as potencias consumidas
  valorPotDiaria = String(potenciaDiariaFlash);
  potenciaDiaria = valorPotDiaria.toFloat();
  valorPotMensal = String(potenciaMensalFlash);
  potenciaMensal = valorPotMensal.toFloat();
  valorPotAnual = String(potenciaAnualFlash);
  potenciaAnual = valorPotAnual.toFloat();
  valorPotTotal = String(potenciaTotalFlash);
  potenciaTotal = valorPotTotal.toFloat();

  // conectando wifi
  conectarWifi();

  // inicializando a leitura do sensor DHT 11
  dht.setup(2, DHTesp::DHT11);

  // inicialização da leitura do ads 1115
  ads.setGain(GAIN_ONE);
  ads.begin();

  //Setando o pino do sensor de presença como entrada
  pinMode(pinPresenca, INPUT);

  //setando os pinos de jump como entrada
  pinMode(pinJumpDHT, INPUT);
  pinMode(pinJumpPresenca, INPUT);
  pinMode(pinReset, OUTPUT);
  digitalWrite(pinReset, HIGH);

  //setando os pinos do rgb como saída
  pinMode(pinGreenRGB, OUTPUT);
  pinMode(pinRedRGB, OUTPUT);

  //configurando MQTT
  client.setServer(endBroker, atoi(portBroker));
  client.setCallback(callback);

  //Inicialização do NTP
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);

  //publica no servidor WEB os valores medidos
  server.on("/data.txt", []() {
    server.send(200, "text/html", valorCorrente);
  });
  server.on("/temp.txt", []() {
    server.send(200, "text/html", valorTemperatura);
  });
  server.on("/umi.txt", []() {
    server.send(200, "text/html", valorUmidade);
  });
  server.on("/pres.txt", []() {
    server.send(200, "text/html", valorPresenca);
  });
  server.on("/potencia.txt", []() {
    server.send(200, "text/html", valorPotencia);
  });
  server.on("/tensao.txt", []() {
    server.send(200, "text/html", valorTensao);
  });

  // publica no servidor WEV as potências kwh diária, mensal, anual e total
  server.on("/pdia.txt", []() {
    server.send(200, "text/html", valorPotDiaria);
  });
  server.on("/pmes.txt", []() {
    server.send(200, "text/html", valorPotMensal);
  });
  server.on("/pano.txt", []() {
    server.send(200, "text/html", valorPotAnual);
  });
  server.on("/ptotal.txt", []() {
    server.send(200, "text/html", valorPotTotal);
  });

  //direciona o endereço do servidor ao código html
  server.on("/", handleRoot);

  server.on("/login", HTTP_POST, formularioBroker);

  server.on("/configMqtt", HTTP_POST, formularioEnviadoMqtt);

  server.on("/servidorMqtt", HTTP_POST, formularioEnviadoMqtt);

  server.on("/reset", HTTP_POST, reiniciarSenhas);

  server.begin();

  //tempos usados como loop
  tempo2 = millis();
  tempo3 = millis();
  tempo4 = millis();
  tempo5 = millis();
  tempo6 = millis();
  tempo7 = millis();
  tempo8 = millis();

  lastReconnectAttempt = 0;
}

void loop(void) {

  tempo2 = millis();
  tempo3 = millis();
  tempo4 = millis();
  tempo5 = millis();
  tempo6 = millis();
  tempo7 = millis();
  tempo8 = millis();


  //lógica do LED quando sistema está conectado no Wifi e broker
  if (client.connected() && WiFi.status() == WL_CONNECTED)
  {
    if (tempo5 - tempoAnterior5 > 5000)
    {
      tempoAnterior5 = millis();
      tempoAnterior11 = millis();
      digitalWrite(pinGreenRGB, LOW); //nível 0 liga o led
      digitalWrite(pinRedRGB, HIGH); //nível 1 desliga
      Serial.println("piscando LED");
    }
    else if (tempo5 - tempoAnterior11 > 500)
    {
      digitalWrite(pinGreenRGB, HIGH);
      digitalWrite(pinRedRGB, HIGH);
    }
  }


  // lógica do LED quando o broker não está conectado
  if (!client.connected())
  {
    long now = millis();
    if (WiFi.status() == WL_CONNECTED)
    {
      if (tempo6 - tempoAnterior6 > 500)
      {
        tempoAnterior6 = tempo6;
        digitalWrite(pinRedRGB, not(digitalRead(pinRedRGB)));
        digitalWrite(pinGreenRGB, HIGH);
      }
    }
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    // Client connected
    client.loop();
  }


  //lógica do LEG quando o wifi não está conectado
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(pinRedRGB, LOW);
    digitalWrite(pinGreenRGB, HIGH);

    if (tempo7 - tempoAnterior8 > 5000)
    {
      tempoAnterior8 = tempo7;
      conectarWifi();
    }
  }

  server.handleClient();

  //loop das funções
  if (tempo3 - tempoAnterior3 > 5000)
  {
    tempoAnterior3 = tempo3;
    lerTensao();
    if (digitalRead(pinJumpDHT) == HIGH)
    {
      lerDHT();
    }
    else {
      valorUmidade = "NAO CONECTADO";
      valorTemperatura = "NAO CONECTADO";
    }
    cont = cont + 0.001388889;
  }


  if (tempo2 - tempoAnterior2 > 2000)
  {
    tempoAnterior2 = tempo2;
    lerCorrente();
    lerPotencia();
    publicarTopicos();
  }

  if (tempo4 - tempoAnterior4 > 60000)
  {
    tempoAnterior4 = tempo4;
    setSyncProvider(getNtpTime);
  }

  if (tempo8 - tempoAnterior12 > 500)
  {
    tempoAnterior12 = tempo8;
    if (digitalRead(pinJumpPresenca) == HIGH)
    {
      lerMovimento();
    }
    else {
      valorPresenca = "NAO CONECTADO";
    }
  }


  //feed do watchdog timer
  ESP.wdtFeed();
}
