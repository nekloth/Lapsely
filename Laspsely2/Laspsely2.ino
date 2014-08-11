#include <LedControl.h>

/* ****** NOTES ****************************************************************************************************************

-- 22FEB2014
Aujourd'hui, j'ai connecté l'interrupteur pour indiquer si on est en mode SELECTION (choix de la durée) ou TIMELAPSE (on commande
l'appareil).

Il est important que l'interrupteur soit connecté à +3.3V/+5 dans une position, et SURTOUT à la masse pour l'autre position. 

Sinon, cela crée des faux signaux, et ... ça CLIGNOTE !!

-- 

******************************************************************************************************************************** */



/*
 _   _            _       _     _           
| | | |          (_)     | |   | |          
| | | | __ _ _ __ _  __ _| |__ | | ___  ___ 
| | | |/ _` | '__| |/ _` | '_ \| |/ _ \/ __|  _____ _____ _____ _____ _____ _____ _____  
\ \_/ / (_| | |  | | (_| | |_) | |  __/\__ \ |_____|_____|_____|_____|_____|_____|_____|
 \___/ \__,_|_|  |_|\__,_|_.__/|_|\___||___/

 */

//Délais (en ms) pendant lequel un "souble clic" n'est pas interprété
#define ANTIBOUNCE 170

//Délais (en s) minimal de l'intervalle, sélectionnable par l'utilisateur
#define VALEURMINI 5

//Délais (en s) maximal de l'intervalle, sélectionnable par l'utilisateur
#define VALEURMAXI 899

//En cas d'appui LONG, pas d'incrémentation/décrémentation
#define PASRAPIDE 10

//Délais (en s) pendant lequel le signal de prise de photo est envoyé à l'appareil
#define DUREEENCLENCHEE 1

// Délais (en s) pendant lequel le message GO est affiché avant de commencer à déclenché l'appareil
#define DELAIS_AFFICHAGE_GO 2

// Défini les ETATS de l'application : mode selection = l'utilisateur défini le temps
#define STATUS_SELECTION 0

// Défini les ETATS de l'application : mode timelapse = c'est parti, on prend des photos
#define STATUS_TIMELAPSE 1

// Défini les ETATS de l'application : mode error = quelque chose ne va pas
#define STATUS_ERROR 2

// INITialisation du status
int statusSession = STATUS_SELECTION;


//Les PORTS de l'ARDUINO utilisé
const int buttonUP = 2;              //Port pour le bouton+ 
const int buttonDOWN = 3;            //Port pour le bouton-
const int switchLETSGO = 4;          //Port pour le switch enclencheur
const int relais = 5;				 //Pour pour le relais


/* *** SNIPPET FOR LONGPRESS *** */
const int  debounceTime = 50;          // the time in milliseconds required for the switch to be stable
const int  fastIncrement = 500;       // increment faster after this many  milliseconds
//int count = 0;                         // count decrements every tenth of a second until reaches 0



//Délais entre deux prises de vue (initialisé au mini, défini par l'utilisateur)
int valeurIntervalle = VALEURMINI; //La valeur sélectionnée par l'utilisateur


//INITialisation de l'afficheur
LedControl lc= LedControl(12,11,10, 1); //DataIn, Clock, Load


// Variables pour une interruption possible pendant le déclenchement
boolean nouveauDeclenchement = true;
long heureDeclenchement;


// Variables pour l'affichage d'un pixel aléatoire
int a = (int)random(0,8);
int b = (int)random(0,8);
long uneColonne;
long uneLigne;
int c = 0; //NON UTILISE POUR LE MOMENT (le but ... if (c%4==0), affiche la LED, sinon ... rien (pour divisé par 4 le nombre de LED affichées au total)




/* *** <Definition du jeu de caractères > *** */


// Les caractères spéciaux
#define CHAR_ERROR -2
#define CHAR_INPROGRESS -1
#define CHAR_ZERO_DIX -10
#define CHAR_GO -3
#define CHAR_BLANK -4


