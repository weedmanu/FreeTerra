/*
 * FreeTerra :
 * geston de la lumière
 * gestion du chauffage au point chaud 
 * lecture point froid
 */

#include <NtpClientLib.h> // https://github.com/gmag11/NtpClient  nécessite https://github.com/me-no-dev/ESPAsyncUDP et https://github.com/PaulStoffregen/Time
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <DHT.h>              // https://github.com/adafruit/DHT-sensor-library
#include <FS.h>           
#include "login.h"
#include <RegulTerra.h>

#define INTERVALLE 5000
#define NTP_TIMEOUT 1500

#define DHT_PIN_Point_Chaud 0                         // D3 pin dht point chaud 
#define DHT_PIN_Point_Froid 13                        // D7 pin dht point froid

#define DHTTYPE DHT21                                 // Le type de sonde DHT (11, 21 ou 22)
DHT DHT_Point_Chaud(DHT_PIN_Point_Chaud, DHTTYPE);    // On crée l'objet DHT_Point_Chaud
DHT DHT_Point_Froid(DHT_PIN_Point_Froid, DHTTYPE);    // On crée l'objet DHT_Point_Froid

#define pin_Relais_Chauffage 14                       // D5 pin du Relais pour le Chauffage
RegulTemp Chauffage(pin_Relais_Chauffage);            // On crée l'objet Chauffage

#define pin_Relais_Lumiere 12 											  // D6 pin du Relais pour la Lumière
Eclairage Lampe(pin_Relais_Lumiere);                  // On crée l'objet Lampe

#define pin_Buzzer 15 											          // D8 pin du Buzzer
Alarme Buzzer(pin_Buzzer);                            // On crée l'objet Buzzer

ESP8266WebServer serveur(80);

int8_t Lever_H, Lever_M, Coucher_H, Coucher_M;
int8_t TargetTemp, TargetTempJ, TargetTempN, TempMin, TempMax;
float TempC, TempF, HumiC, HumiF, HysTemp;

int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
const char *ntpServer = "fr.pool.ntp.org";
bool wifiFirstConnected = false;
unsigned long lastmillis = 0;
boolean syncEventTriggered = false;   // vrai si un event arrive
NTPSyncEvent_t ntpEvent;              // dernier event reçu
bool Flag_Init = true;
bool Flag_Alarme_Sonde_PC, Flag_Alarme_Sonde_PF, Flag_Alarme_Temp_Min, Flag_Alarme_Temp_Max = false;
bool Etat_Lum, Etat_Chauff = false;

String date, heure;

void onSTAConnected (WiFiEventStationModeConnected ipInfo) {
    Serial.printf("Connecté à %s\r\n", ipInfo.ssid.c_str ());
}

// Démarrer NTP uniquement après la connexion au réseau IP
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
    Serial.printf ("IP obtenue : %s\r\n", ipInfo.ip.toString ().c_str ());
    Serial.printf("Connecté : %s\r\n", WiFi.status () == WL_CONNECTED ? "oui" : "non");
    wifiFirstConnected = true;
}

// Gérer la déconnexion du réseau
void onSTADisconnected (WiFiEventStationModeDisconnected event_info) {
    Serial.printf("Déconnecté du SSID : %s\n", event_info.ssid.c_str ());
    Serial.printf("Raison : %d\n", event_info.reason);
    NTP.stop(); 																	
    WiFi.reconnect();
}

void processSyncEvent (NTPSyncEvent_t ntpEvent) {
    if (ntpEvent < 0) {
        Serial.printf("Erreur de synchronisation de l'heure: %d\n", ntpEvent);
        if (ntpEvent == noResponse)            Serial.println("Serveur NTP inaccessible");
        else if (ntpEvent == invalidAddress)   Serial.println("Adresse de serveur NTP invalide");
        else if (ntpEvent == errorSending)     Serial.println("Erreur lors de l'envoi de la demande");
        else if (ntpEvent == responseError)    Serial.println("Erreur de réponse NTP");
    } else {
        if (ntpEvent == timeSyncd) {
            Serial.print("Récupération du temps NTP : ");
            Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
        }
    }
}

