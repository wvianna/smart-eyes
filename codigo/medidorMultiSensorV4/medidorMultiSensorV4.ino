/*

   D0 -
   D1 - SCL conversor ADS1015
   D2 - SDA conversor ADS1015
   D3 - pino para led vermelho (nível 0 liga led)
   D4 -
   D5 -
   D6 -
   D7 - pino para led verde (nível 0 liga led)
   D8 - pino para reset
   D10

   SSID AP padrão "TroqueSSID" e senha "12345678"  (sem aspas)
   login padrão para configurações "admin" e senha "123456" (sem aspas)

   ACESSO AOS DADOS MEDIDOS VIA WEB

    valorNível
    http://IPMEDIDOR/data.json

*/


#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

//Cria instância do cliente MQTT
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


// definição do pino de reset
#define pinReset D8

// definição dos les de sinalização
#define pinRedRGB D3
#define pinGreenRGB D7

// definição dos pinos para sensor de distância
#define pinTrig D6
#define pinEcho D5

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701



//declarando variáveis
float cont = 0;
float nivel = 0, pacote = 0;//, potenciaDiaria = 0, potenciaMensal = 0, potenciaAnual = 0, potenciaTotal = 0, tensao = 0;
char endBroker[40], portBroker[40], userBroker[40], passBroker[40], ssidwifi[40], passwifi[40], staticIP[40];
char ssidAp[40], passAp[40], userConf[40], passConf[40], subnetWiFi[40], gatewayWiFi[40], dnsWiFi[40];
//char potenciaMensalFlash[40], potenciaAnualFlash[40], potenciaTotalFlash[40], potenciaDiariaFlash[40],
char topicoMQTT[40];
//char coeficienteLinearTensao[40], coeficienteAngularTensao[40], coeficienteACorrente[40], coeficienteBCorrente[40], coeficienteCCorrente[40], shuntCorrente[40];
int vetorIP[4] = {0, 0, 0, 0}, vetorGateway[4] = {0, 0, 0, 0}, vetorSubnet[4] = {0, 0, 0, 0}, vetorDNS[4] = {0, 0, 0, 0};
unsigned long tempo2 = 0, lastReconnectAttempt = 0, tempo3 = 0, tempo4 = 0, tempo5 = 0, tempo6 = 0, tempo7 = 0, tempo8 = 0;
unsigned long tempoAnterior2 = 0, tempoAnterior3 = 0, tempoAnterior4 = 0, tempoAnterior5 = 0, tempoAnterior6 = 0;
unsigned long tempoAnterior7 = 0, tempoAnterior8 = 0, tempoAnterior9 = 0, tempoAnterior10 = 0, tempoAnterior11 = 0, tempoAnterior12 = 0;
//unsigned long pacote = 0;
String valorNivel = "0.00", valorPacote = "0";
//, valorTemperatura = "0.00", valorUmidade = "0.00", valorPresenca = "OFF", valorPotencia = "0.00", valorTensao = "0.00";
//String valorPotDiaria = "0.00", valorPotMensal = "0.00", valorPotAnual = "0.00", valorPotTotal = "0.00";
byte packetBuffer[48]; //buffer to hold incoming & outgoing packets
char msgMQTT[300];

long duration;
float distanceCm;
float distanceInch;


