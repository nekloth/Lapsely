#include <LedControl.h>

/* ********** GLOBAL *************** */

/* MISC */
#define ANTIBOUNCE 200
#define VALEURMINI 2
#define VALEURMAXI 899
#define PASRAPIDE 10
#define DUREEENCLENCHEE 1 
// Le déclenchement sera d'une seconde


const int buttonUP = 2;
const int buttonDOWN = 3;

const int  debounceTime = 50;          // the time in milliseconds required for the switch to be stable
const int  fastIncrement = 500;       // increment faster after this many  milliseconds
int count = 0;                         // count decrements every tenth of a second until reaches 0

int valeurIntervalle = VALEURMINI; //La valeur sélectionnée par l'utilisateur


/* Déclaration de mon afficheur */
LedControl lc= LedControl(12,11,10, 1); //DataIn, Clock, Load

/* Quel est l'état de la session (on en est où ?) */
#define STATUS_SELECTION 0
#define STATUS_TIMELAPSE 1
#define STATUS_ERROR 2

int statusSession = STATUS_SELECTION;

/*  0 = Mode de sélection de la durée
    1 = Mode de TimeLapse !
   99 = Mode erreur 
*/



/* ************* <GESTION DE L'AFFICHAGE> ************************** */
//POUR L'AFFICHAGE DES CHIFFRES
#define CHAR_ERROR -2
#define CHAR_INPROGRESS -1
#define CHAR_ZERO_DIX -10

//L'alphabet
const static byte myAlphabet[56][8]={
  	{0x0E,0x0A,0x0A,0x0A,0x0E,0x00,0x00,0x00},          //0
	{0x02,0x02,0x02,0x02,0x02,0x00,0x00,0x00},          //1
	{0x0E,0x02,0x0E,0x08,0x0E,0x00,0x00,0x00},          //2
	{0x0E,0x02,0x06,0x02,0x0E,0x00,0x00,0x00},          //3
	{0x0A,0x0A,0x0E,0x02,0x02,0x00,0x00,0x00},          //4
	{0x0E,0x08,0x0E,0x02,0x0E,0x00,0x00,0x00},          //5
	{0x0E,0x08,0x0E,0x0A,0x0E,0x00,0x00,0x00},          //6
	{0x0E,0x02,0x02,0x02,0x02,0x00,0x00,0x00},          //7
	{0x0E,0x0A,0x0E,0x0A,0x0E,0x00,0x00,0x00},          //8
	{0x0E,0x0A,0x0E,0x02,0x0E,0x00,0x00,0x00},          //9
        {0xE0,0xA0,0xA0,0xA0,0xE0,0x00,0x00,0x00},          //00 (zéro pour les dizaines)                       --10
	{0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00},          //10                           	                --11
	{0xE0,0x20,0xE0,0x80,0xE0,0x00,0x00,0x00},          //20                            	                --12
	{0xE0,0x20,0xE0,0x20,0xE0,0x00,0x00,0x00},          //30						--13
	{0xA0,0xA0,0xE0,0x20,0x20,0x00,0x00,0x00},          //40						--15
	{0xE0,0x80,0xE0,0x20,0xE0,0x00,0x00,0x00},          //50						--16
	{0xE0,0x80,0xE0,0xA0,0xE0,0x00,0x00,0x00},          //60						--17
	{0xE0,0x20,0x20,0x20,0x20,0x00,0x00,0x00},          //70						--18
	{0xE0,0xA0,0xE0,0xA0,0xE0,0x00,0x00,0x00},          //80						--19
	{0xE0,0xA0,0xE0,0x20,0xE0,0x00,0x00,0x00},          //90						--20
	{0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00},          //100						--21
	{0x00,0x00,0x00,0x00,0x00,0x00,0xA0,0x00},          //200						--22
	{0x00,0x00,0x00,0x00,0x00,0x00,0xA8,0x00},          //300						--23
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x00},          //400						--24
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x40},          //500						--25
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x50},          //600						--26
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x54},          //700						--27
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x55},          //800						--28
        {0x60,0xFF,0x81,0x99,0x99,0x81,0xFF,0x00},          //INPROGRESS					--29
        {0x7E,0x81,0xA5,0x81,0x99,0xA5,0x81,0x7E},          //ERROR						--30
	{0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00},          //Cinq-1						--31
	{0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x00},          //Cinq-2						--32
	{0x00,0x3C,0x20,0x3C,0x04,0x04,0x3C,0x00},          //Cinq-3						--33
	{0x3E,0x20,0x20,0x3C,0x02,0x02,0x02,0x3C},          //Cinq-4						--34
	{0x00,0x00,0x00,0x08,0x08,0x00,0x00,0x00},          //Quatre-1						--35
	{0x00,0x00,0x18,0x18,0x08,0x00,0x00,0x00},          //Quatre-2						--36
	{0x00,0x24,0x24,0x3C,0x04,0x04,0x00,0x00},          //Quatre-3						--37
	{0x22,0x22,0x22,0x22,0x3E,0x02,0x02,0x02},          //Quatre-4						--38
	{0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00},          //Trois-1						--39
	{0x00,0x00,0x18,0x08,0x18,0x18,0x00,0x00},          //Trois-2						--40
	{0x00,0x38,0x04,0x38,0x04,0x04,0x38,0x00},          //Trois-3						--41
	{0x7C,0x02,0x02,0x3C,0x02,0x02,0x02,0x7C},          //Trois-4						--42
	{0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00},          //Deux-1						--43
	{0x00,0x00,0x18,0x08,0x18,0x18,0x00,0x00},          //Deux-2						--44
	{0x00,0x38,0x04,0x18,0x20,0x3C,0x00,0x00},          //Deux-3						--45
	{0x3C,0x02,0x02,0x0C,0x10,0x20,0x20,0x3E},          //Deux-4						--46
	{0x00,0x00,0x00,0x08,0x08,0x00,0x00,0x00},          //Un-1						--47
	{0x00,0x00,0x18,0x08,0x08,0x00,0x00,0x00},          //Un-2						--48
	{0x00,0x00,0x08,0x18,0x08,0x08,0x1C,0x00},          //Un-3						--49
	{0x08,0x18,0x08,0x08,0x08,0x08,0x08,0x1C},          //Un-4						--50
	{0x00,0x00,0x00,0x14,0x14,0x00,0x00,0x00},          //Go-1						--51
	{0x00,0x00,0x36,0x36,0x36,0x00,0x00,0x00},          //Go-2						--52
	{0x00,0x00,0x22,0x55,0x22,0x00,0x00,0x00},          //Go-3						--53
	{0x00,0x62,0x85,0xB5,0x95,0x62,0x00,0x00},          //Go-4						--54
	{0x00,0x00,0x00,0x66,0x66,0x00,0x00,0x00},          //WAIT-1						--55
	{0x00,0x00,0x00,0xDB,0xDB,0x00,0x00,0x00}           //WAIT-2						--56
};