// la fonction qui lit et traite les fichiers presents dans la mémoire SPIFFS 
  bool loadFromSpiffs(String path){        
    if (SPIFFS.exists(path)) { // If the file exists
      String dataType = "text/plain";
      if(path.endsWith("/")) path += "index.html";
      if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
      else if(path.endsWith(".html")) dataType = "text/html";
      else if(path.endsWith(".htm")) dataType = "text/html";
      else if(path.endsWith(".css")) dataType = "text/css";
      else if(path.endsWith(".js")) dataType = "application/javascript";
      else if(path.endsWith(".csv")) dataType = "text/csv";
      else if(path.endsWith(".png")) dataType = "image/png";
      else if(path.endsWith(".gif")) dataType = "image/gif";
      else if(path.endsWith(".jpg")) dataType = "image/jpeg";
      else if(path.endsWith(".ico")) dataType = "image/x-icon";
      else if(path.endsWith(".xml")) dataType = "text/xml";
      else if(path.endsWith(".pdf")) dataType = "application/pdf";
      else if(path.endsWith(".zip")) dataType = "application/zip";
      File dataFile = SPIFFS.open(path.c_str(), "r");    
      if (serveur.hasArg("download")) {
        dataType = "application/octet-stream";
      }
      if (serveur.streamFile(dataFile, dataType) != dataFile.size()) {    
        Serial.println("Fichier mal chargé");
      }
      dataFile.close();  
      return true;
    } else {
      return false;
    }
  }

void terrarium () {   
  if ((millis() - lastmillis) > INTERVALLE) {        
    lastmillis = millis();   
    if (Flag_Init == false) {        
      Serial.println("***** Terrarium *****");
      Serial.println("Config --> ");
      Serial.print("target temp : ");           Serial.println(TargetTemp);
      Serial.print("TargetTempJ : ");           Serial.println(TargetTempJ);   
      Serial.print("TargetTempN : ");           Serial.println(TargetTempN);
      Serial.print("HysTemp : ");               Serial.println(HysTemp);
      Serial.print("TempMax : ");               Serial.println(TempMax);
      Serial.print("TempMin : ");               Serial.println(TempMin);    
      Serial.print("Lever_H : ");               Serial.println(Lever_H);    
      Serial.print("Lever_M : ");               Serial.println(Lever_M);
      Serial.print("Coucher_H : ");             Serial.println(Coucher_H);  
      Serial.print("Coucher_M : ");             Serial.println(Coucher_M);        
      Serial.println();
      Serial.println("datas --> "); 
      Serial.print(WiFi.isConnected() ? "connecté" : "non connecté"); Serial.println(" au réseau.");
      Serial.print("Heure : "); Serial.print(NTP.getTimeStr());  Serial.println(NTP.isSummerTime() ? " Heure d'été." : " Heure d'hiver.");  
      Serial.print("Date  : "); Serial.println(NTP.getDateStr()); 
      
      date = NTP.getDateStr();
      heure = NTP.getTimeStr();
      int8_t Maintenant_H = hour();
      int8_t Maintenant_M = minute();
      
      if (Lampe.gestion( Maintenant_H, Maintenant_M, Lever_H, Lever_M, Coucher_H, Coucher_M)) { 
        Serial.println("lumière ON"); 
        TargetTemp = TargetTempJ;
        Etat_Lum = true;
      } else { 
        Serial.println("lumière OFF"); 
        TargetTemp = TargetTempN;
        Etat_Lum = false;
      }  
      
      TempC = DHT_Point_Chaud.readTemperature();
      HumiC = DHT_Point_Chaud.readHumidity();            
      TempF = DHT_Point_Froid.readTemperature();
      HumiF = DHT_Point_Froid.readHumidity();         
  
      if (isnan(TempC) || isnan(HumiC)) { 
        Serial.println("Échec de la lecture du capteur DHT Point Chaud!"); 
        Flag_Alarme_Sonde_PC = true;
        Buzzer.Alerte_Sonore();
      } else {
        Flag_Alarme_Sonde_PC = false;
      }
      Serial.print("Température Point Chaud: ");
      Serial.print(TempC);
      Serial.println(" °C");
      Serial.print("Humidité Point Chaud: ");
      Serial.print(HumiC);
      Serial.println(" %");   
      if (isnan(TempF) || isnan(HumiF)) { 
        Serial.println("Échec de la lecture du capteur DHT Point Froid!"); 
        Flag_Alarme_Sonde_PF = true;
        Buzzer.Alerte_Sonore();
      } else {
        Flag_Alarme_Sonde_PF = false;
      }      
      Serial.print("Température Point Froid: ");
      Serial.print(TempF);
      Serial.println(" °C");
      Serial.print("Humidité Point Froid: ");
      Serial.print(HumiF);
      Serial.println(" %");   
      if (Chauffage.gestion(TempC, TargetTemp, HysTemp)) { Etat_Chauff = true; Serial.println("chauffage ON"); }
      else { Etat_Chauff = false; Serial.println("chauffage OFF"); }  
      
      if (Buzzer.Alerte_Temp(TempC, TempMin, TempMax) == 0) { 
        Serial.println("Alerte temp min !!!"); 
        Flag_Alarme_Temp_Min = true;
      } else if (Buzzer.Alerte_Temp(TempC, TempMin, TempMax) == 2) { 
        Serial.println("Alerte temp max !!!"); 
        Flag_Alarme_Temp_Max = true;
      } else { 
        Serial.println("Température OK"); 
        Flag_Alarme_Temp_Min = false;
        Flag_Alarme_Temp_Max = false;
      }
      Serial.println("********************");
      Serial.println("");
    } else {
      Serial.println("Première Connexion !!!");
      Serial.println("Vous devez envoyer les paramètres par l'application");
      Serial.println();
      Buzzer.Alerte_Sonore(); 
    }
  }
}

