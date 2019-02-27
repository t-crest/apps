/*****************************************************************************
*                                                                            *
* Projet  :	VIP4F						                                 *
*                                                                            *
* Module  :	F51_Inv = MAX(Iinv)							        *
*                                                                            *
* Description : Protection a maximum de courant inverse.                     *
*                                                                            *
* ************************************************************************** *
*****************************************************************************/ 

#include "../include/define.h"

// table pour comptage des tempos a temps inverse
#include "TempsInv.h"
#include "Pts.h"
#include "Bit.h"

// constantes propres a la fonction 

// stucture des reglages  
typedef struct strucReglages {
	char  On;
	char  Courbe;
	long  Seuil;
	short Tempo;
} strucReglages;

static strucReglages Reglages;		

// stucture de donnees pour les reglages operationnels 
typedef struct strucParamFonc {
	/*------------ transposition des reglages E2Prom --------*/
	char  On;
	char  Courbe;
	long  SeuilDecl;
	long  SeuilDega;
	long  Tempo;
   /*------------- variables de travail ------*/
	long  CptTempo;
	long  KX;
	long  SeuilConst;	 
} strucParamFonc;

static strucParamFonc ParamFonc[5];

static unsigned long  PtsCanIn2;// Pour prise en compte cr�te filtr�e.

char  VA_ProtStatus[5];	// sorties des protections #############� dimensionner

/*****************************************************************************
*                                                                            *
* Fonction: calculParam   				                                     *
*                                                                            *
* Appel   : calculParam(inst)	                               		         *
*                                                                            *
* Entrees : 					                                             *
* 	inst = instance de la protection (0) 		                             *
*                                                                            *
* Sorties : 					                                             *
* 	variables globales F51_Inv								                     *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*                                                                            *
*                                                                            *
*****************************************************************************/
 
static void calculParam(char inst) {
	

long valeur;
unsigned long	val, DegaTalon, seuilDecl;
float	 dval;

	
	// ON-OFF
	ParamFonc[inst].On = (char)(Reglages.On);
   
    // Courbe
	ParamFonc[inst].Courbe = (char)(Reglages.Courbe);

	// Seuil de d�clenchement en points CAN	(V_TRS_In = courant nominal phase) :
	// *************************************************************************
	if (Reglages.Seuil < 500000L)
		// - R�glage seuil en dA (=> A*10). Ex. : val seuil = 5000 (1388h) pour 500A sur la DSM.
		// - PtsCanA10InH1 = 5500 points
		// - V_TRS_In = 630 pour un r�glage In = 630A
		valeur = (Reglages.Seuil * PtsCanA10InH1) / (10L * 10L * V_TRS_In);
	else
		valeur = ((Reglages.Seuil/10L) * PtsCanA10InH1) / (10L * V_TRS_In);


	// seuil = reglage converti en Pts Can carres
	ParamFonc[inst].SeuilDecl = valeur * valeur;

			
	// Pour la prise en compte de la cr�te filtr�e dans le cycle rapide :
	// ******************************************************************
	// Calcul de In en points CAN�.
	// Note : Unit� de CA_In en Amp�res (max : 6250 A).
	//        Unit� de PtsCanA10InH1 en points CAN (= 5004 points).
	PtsCanIn2 = diviseAuMieux((PtsCanA10InH1 * PtsCanA10InH1), 100L);

	// Seuil de d�gagement :
	// *********************
	// Prendre la plus petite valeur entre les 93,5% du seuil de d�clenchement
	// et les 1,5% de In.
	//
	// 1- Calcul du seuil d�gagement = 93,5% du seuil de d�clenchement =
	//    7/8 du seuil (carr�) de d�clenchement
	val = ParamFonc[inst].SeuilDecl;
	val = val - (val>>3);	// Seuil d�gagement en pts can^2

	// 2- Calcul du seuil d�gagement = 1,5% In
	DegaTalon = diviseAuMieux((PtsCanA10InH1 * 150000L), 100000000L);
	// Calcul (seuil D�gagement)� = (seuil D�clenchement - 1,5% In)�
	seuilDecl = valeur;
	
	if (seuilDecl > DegaTalon)
	{
		DegaTalon = seuilDecl - DegaTalon;
		DegaTalon *= DegaTalon;
	}
	else
		DegaTalon = 0;

	// Prendre la + petite valeur entre (93,5% seuil D�cl.)� et (Seuil D�cl. - 1,5% In)�
	ParamFonc[inst].SeuilDega = (long)((val < DegaTalon) ? val : DegaTalon);
	

	// Pour satisfaire aux exigences ENEL, ajout de 6 points CAN au calcul du seuil de
	// d�gagement pour un seuil de d�clenchement < 0,15 In :
	// (seuil D�gagement)� = (seuil D�clenchement + 6 points CAN)�

	val = diviseAuMieux((PtsCanA10InH1 * 150000L), 10000000L);
	val = val * val;  // val = 15% In

	if (ParamFonc[inst].SeuilDecl < val)
	{
		dval = isqrt((double)ParamFonc[inst].SeuilDega);
		dval = dval + 6;
		ParamFonc[inst].SeuilDega = (long)(dval * dval);
	}

    // facteur pour normalisation du courant de defaut	   
	// KX = 2^27 / Iscan2 (16 bits)
	// ***********************************************
    valeur = diviseAuMieux(PtsCanA10InH1 * Reglages.Seuil, (10L *10L * V_TRS_In));
	valeur *= valeur;									// Is^2 en points Can carres
	ParamFonc[inst].KX = diviseAuMieux(1L << 27, valeur);
	
  
	// Tempo de declenchement
	// **********************
	if (ParamFonc[inst].Courbe == 0)
	{
		valeur = (long)Reglages.Tempo * 1000L;
		ParamFonc[inst].Tempo = diviseAuMieux(valeur, 500L); // cycle de 3te (5 ms)
		if (ParamFonc[inst].Tempo > 0 )  ParamFonc[inst].Tempo--;
	}
	else
	{
	// 50 Hz, temps dependant (mini = 40000  pour reglage = 5 cs ) 
		valeur = (long)Reglages.Tempo * 40000L;
		ParamFonc[inst].Tempo = diviseAuMieux(valeur, 5L);
	}
	
	// initialiser les compteurs de tempo
	ParamFonc[inst].CptTempo  = ParamFonc[inst].Tempo;
}

												
/*****************************************************************************
*                                                                            *
* Fonction:	F51_Inv_Init						                                 *
*                                                                            *
* Appel   : F51_Inv_Init()                                                       *
*                                                                            *
* Entrees : 					                                             *
*                                                                            *
* Sorties : 					                                             *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle d'init de la protection F51_Inv                                        *
*                                                                            *
*****************************************************************************/
 

