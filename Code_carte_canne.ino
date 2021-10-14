//===================================================================================
//--------------------------------CODE WECANE----------------------------------------
//===================================================================================


#include <dht.h>               // Librairie dth11
#include <MPU6050_tockn.h>     // Librairie accéléromètre MPU6050
#include <Wire.h>              // Librairie accéléromètre MPU6050
#include <SoftwareSerial.h>

#define dht_apin A0             // Definition dht11 pin A0
SoftwareSerial sim800l(2,3);    // Definition sim800l ( TX->2 ,RX->3 ) 
MPU6050 mpu6050(Wire);          // Defnition de la connexion MPU6050

//++++++++++++++++++++++++++Déclaration des Varibles+++++++++++++++++++++++++++++++++
float acceleration = 0 ;        
int previous_acc ;
int current_acc ;
int nb_pas = 0  ; 
long prevMillis_accelerometre = 0  ; 
int accelerometre_delay = 200 ; 
long prevMillis_temp = 0 ; 
int temp_delay = 2000 ; 
long prevMillis_pas = 0 ; 
int pas_delay = 3000 ; 
const byte bouton = 7 ; 
int etat_bouton =0 ;
int pin_buzzer  = 9 ;
int fsr_valeur ; 
int fsr_AnalogPin = 1; 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
dht DHT;

//==========================================Partie Initialisation=====================================================
void setup() {
    Serial.begin(115200);                                         // Déclaration communication Bluetooth sur la carte 
    sim800l.begin(9600);                                          // Déclaration communication avec Bluno 
    Wire.begin();                                                 // Déclaration communication avec accéléromètre 
    pinMode(11, OUTPUT);                                         
    mpu6050.begin();                                              // Initialisation accéléromètre 
    delay(500);                                                   // Delay to let system boot
    Serial.println("DHT11 Humidity & temperature Sensor\n\n");    // Information d'attente dht11
    delay(1000);                                                  // Wait before accessing Sensor
    mpu6050.update();                                             // Prise des valeurs de l'accéléromètre 
    previous_acc = mpu6050.getAccZ() * 100 ;                      // Premier prise de l'axe Z de l'accéléromètre 
   pinMode(bouton, INPUT_PULLUP);                                 // Déclaration du bouton avec filtre anti-rebond
}
//=============================================Fin Initialisation=====================================================

//====================================================================================================================
//============================================= WeCane allumée =======================================================
void loop()
{

if (millis() - prevMillis_temp > temp_delay )               // Intérruption temporaire pour l'envoi de la température et l'humiditée 
    {
      DHT.read11(dht_apin);                                 // Lecture de la valeur du capteur 
      byte buffer[3] = {0xAD,                               // Definition de l'ID du message 
                        (byte)DHT.temperature,              // Definition de la température sur un octet pour la data
                        (byte)DHT.humidity                  // Definition de l'humiditée sur un octet pour la data
                       };
    Serial.write(buffer, sizeof(buffer));                   // Envoi par bluetooth des données 
    prevMillis_temp = millis() ;                            // Prise du temps actuel pour définir la prochaine interruption temporaire 
    }

if (millis() - prevMillis_pas > pas_delay  )                // Intérruption temporaire pour l'envoi du nombre de pas 
    { 
    byte buffer3[2] = {0xAB,                                // Definition de l'ID du message
                          (byte)nb_pas                      // Definition de la valeurs de la variable nb_pas sur un octet pour la data
                         };
    Serial.write(buffer3, sizeof(buffer3));                 // Envoi par bluetooth des données 
    prevMillis_pas = millis() ;                             // Prise du temps actuel pour définir la prochaine interruption temporaire 
    }

if (millis() - prevMillis_accelerometre > accelerometre_delay )    // Intérruption temporaire pour l'envoi des valeurs de l'accéléromètre 
    {   
      mpu6050.update();                                            // Prise des valeurs de l'accéléromètre 
      int x = mpu6050.getAccX() * 100 ;                            // Definition x comme valeurs de l'axe x de l'accéromètre avec un gain de 100
      int y = mpu6050.getAccY() * 100 ;                            // Definition y comme valeurs de l'axe y de l'accéromètre avec un gain de 100
      int z = mpu6050.getAccZ() * 100 ;                            // Definition z comme valeurs de l'axe z de l'accéromètre avec un gain de 100
      current_acc = z ;                                            // Definition de z comme valeur de mouvement 
      byte buffer2[4] = {0xAC,                                     // Definition de l'ID du message
                          (byte)x,                                 // Definition de la valeurs de la variable x sur un octet pour la data
                          (byte)y,                                 // Definition de la valeurs de la variable y sur un octet pour la data
                          (byte)z                                  // Definition de la valeurs de la variable z sur un octet pour la data
                         };
      Serial.write(buffer2, sizeof(buffer2));                      // Envoi par bluetooth des données 
      fsr_valeur = analogRead(fsr_AnalogPin);                      // Lecture de la valeur du capteur de force 
          if( previous_acc - current_acc > 100 && fsr_valeur > 30 || previous_acc - current_acc < -100 && fsr_valeur > 30) // Si vous tenez la canne et que vous tombez 
          {
                Send_SMS() ;                             // Envoi d'un SMS d'alerte 
                while(digitalRead(bouton) == HIGH ){     // Réalistion du code ci-dessous tant que le bouton n'est pas appuyé 
                 tone(pin_buzzer , 500 , 50) ;           // Faire sonner le buzzer 
                } 
          }
          if( previous_acc - current_acc > 30 && fsr_valeur > 30 || previous_acc - current_acc < -30 && fsr_valeur > 30)   // Si vous tenez la canne et que vous faites un pas 
          {
                nb_pas++ ;                               // Incrémetation de la variables de pas 
          }
    previous_acc = current_acc ;                         // Mise de jour de la variable de mouvement pour capturer le mouvement 
    prevMillis_accelerometre = millis() ;                // Prise du temps actuel pour définir la prochaine interruption temporaire 
    }

    
    if(Serial.available())                              // Si on reçoit quelque chose sur le module Bluetooth 
    {
             byte cmd = Serial.read();                  // Lire la donnée reçue 
      
          if (cmd == 0x01) {                            // Si le message est 0x01
            tone(pin_buzzer , 500 , 1000) ;             // Faire sonner le buzzer pendant 1 seconde 
          }
    }

    if(digitalRead(bouton) == LOW)                      // Si on appuie sur le bouton physique 
    {
      while(digitalRead(bouton) == LOW ){               // Réalistion du code ci-dessous tant que le bouton est appuyé 
       tone(pin_buzzer , 500 , 50) ;                    // Faire sonner le buzzer 
      }
    }

}  
//====================================================================================================================

//=========================================== Fonction envoi d'un SMS ================================================
void Send_SMS(){
  sim800l.print("AT+CMGF=1\r");                   // Mise en mode rédaction SMS
  delay(100);                                     // Attente de mise en place 
  sim800l.print("AT+CMGS=\"+33760402867\"\r");    // Définition du numéro d'envoi 
  delay(500);                                     // Attente de mise en place
  sim800l.print("LA PERSONNE EST TOMBEE !");      // Rédaction du message 
  delay(500);                                     // Attente de mise en place
  sim800l.print((char)26);                        // Point de fin de rédaction du SMS
  delay(500);                                     // Attente de mise en place
  sim800l.println();                              // Envoi du SMS
  delay(500);                                     // Attente de mise en place
}
//====================================================================================================================