// la fonction qui renvoie vers la page index.html quand on est a la racine du serveur
void handleRoot(){
  serveur.sendHeader("Location", "/index.html",true);   
  serveur.send(302, "text/plain","");
}

// la fonction qui traite la requète HTTP
void handleWebRequests(){
  if(loadFromSpiffs(serveur.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += serveur.uri();
  message += "\nMethod: ";
  message += (serveur.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += serveur.args();
  message += "\n";
  for (uint8_t i=0; i<serveur.args(); i++){
    message += " NAME:"+serveur.argName(i) + "\n VALUE:" + serveur.arg(i) + "\n";
  }
  serveur.send(404, "text/plain", message);
  Serial.println(message);
}

void load_from_file(String file_name) {  
  delay(50);
  Serial.println();
  Serial.println("Chargement du fichier config.csv --> ");  
  String result;
  if (SPIFFS.exists(file_name)) { 
    File this_file = SPIFFS.open(file_name.c_str(), "r");   
    if (!this_file) { 
      Serial.println(result);
    }
    while (this_file.available()) {
        result += (char)this_file.read();
    }  
    this_file.close();
    
    Serial.println();
    Serial.println(result);
    Serial.println();    
    
    int i1 = result.indexOf(',');
    int i2 = result.indexOf(',', i1+1);
    int i3 = result.indexOf(',', i2+1);
    int i4 = result.indexOf(',', i3+1);
    int i5 = result.indexOf(',', i4+1);
    int i6 = result.indexOf(',', i5+1);
    int i7 = result.indexOf(',', i6+1);
    int i8 = result.indexOf(',', i7+1);
    int i9 = result.indexOf(',', i8+1);
    int i10 = result.indexOf(',', i9+1);
    int i11 = result.indexOf(',', i10+1);    

    TargetTempJ = (result.substring(0, i1)).toInt();
    TargetTempN = (result.substring(i1 + 1, i2)).toInt(); 
    HysTemp = (result.substring(i2 + 1, i3)).toFloat();  
    TempMax = (result.substring(i3 + 1, i4)).toInt();
    TempMin = (result.substring(i4 + 1, i5)).toInt();
    Lever_H = (result.substring(i5 + 1, i6)).toInt();
    Lever_M = (result.substring(i6 + 1, i7)).toInt();
    Coucher_H = (result.substring(i7 + 1, i8)).toInt();
    Coucher_M = (result.substring(i8 + 1, i9)).toInt();
    
    Serial.println("Config chargée");
    Serial.println();           
    Flag_Init = false;
  } else {
    Flag_Init = true;
  }
}

void recup_datas_appli() {      
  Serial.println("***** Réception des datas envoyées par l'application *****");    
  String message = "Nombre d'arguments reçu : ";
  message += serveur.args();                                  // récuperation du nombre d'arguments
  message += "\n";  
  for (int i = 0; i < serveur.args(); i++) {
    message += "Argument" + (String)i + " –> ";               // Include the current iteration value
    message += serveur.argName(i) + ": ";                     // récuperation du nom de l'argument
    message += serveur.arg(i) + "\n";                         // récuperation de la valeur l'argument
  }  
  
  Serial.println(message);
  Serial.println();    
        
  TargetTempJ = (serveur.arg(0)).toInt();
  TargetTempN = (serveur.arg(1)).toInt();
  HysTemp = (serveur.arg(2)).toFloat(); 
  TempMax =(serveur.arg(3)).toInt();
  TempMin = (serveur.arg(4)).toInt();
  Lever_H = (serveur.arg(5)).toInt();  
  Lever_M = (serveur.arg(6)).toInt();  
  Coucher_H = (serveur.arg(7)).toInt(); 
  Coucher_M = (serveur.arg(8)).toInt();   

  Serial.println();
  Serial.print("Écriture Config la mémoire  --> ");    
  File file = SPIFFS.open("/config.csv", "w+");
  if(!file){    
    Serial.println("Impossible d'ouvrir config.csv");
    Serial.println();
    return;
  } else {
    String Config = String(TargetTempJ) + "," + String(TargetTempN) + "," + String(HysTemp) + "," + 
                    String(TempMax) + "," + String(TempMin) + "," + 
                    String(Lever_H) + "," + String(Lever_M) + "," + String(Coucher_H) + "," + String(Coucher_M);
    file.println(Config);
    file.close();     
    Serial.println(Config); 
    Serial.println("Config modifié !!!");
    Serial.println();
    load_from_file("/config.csv");             
  }
  serveur.send(200, "text/plain", "Traitement config ok");  
  Serial.println("***** Fin du Traitement des datas envoyées par l'application *****");
}

void envoyer_datas() {    
  String datas = String(date) + "," + String(heure) + "," + String(TargetTemp) + "," +  String(TempC) + "," + String(HumiC) + "," +  
                 String(TempF) + "," + String(HumiF) + "," + String(Flag_Alarme_Sonde_PC) + "," + String(Flag_Alarme_Sonde_PF) + "," +
                 String(Flag_Alarme_Temp_Min) + "," + String(Flag_Alarme_Temp_Max) + "," + String(Etat_Lum) + "," + String(Etat_Chauff);    
  serveur.send(200, "text/plain", datas);
}

void envoyer_config() {    
  String Config = String(date) + "," + String(heure) + "," + String(TargetTempJ) + "," + String(TargetTempN) + "," + String(HysTemp) + "," + 
                  String(TempMax) + "," + String(TempMin) + "," + 
                  String(Lever_H) + "," + String(Lever_M) + "," + String(Coucher_H) + "," + String(Coucher_M);
  serveur.send(200, "text/plain", Config);
}

void setup() {
  static WiFiEventHandler e1, e2, e3;
	DHT_Point_Chaud.begin();
  DHT_Point_Froid.begin();
  
  Serial.begin (115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(RESEAU, PASSWORD);
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
      ntpEvent = event;
      syncEventTriggered = true;
  });

  if(!SPIFFS.begin()){ 
    Serial.println("Une erreur s'est produite lors du montage SPIFFS");  
  }

  serveur.on("/",handleRoot);                                   // on associe la fonction handleRoot à la racine du serveur
  serveur.on("/datas", envoyer_datas);                          // on associe la fonction envoyer_datas au chemin IP/datas
  serveur.on("/config", recup_datas_appli);                     // on associe la fonction recup_datas_appli au chemin IP/config      
  serveur.on("/data_config", envoyer_config);                   // on associe la fonction affiche_config au chemin IP/data_config   
  serveur.onNotFound(handleWebRequests);
  serveur.begin();
  ArduinoOTA.setPort(8266);                     // le port de transmission   
  ArduinoOTA.setHostname(HOST);                 // on donne un nom au port réseau  
  ArduinoOTA.begin();                           // on démarre le port réseau
  e1 = WiFi.onStationModeGotIP (onSTAGotIP);										// Dès que le WiFi est connecté, démarrez le client NTP
  e2 = WiFi.onStationModeDisconnected (onSTADisconnected);
  e3 = WiFi.onStationModeConnected (onSTAConnected);
  load_from_file("/config.csv");
}

void loop() {    
  serveur.handleClient();
  ArduinoOTA.handle();                          // on écoute en boucle
  
  if (wifiFirstConnected) {
      wifiFirstConnected = false;
      NTP.setInterval(600);
      NTP.setNTPTimeout(NTP_TIMEOUT);
      NTP.begin(ntpServer, timeZone, true, minutesTimeZone);
  }

  if (syncEventTriggered) {
      processSyncEvent(ntpEvent);
      syncEventTriggered = false;
  }
  
  terrarium(); 
}
