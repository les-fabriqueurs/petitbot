///////////////
// Petit Bot //
///////////////
// Un programme pedagogique des petits debrouillards ?=+ pour gerer le robot "Petit Bot" 
// Voir sur http://wikidebrouillard.org/index.php?title=Petit_Bot_un_robot_controlable_en_Wifi
// Ce programme est inspire de : http://www.esp8266.com/viewtopic.php?f=29&t=6419#sthash.gd1tJhwU.dpuf
// Sous licence CC-By-Sa
// Par des gens bien

#include <ESP8266WiFi.h> // on appelle la bibliotheque qui gere le WemosD1 mini
//////////////////////////
//  Definition du WiFi //
//////////////////////////
const char WiFiAPPSK[] = "1234567890"; //C'est le mot de passe du wifi "petit bot"

/////////////////////////////
// Attribution des broches //
/////////////////////////////

#include <Servo.h> //on appelle la bibliotheque qui gere les servomoteurs

Servo monservo1; //nom du servo qui gere la premiere roue

Servo monservo2; //seconde roue

WiFiServer server(80);


#define AUTO 1
#define MANUEL 0
#define DISTMUR 10
#define NOMWIFI "petitbot8"

// définition des broches utilisées 
int trig = 0; 
int echo = 2; 
long lecture_echo; 
long cm;
int modeBot = MANUEL;
int distance =0;

void setup()
{
  initHardware();
  setupWiFi();
  server.begin();
  monservo1.attach(4);  // changer pour avoir le bon connecteur du moteur 1
  monservo2.attach(5);  // changer pour avoir le bon connecteur du moteur 2
  
  monservo1.write(90); //pour mettre les servomoteur à l'arret, on leur donne une valeur au milieu entre 0 et 180.
  monservo2.write(90); 

  pinMode(trig, OUTPUT); 
  digitalWrite(trig, LOW); 
  pinMode(echo, INPUT); 

}

void calcDistance() {
    digitalWrite(trig, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(trig, LOW); 
    lecture_echo = pulseIn(echo, HIGH); 
    distance = lecture_echo / 58.8235; // en fait 58.8235 et pas 58
}

void loop()
{

   calcDistance();

   if (modeBot == AUTO) {
      if (distance < DISTMUR) { 

        // on recule 
         monservo2.write(0);   //servo 2 roule vers l'arriere. 
         monservo1.write(180); //servo 1 roule vers l'arriere. 
         delay(500);
         
        // on va a gauche
         monservo1.write(180);     //vers l'avant. 
         monservo2.write(180);     //vers l'arriere.
         delay(500);
         
      } else { // on avance
         monservo2.write(180); //vers l'avant.
         monservo1.write(0);   //vers l'avant        
      }
      delay(100);
   }

  
  // Vérifie si il y a navigateur connecte
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Lit la premiere ligne de la requete
  String req = client.readStringUntil('\r');
  
  client.flush();

  // affecte une valeur a "val" suivant la requete 
  int val = -1; 

  if (req.indexOf("/stop") != -1)
    val = 0; 
  else if (req.indexOf("/avance") != -1)
    val = 1;
  else if (req.indexOf("/recule") != -1)
    val = 2; 
  else if (req.indexOf("/gauche") != -1)
    val = 3; 
  else if (req.indexOf("/droite") != -1)
    val = 4; 
  else if (req.indexOf("/auto") != -1) 
    val = 5;
  else if (req.indexOf("/manuel") != -1)
    val = 6;



  // Prepare la page web de réponse. on commence par le "header" commun; (le code HTML sera ecrit dans la chaine de carcatere s).
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n";
  s += "<html>\r\n";
  
  s += "<center>";
  s += "<h1 style=\"font-size:300%;\"\> Le petit bot ";

  // on va finaliser la chaine de caracteres et envoyer les instructions
  if (val == 2 ) //Recule
  {
    s += " recule ";      //comme les servomoteurs sont colles un dans un sens et le second dans l'autre sens, il faut leur donner a chacun des instructions inverses.
    monservo2.write(0);   //servo 2 roule vers l'arriere. 
    monservo1.write(180); //servo 1 roule vers l'arriere. 
    modeBot = MANUEL;
  }
  if (val == 1)//Avance
  {
    s += " avance ";
    monservo2.write(180); //vers l'avant.
    monservo1.write(0);   //vers l'avant.
    modeBot = MANUEL;
  }
  if (val == 0)//stop
  { 
    s += " est a l'arret ";
    monservo1.write(90);  //stop
    monservo2.write(90);  //stop
    modeBot = MANUEL;
  }
  if (val == 3)//tourne a gauche
  { 
    s += " tourne a gauche "; //Pour faire tourner, il suffit qu'une roue tourne dans un sens et la seconde dans le sens inverse
    monservo1.write(180);     //vers l'avant. 
    monservo2.write(180);     //vers l'arriere.
    modeBot = MANUEL;
  }
  if (val == 4)//tourne a droite
  { 
    s += " tourne a droite ";
    monservo1.write(0);  //vers l'arriere.
    monservo2.write(0);  //vers l'avant. 
    modeBot = MANUEL;
  }
  if (val == 5) 
  {
    modeBot = AUTO;
  }
  if (val == 6)
  {
    modeBot = MANUEL;
    monservo1.write(90);  //stop
    monservo2.write(90);  //stop
  }

  s += "</h1>";
  s += "<br>"; //aller a la ligne
  s += "<br>"; //aller a la ligne
  s += "<a href=\"/stop\"\"><button style=\"font-size:200%; width: 18%\"\>Stop </button></a>\r\n";// creer un boutton "Stop"
  s += "<a href=\"/avance\"\"><button style=\"font-size:200%; width: 18%\"\>Avance </button></a>\r\n";
  s += "<a href=\"/recule\"\"><button style=\"font-size:200%; width: 18%\"\>Recule </button></a>\r\n";
  s += "<a href=\"/droite\"\"><button style=\"font-size:200%; width: 18%\"\>Droite </button></a>\r\n";
  s += "<a href=\"/gauche\"\"><button style=\"font-size:200%; width: 18%\"\>Gauche </button></a><br />\r\n";
  s += "</center>";
  
  s += "</html>\n"; //Fin de la page Web

  // Envoie de la reponse au navigateur
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");
  client.flush();

}

void setupWiFi() //reglage du reseau wifi emis par le WemosD1 mini
{
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = NOMWIFI; //nom du reseau

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK, 7);
}

void initHardware()
{
  Serial.begin(115200); //ouvre une connection serie pour monitorer le fonctionnement du code quand on reste branche a l'ordinateur
} 