/* Où trouver le chiffre qui va bien */
int getNumPattern(int input){
	switch(input) {
		case 0:	case 1: case 2:	case 3: case 4:	case 5: case 6:	case 7: case 8:	case 9: return input; break;

                case CHAR_ZERO_DIX: return 10; break;case 10: return 11; break;case 20: return 12; break;case 30: return 13; break;
		case 40: return 14; break;case 50: return 15; break;case 60: return 16; break;case 70: return 17; break;
		case 80: return 18; break;case 90: return 19; break;

		case 100: return 20; break;case 200: return 21; break;case 300: return 22; break;case 400: return 23; break;
		case 500: return 24; break;case 600: return 25; break;case 700: return 26; break;case 800: return 27; break;

		case CHAR_INPROGRESS: return 28; break;case CHAR_ERROR: default: return 29; break;
	}
}  


/* Affichage d'un caractère */
void printChar(byte leCar[]) {
  for (int j=0; j<8;j++) {lc.setRow(0,j,leCar[j]);}
}

void printValue(int theValue) {
  byte afficheMoiCa[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  //La valeur demandée n'existe pas, on ne peut pas l'afficher, on affiche le caractère ERROR
  if (theValue>899) { 
    for (int i=0; i<8;i++) {afficheMoiCa[i] |= myAlphabet[getNumPattern(CHAR_ERROR)][i];}
    printChar(afficheMoiCa); return;
  }
 
  int centaine = (int) theValue/100;
  int dizaine = (int) (theValue-centaine*100)/10;  
  int unite = (theValue) - (centaine*100) - (dizaine*10);
  
  //Pour les centaines
  if (centaine>0) {
      for (int i=0; i<8;i++) {
       afficheMoiCa[i] |= myAlphabet[getNumPattern(centaine*100)][i];
    }
  }

  //Pour les dizaines
  if (dizaine==0 && theValue > 99) {
    for (int i=0; i<8;i++) {
       afficheMoiCa[i] |= myAlphabet[getNumPattern(CHAR_ZERO_DIX)][i];
    }
  }

  if (dizaine>0) {
    for (int i=0; i<8;i++) {
       afficheMoiCa[i] |= myAlphabet[getNumPattern(dizaine*10)][i];
    }
  }
  
  //Pour les unités
  for (int i=0; i<8;i++) {
     afficheMoiCa[i] |= myAlphabet[getNumPattern(unite)][i];
  }
  printChar(afficheMoiCa);
}
/* ************* </GESTION DE L'AFFICHAGE> ************************** */



/* ************* <SETUP, on initialise tout ce dont on a besoin !> **************** */
void setup() {
  lc.shutdown(0,false);     //On sort le LEDMATRIX de l'hibernation
  lc.setIntensity(0, 0);    //De 0 à 15 (le deuxième paramètre)
  lc.clearDisplay(0);       //On efface l'écran
  Serial.begin(9600);       //Pour le debug
  
  //Si le bouton TIMELAPSE est enclenché, on passe en ERREUR
  //if ( .... ) statusSession = STATUS_ERROR;  
  pinMode(buttonUP, INPUT);
  pinMode(buttonDOWN, INPUT);
  
}
/* ************* </SETUP, on initialise tout ce dont on a besoin !> **************** */


/* ** Bouton de selection de temps ** */
void pressUP() {if (valeurIntervalle+1 <= VALEURMAXI) {valeurIntervalle++;}}

void pressQUICKUP() {
  if (valeurIntervalle+PASRAPIDE <= VALEURMAXI) {valeurIntervalle+=PASRAPIDE;} else {valeurIntervalle=VALEURMAXI;}
}

void pressDOWN() {if (valeurIntervalle-1 >= VALEURMINI) {valeurIntervalle--;}}

void pressQUICKDOWN() {
   if (valeurIntervalle-PASRAPIDE >= VALEURMINI) {valeurIntervalle-=PASRAPIDE;} else {valeurIntervalle=VALEURMINI;}
}
/* ********** */


long switchTime_UP()
{
  static unsigned long startTime = 0;  // the time the switch state change was first detected
  static boolean state;                // the current state of the switch

  if(digitalRead(buttonUP) != state) // check to see if the switch has changed state
  {
    state = ! state;       // yes, invert the state
    startTime = millis();  // store the time
  }
  if( state == HIGH)
    return millis() - startTime;   // switch pushed, return time in milliseconds
  else
    return 0; // return 0 if the switch is not pushed (in the HIGH state);
}

long switchTime_DOWN()
{
  static unsigned long startTime = 0;  // the time the switch state change was first detected
  static boolean state;                // the current state of the switch

  if(digitalRead(buttonDOWN) != state) // check to see if the switch has changed state
  {
    state = ! state;       // yes, invert the state
    startTime = millis();  // store the time
  }
  if( state == HIGH)
    return millis() - startTime;   // switch pushed, return time in milliseconds
  else
    return 0; // return 0 if the switch is not pushed (in the HIGH state);
}

/* ********** <MODE SELECTION> ********** */
void sessionSELECTION () {
  printValue(valeurIntervalle);
  
  int duration_UP = switchTime_UP();
  if ( duration_UP > fastIncrement) {pressQUICKUP();delay(ANTIBOUNCE);}
    else if ( duration_UP > debounceTime) {pressUP();delay(ANTIBOUNCE);}

  int duration_DOWN = switchTime_DOWN();
  if ( duration_DOWN > fastIncrement) {pressQUICKDOWN();delay(ANTIBOUNCE);}
    else if ( duration_DOWN > debounceTime) {pressDOWN();delay(ANTIBOUNCE);}
  

}
/* ********** </MODE SELECTION> ********** */



/* ********** <MODE TIMELAPSE> ********** */
void sessionTIMELAPSE () {
  printValue(CHAR_INPROGRESS);    
}
/* ********** </MODE TIMELAPSE> ********** */



/* ********** <MODE ERROR> ********** */
void sessionERROR() {
   /*
   if (DECLENCHEUR DESACTIVE) {
      statusSession = STATUS_SELECTION;
      return;
   }
   */
  printValue(CHAR_ERROR);  
}
/* ********** </MODE ERROR> ********** */



/* ************* <LOOP, C'est là que tout se passe> **************** */
void loop() {
  /*
   if (statusSession != STATUS_ERROR && interrupteurEnclencheur=1) {
       statusSession = STATUS_TIMELAPSE;
   }
   if (statusSession != STATUS_ERROR && interrupteurEnclencheur=0) {
       statusSession = STATUS_SELECTION;
   }
   */
  
 
 
 
 
  switch(statusSession) {
    case STATUS_SELECTION:
      sessionSELECTION();
      break;
    case STATUS_TIMELAPSE:
      sessionTIMELAPSE();
      break;
    case STATUS_ERROR:
      sessionERROR();
      break;    
  }

}
/* ************* </LOOP, C'est là que tout se passe> **************** */