//L'alphabet
const static byte myAlphabet[56][8]={
  	{0x0E,0x0A,0x0A,0x0A,0x0E,0x00,0x00,0x00},	//0
	{0x02,0x02,0x02,0x02,0x02,0x00,0x00,0x00},	//1
	{0x0E,0x02,0x0E,0x08,0x0E,0x00,0x00,0x00},	//2
	{0x0E,0x02,0x06,0x02,0x0E,0x00,0x00,0x00},	//3
	{0x0A,0x0A,0x0E,0x02,0x02,0x00,0x00,0x00},	//4
	{0x0E,0x08,0x0E,0x02,0x0E,0x00,0x00,0x00},	//5
	{0x0E,0x08,0x0E,0x0A,0x0E,0x00,0x00,0x00},	//6
	{0x0E,0x02,0x02,0x02,0x02,0x00,0x00,0x00},	//7
	{0x0E,0x0A,0x0E,0x0A,0x0E,0x00,0x00,0x00},	//8
	{0x0E,0x0A,0x0E,0x02,0x0E,0x00,0x00,0x00},	//9
    {0xE0,0xA0,0xA0,0xA0,0xE0,0x00,0x00,0x00},	//00 (zéro pour les dizaines)		--10
	{0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00},	//10                           	    --11
	{0xE0,0x20,0xE0,0x80,0xE0,0x00,0x00,0x00},	//20                            	--12
	{0xE0,0x20,0xE0,0x20,0xE0,0x00,0x00,0x00},	//30								--13
	{0xA0,0xA0,0xE0,0x20,0x20,0x00,0x00,0x00},	//40								--14
	{0xE0,0x80,0xE0,0x20,0xE0,0x00,0x00,0x00},	//50								--15
	{0xE0,0x80,0xE0,0xA0,0xE0,0x00,0x00,0x00},	//60								--16
	{0xE0,0x20,0x20,0x20,0x20,0x00,0x00,0x00},	//70								--17
	{0xE0,0xA0,0xE0,0xA0,0xE0,0x00,0x00,0x00},	//80								--18
	{0xE0,0xA0,0xE0,0x20,0xE0,0x00,0x00,0x00},	//90								--19
	{0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00},	//100								--20
	{0x00,0x00,0x00,0x00,0x00,0x00,0xA0,0x00},	//200								--21
	{0x00,0x00,0x00,0x00,0x00,0x00,0xA8,0x00},	//300								--22
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x00},	//400								--23
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x40},	//500								--24
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x50},	//600								--25
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x54},	//700								--26
	{0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x55},	//800								--27
    {0x60,0xFF,0x81,0x99,0x99,0x81,0xFF,0x00},	//INPROGRESS						--28
    {0x7E,0x81,0xA5,0x81,0x99,0xA5,0x81,0x7E},	//ERROR								--29
    {0x00,0x62,0x85,0xB5,0x95,0x62,0x00,0x00},	//Go								--30
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}	//BLANK								--31
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
		case CHAR_INPROGRESS: return 28; break;
        case CHAR_GO: return 30; break;
        case CHAR_BLANK: return 31; break;
		case CHAR_ERROR: default: return 29; break;
		
	}
}  


/* Affichage d'un caractère */
void printChar(byte leCar[]) {
  for (int j=0; j<8;j++) {lc.setRow(0,j,leCar[j]);}
}