void F51_Inv_Init(char inst, long seuil, short tempo) {

  
  /*  mettre les reglages au format long */
	Reglages.On = 1;				// 	on
	Reglages.Courbe = 1;    // Tps inverse
	Reglages.Seuil = seuil;  // 100A
	Reglages.Tempo = tempo;  // ms
	// calculer les parametres fonctionnels 
	calculParam(inst);
}

/*****************************************************************************
*                                                                            *
* Fonction:	F51_Inv_Rapide  					                                 *
*                                                                            *
* Appel   : F51_Inv_Rapide()                                                     *
*                                                                            *
* Entrees : 					                                             *
*                                                                            *
* Sorties : 					                                             *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle rapide de la protection F51_Inv  = Max(I inverse)                      *
*                                                                            *
*****************************************************************************/
void F51_Inv_RapideExp(char inst, long V_mod2Imax) {

short indice;			// indice pour lire un delta dans une table de temps inverse
long delta;		// delta a appliquer au compteur de tempo dependante
int DefautPresent;	// indicateur vrai si defaut present

	
	// detecter le defaut
	if      (V_mod2Imax > ParamFonc[inst].SeuilDecl) DefautPresent = 1;
	else if (V_mod2Imax < ParamFonc[inst].SeuilDega) DefautPresent = 0;

	// Si la protection est inactive
	// -----------------------------
	if (ParamFonc[inst].On == 0) {

		// RAZ les sorties
		VA_ProtStatus[inst] &= ~(BIT_TEMPO | BIT_INST);

	// Sinon : la protection est active
	// -----------------------------
	} else {
		
		// si defaut present 
		// -----------------
		if ( DefautPresent ) {

			// monter le bit instantane 
			VA_ProtStatus[inst] |= BIT_INST;

			// si la tempo de declenchement n'est pas echue ...
			if ( ParamFonc[inst].CptTempo > 0) {

				// decompter la tempo ...  
				if (ParamFonc[inst].Courbe == 0) {					
					
					//... a temps constant
					ParamFonc[inst].CptTempo--;
						
				} else {
					static unsigned long g,gh,gl,r;					
					static short r2;
					static int octave, indice;
					// ... ou a temps dependant
					if      (V_mod2Imax > ParamFonc[inst].SeuilConst) 
						// indice = normalise(ParamFonc.SeuilConst, (short)ParamFonc.KX) ;
						g = ParamFonc[inst].SeuilConst;
					else
						// indice = normalise(mod2Imax, (short)ParamFonc.KX);
						g = V_mod2Imax;

					// calculer le produit Kx * grandeur/ 2^24
					gh = (g & 0xffff0000) >> 16;			// poids fort de grandeur g
					gl = (g & 0x0000ffff);					// poids faible de grandeur g
					r  = (gh * ParamFonc[inst].KX) + ( (gl * ParamFonc[inst].KX)>>16 );
					r  =  r>>8;

					// determiner le numero d'octave (15 a 4), et l'indice dans l'octave
					// ( la table comporte 12 octaves, reperee de 0 a 11 )
					r2 = r;
					octave = 11;
					while (r2 > 0) {
						octave--;
						r2 <<=1;
					} 
					indice = (r2>>8) & 127;

					// determiner l'indice dans la table complete, pour y lire le delta a appliquer
					indice = 128 * octave + indice;

					// lire le delta et l'appliquer au compteur
					delta = (unsigned)TabMaxIinv[ indice];
					ParamFonc[inst].CptTempo -= delta;

					// anticiper de 2 cycles , en provoquant le declenchement des que le compteur < delta 
					if (ParamFonc[inst].CptTempo < 2*delta ) ParamFonc[inst].CptTempo = 0 ;
				}  

			} // fin si tempo en cours

			// Si la tempo est echue, monter le bit temporise
			if (ParamFonc[inst].CptTempo==0) 
				VA_ProtStatus[inst] |= BIT_TEMPO;

						
		} else { // defaut absent 
				 // -------------
			// RAZer les bits instantane et temporise
			VA_ProtStatus[inst] &= ~(BIT_INST|BIT_TEMPO); 
			
			// rearmer le compteur
			ParamFonc[inst].CptTempo = ParamFonc[inst].Tempo;
		}

	} // Finsi protection active
}