//html da página com a introdução do AJAX
void handleRoot(void) {

  String page;
  page = "<html><head><title>Medidor Multisensor</title>";
  page += "</head><body>";
  page += "<h1><center>ROVEQ</center></h1>";
  page += "<h2>MEDIDOR MULTISENSOR</h2>";

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
  //page += "<p>Coeficiente linear calibracao tensao: ";
  //page += String(coeficienteLinearTensao);
  //page += "<p>Coeficiente angular calibracao tensao: ";
  //page += String(coeficienteAngularTensao);
  //page += "<p>Coeficiente A calibracao corrente polinomial grau 2: ";
  //page += String(coeficienteACorrente);
  //page += "<p>Coeficiente B calibracao corrente polinomial grau 2: ";
  //page += String(coeficienteBCorrente);
  //page += "<p>Coeficiente C calibracao corrente polinomial grau 2: ";
  //page += String(coeficienteCCorrente);
  //page += "<p>Shunt corrente: ";
  //page += String(shuntCorrente);
  page += "</p></fieldset>";

  page += "<fieldset><legend><b>Json: </b></legend>";
  page += "<table>";
  //page += "<header><font size=2> Observacao: Para atualizacao dos parametros, deve-se atualizar a pagina. </font></header>";

  //page += "<tr><td Width=200>JSON: </td><td Align=Middle width=100 id=\"json\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Pacote (unid): </td><td Align=Middle width=100 id=\"pacote\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Potencia Consumida Diaria (kWh): </td><td Align=Middle width=100 id=\"pdiaria\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Potencia Consumida Mensal (kWh): </td><td Align=Middle width=100 id=\"pmensal\">" "</td></tr>\r\n";
  //page += "<tr><td Width=300>Potencia Consumida Anual (kWh): </td><td Align=Middle width=100 id=\"panual\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Potencia Consumida Total(kWh): </td><td Align=Middle width=100 id=\"pototal\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Tensao (V): </td><td Align=Middle width=100  id=\"tensao\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Temperatura (ºC):</td><td Align=Middle width=100 id=\"temperatura\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Umidade (%): </td><td Align=Middle width=100 id=\"umidade\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Presenca: </td><td Align=Middle width=100 id=\"presenca\">" "</td></tr>\r\n";
  //page += "<tr><td Width=200>Contador(teste): </td><td Align=Middle width=100>" + String(cont) + "</td></tr>\r\n";
  page += "</fieldset>";

  page += "<script>\r\n";

  page += "var x = setInterval(function() {loadData(\"data.json\",updateData)},3150);\r\n";
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
  page += "document.getElementById(\"json\").innerHTML = this.responseText;\r\n";
  page += "}\r\n";
  /*
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
  */
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
    /*html += "<input type=text size=40 name=coefLinearTensao value=" + String(coeficienteLinearTensao) + " /></p> ";
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
    */
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
  /*
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
  */

  html += "<p><center><a href=javascript:window.history.go(-1)><input type=submit name=botao value=Voltar /></a></center></p>";
  html += "</form>";
  html += "<form method='POST' action='/reset'>";
  html += "<p><center><input name=button3 type=submit value=Reset_WiFi_AccessPoint /></center></p>";
  html += "</form>";
  html += "<p><center><a href=/>Pagina Inicial</a></center></p>";
  html += "</body></html>";

  // Enviando HTML para o servidor
  server.send(200, "text/html", html);
  //valorPotDiaria.toCharArray(potenciaDiariaFlash, 40);
  //valorPotMensal.toCharArray(potenciaMensalFlash, 40);
  //valorPotAnual.toCharArray(potenciaAnualFlash, 40);
  //valorPotTotal.toCharArray(potenciaTotalFlash, 40);

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
  /*EEPROM.get(endereco, potenciaDiariaFlash);
    endereco += 40;
    EEPROM.get(endereco, potenciaMensalFlash);
    endereco += 40;
    EEPROM.get(endereco, potenciaAnualFlash);
    endereco += 40;
    EEPROM.get(endereco, potenciaTotalFlash);
    endereco += 40;*/
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
  /*endereco += 40;
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
    endereco += 40;*/
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
  /*  EEPROM.put(endereco, potenciaDiariaFlash);
    endereco += 40;
    EEPROM.put(endereco, potenciaMensalFlash);
    endereco += 40;
    EEPROM.put(endereco, potenciaAnualFlash);
    endereco += 40;
    EEPROM.put(endereco, potenciaTotalFlash);
    endereco += 40;*/
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
  /*  endereco += 40;
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
    endereco += 40;*/
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

/*
  void lerTensao() {
  /*
    Cálculo da tensão a partir do secundário do transformador, o qual também alimenta o sistema
*/
/*
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
/*
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
*/
/*
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
*/
void publicarTopicos(void) {

  /*
     Coloque aqui o código para publicar os tópicos
  */

  if (client.connected())
  {
    StaticJsonDocument<500> doc;
    doc["tag"] = "LT100";
    doc["name"] = "Paraiba do Sul";
    doc["lat"] = "-21.62018027266234";
    doc["lon"] = "-41.031522464095616";
    doc["extrainfo"] = "sensor Paraiba do Sul - Foz";
    doc["nivel"] = valorNivel;
    doc["pacote"] = valorPacote;
    //doc["Temperatura"] = valorTemperatura;
    //doc["Umidade"] = valorUmidade;
    //doc["Presenca"] = valorPresenca;
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

void lerNivel(void) {
  float nivel = 0;
  nivel = random(0, 10);
  valorNivel = String(nivel, 2);
  pacote++;
  valorPacote = String(pacote, 0);

}
/*
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
*/

void lerDistancia(void) {
  // Clears the trigPin
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(pinEcho, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;

  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
}
void setup(void) {

  Serial.begin(115200);
  EEPROM.begin(2048);
  ESP.wdtEnable(10000);

  //troca velocidade do i2c para 400kHz no esp8266
  Wire.setClock(400000L);

  lerEEPROM();

  //carregando os valores salvos na EEPROM para as potencias consumidas
  /*valorPotDiaria = String(potenciaDiariaFlash);
    potenciaDiaria = valorPotDiaria.toFloat();
    valorPotMensal = String(potenciaMensalFlash);
    potenciaMensal = valorPotMensal.toFloat();
    valorPotAnual = String(potenciaAnualFlash);
    potenciaAnual = valorPotAnual.toFloat();
    valorPotTotal = String(potenciaTotalFlash);
    potenciaTotal = valorPotTotal.toFloat();*/

  // conectando wifi
  conectarWifi();

  // inicializando a leitura do sensor DHT 11
  //dht.setup(2, DHTesp::DHT11);

  // inicialização da leitura do ads 1115
  ads.setGain(GAIN_ONE);
  ads.begin();

  //Setando o pino do sensor de presença como entrada
  //pinMode(pinPresenca, INPUT);

  //setando os pinos de jump como entrada
  //pinMode(pinJumpDHT, INPUT);
  //pinMode(pinJumpPresenca, INPUT);

  pinMode(pinTrig, OUTPUT); // Sets the trigPin as an Output
  pinMode(pinEcho, INPUT); // Sets the echoPin as an Input

  pinMode(pinReset, OUTPUT);
  digitalWrite(pinReset, HIGH);

  //setando os pinos do rgb como saída
  pinMode(pinGreenRGB, OUTPUT);
  pinMode(pinRedRGB, OUTPUT);

  //configurando MQTT
  client.setServer(endBroker, atoi(portBroker));
  client.setCallback(callback);

  //Inicialização do NTP
  //Udp.begin(localPort);
  //setSyncProvider(getNtpTime);

  //publica no servidor WEB os valores medidos
  server.on("/data.json", []() {
    server.send(200, "text/html", msgMQTT);
  });
  /*server.on("/pacote.txt", []() {
    server.send(200, "text/html", valorPacote);
    });
    server.on("/json.txt", []() {
    server.send(200, "text/html", msgMQTT);
    });*/


  /*
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
    });*/

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
    lerDistancia();
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
  }



  if (tempo2 - tempoAnterior2 > 5000)
  {
    tempoAnterior2 = tempo2;
    lerNivel();
    //lerCorrente();
    //lerPotencia();
    publicarTopicos();
  }
  /*
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
  */
  //  Serial.println(second());

  //feed do watchdog timer
  ESP.wdtFeed();
}