void printValue(int theValue) {
  byte afficheMoiCa[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  
  //On cherche à afficher un caractère spécial
  if(theValue==CHAR_ERROR || theValue==CHAR_INPROGRESS || theValue==CHAR_GO || theValue==CHAR_BLANK) {
    for (int i=0; i<8;i++) {afficheMoiCa[i] |= myAlphabet[getNumPattern(theValue)][i];}
    printChar(afficheMoiCa); return;
  }

  //La valeur demandée n'existe pas, on ne peut pas l'afficher, on affiche le caractère ERROR
  if (theValue>899) { 
    for (int i=0; i<8;i++) {afficheMoiCa[i] |= myAlphabet[getNumPattern(CHAR_ERROR)][i];}
    printChar(afficheMoiCa); return;
  }

  // On détermine ce qui doit être affiché
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
  lc.shutdown(0,false);                 //On sort le LEDMATRIX de l'hibernation
  lc.setIntensity(0, 0);                //De 0 à 15 (le deuxième paramètre)
  lc.clearDisplay(0);                   //On efface l'écran
  Serial.begin(9600);                   //Pour le debug
  
  //Si le bouton TIMELAPSE est enclenché, on passe en ERREUR
  pinMode(buttonUP, INPUT);            //Pour le bouton+
  pinMode(buttonDOWN, INPUT);          //Pour le bouton-
  pinMode(switchLETSGO, INPUT);        //Pour le switch enclenchement
  pinMode(relais,OUTPUT);				//Pour le relais
  if(digitalRead(switchLETSGO)==HIGH) statusSession = STATUS_ERROR;
}
/* ************* </SETUP, on initialise tout ce dont on a besoin !> **************** */


/* ** Bouton de selection de temps ************************************************* */
void pressUP() {if (valeurIntervalle+1 <= VALEURMAXI) {valeurIntervalle++;}}
void pressQUICKUP() {if (valeurIntervalle+PASRAPIDE <= VALEURMAXI) {valeurIntervalle+=PASRAPIDE;} else {valeurIntervalle=VALEURMAXI;}}
void pressDOWN() {if (valeurIntervalle-1 >= VALEURMINI) {valeurIntervalle--;}}
void pressQUICKDOWN() {if (valeurIntervalle-PASRAPIDE >= VALEURMINI) {valeurIntervalle-=PASRAPIDE;} else {valeurIntervalle=VALEURMINI;}}
/* ********** */


/* ********** <On appuie sur le bouton UP> ********** */
long switchTime_UP() {
  static unsigned long startTime = 0;
  static boolean state;
  if(digitalRead(buttonUP) != state)
  {
    state = ! state;
    startTime = millis();
  }
  if( state == HIGH) 
    return millis() - startTime;   
  else
    return 0;
}
/* ********** </On appuie sur le bouton UP> ********** */



/* ********** <On appuie sur le bouton DOWN> ********** */
long switchTime_DOWN() {
  static unsigned long startTime = 0;
  static boolean state;

  if(digitalRead(buttonDOWN) != state)
  {
    state = ! state;
    startTime = millis();
  }
  if( state == HIGH)
    return millis() - startTime;
  else
    return 0;
}
/* ********** </On appuie sur le bouton DOWN> ********** */





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
  
  if (nouveauDeclenchement) {
    nouveauDeclenchement = false;
    printValue(CHAR_GO);
    delay(DELAIS_AFFICHAGE_GO*1000);
    heureDeclenchement = millis();
    lc.clearDisplay(0);
  } else {
    
    uneColonne = random(0,8);
    while (a == (int)uneColonne) {
      uneColonne = random(0,8);
    }
    a = uneColonne;
    
    uneLigne = random(0,8);
    while (b == (int)uneLigne) {
      uneLigne = random(0,8);
    }
    b = uneLigne;
    
    lc.clearDisplay(0);
    lc.setLed(0,(int)uneColonne,(int)uneLigne,true);
    delay(500);
    
    if (millis()-heureDeclenchement>=valeurIntervalle*1000) {
      //ON DECLENCHE
      lc.clearDisplay(0);
      printValue(CHAR_INPROGRESS);
      delay(1000);
      heureDeclenchement = millis(); 
    }
  }   
}
/* ********** </MODE TIMELAPSE> ********** */







/* ********** <MODE ERROR> ********** */
void sessionERROR() {
  printValue(CHAR_ERROR);  
}
/* ********** </MODE ERROR> ********** */







/* ************* <LOOP, C'est là que tout se passe> **************** */
void loop() {

	// Si LETSGO n'est pas enclenché
	if (digitalRead(switchLETSGO) == LOW) { 
		statusSession = STATUS_SELECTION;
		if (!nouveauDeclenchement) nouveauDeclenchement=true;		
	}

	// Si LETSGO est enclenché, mais qu'on n'est pas en ERROR, on y va !
	if (statusSession != STATUS_ERROR && digitalRead(switchLETSGO) == HIGH) {
		statusSession = STATUS_TIMELAPSE;
	}	 




	// On appelle la fonction adéquate selon l'état
	switch(statusSession) {
		case STATUS_SELECTION:
			sessionSELECTION();break;
		case STATUS_TIMELAPSE:
			sessionTIMELAPSE();break;
		case STATUS_ERROR:
			sessionERROR();break;    
	}

}
/* ************* </LOOP, C'est là que tout se passe> **************** */