/*****************************************************************************
*                                                                            *
* Fonction:	F51_Inv_Rapide  					                                 *
*                                                                            *
* Appel   : F51_Inv_Rapide()                                                     *
*                                                                            *
* Entrees : 					                                             *
*                                                                            *
* Sorties : 					                                             *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle rapide de la protection F51_Inv  = Max(I inverse)                      *
*                                                                            *
*****************************************************************************/
void F51_Inv_RapideDemo(char inst, long V_mod2Imax, char *status, int *SeuilDecl) {

short indice;			// indice pour lire un delta dans une table de temps inverse
long delta;		// delta a appliquer au compteur de tempo dependante
int DefautPresent;	// indicateur vrai si defaut present

	
	// detecter le defaut
	if      (V_mod2Imax > ParamFonc[inst].SeuilDecl) DefautPresent = 1;
	else if (V_mod2Imax < ParamFonc[inst].SeuilDega) DefautPresent = 0;

	*SeuilDecl = ParamFonc[inst].SeuilDecl;

	// Si la protection est inactive
	// -----------------------------
	if (ParamFonc[inst].On == 0) {

		// RAZ les sorties
		VA_ProtStatus[inst] &= ~(BIT_TEMPO | BIT_INST);

	// Sinon : la protection est active
	// -----------------------------
	} else {
		
		// si defaut present 
		// -----------------
		if ( DefautPresent ) {

			// monter le bit instantane 
			VA_ProtStatus[inst] |= BIT_INST;

			// si la tempo de declenchement n'est pas echue ...
			if ( ParamFonc[inst].CptTempo > 0) {

				// decompter la tempo ...  
				if (ParamFonc[inst].Courbe == 0) {					
					
					//... a temps constant
					ParamFonc[inst].CptTempo--;
						
				} else {
					static unsigned long g,gh,gl,r;					
					static short r2;
					static int octave, indice;
					// ... ou a temps dependant
					if      (V_mod2Imax > ParamFonc[inst].SeuilConst) 
						// indice = normalise(ParamFonc.SeuilConst, (short)ParamFonc.KX) ;
						g = ParamFonc[inst].SeuilConst;
					else
						// indice = normalise(mod2Imax, (short)ParamFonc.KX);
						g = V_mod2Imax;

					// calculer le produit Kx * grandeur/ 2^24
					gh = (g & 0xffff0000) >> 16;			// poids fort de grandeur g
					gl = (g & 0x0000ffff);					// poids faible de grandeur g
					r  = (gh * ParamFonc[inst].KX) + ( (gl * ParamFonc[inst].KX)>>16 );
					r  =  r>>8;

					// determiner le numero d'octave (15 a 4), et l'indice dans l'octave
					// ( la table comporte 12 octaves, reperee de 0 a 11 )
					r2 = r;
					octave = 11;
					while (r2 > 0) {
						octave--;
						r2 <<=1;
					} 
					indice = (r2>>8) & 127;

					// determiner l'indice dans la table complete, pour y lire le delta a appliquer
					indice = 128 * octave + indice;

					// lire le delta et l'appliquer au compteur
					delta = (unsigned)TabMaxIinv[ indice];
					ParamFonc[inst].CptTempo -= delta;

					// anticiper de 2 cycles , en provoquant le declenchement des que le compteur < delta 
					if (ParamFonc[inst].CptTempo < 2*delta ) ParamFonc[inst].CptTempo = 0 ;
				}  

			} // fin si tempo en cours

			// Si la tempo est echue, monter le bit temporise
			if (ParamFonc[inst].CptTempo==0) 
				VA_ProtStatus[inst] |= BIT_TEMPO;

						
		} else { // defaut absent 
				 // -------------
			// RAZer les bits instantane et temporise
			VA_ProtStatus[inst] &= ~(BIT_INST|BIT_TEMPO); 
			
			// rearmer le compteur
			ParamFonc[inst].CptTempo = ParamFonc[inst].Tempo;
		}

	} // Finsi protection active
	*status = VA_ProtStatus[inst];
}
