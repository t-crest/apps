/*****************************************************************************
*                                                                            *
* Projet  :	VIP4F						                                                 *
*                                                                            *
* Module  :	F51 = MAX(I)							version : 00                           *
*                                                                            *
* Auteur  :   F. RAGOT                                                       *
*                                                  *
* Date de creation : 27/02/2009                                              *
*                                                                            *
* Date de modif    :                                                         *
*                                                                            *
* Dossier de specification :                                                 *
* Dossier de conception    :                                                 *
*                                                                            *
* Description : Protection a maximum de courant phase.                       *
*                                                                            *
* ************************************************************************** *
* ************************************************************************** *
*                                                                            *
* Modifications :                                                            *
*                                                                            *
*****************************************************************************/ 

#include "../include/typedef.h"
#include "../include/define.h"

#include "Bit.h"
#include "Pts.h"

// constantes propres a la fonction 

#define NB_REGL			4		// nombre de reglages par exemplaire et par jeu 

#define	RGL_ON_OFF		0		// indice de reglage pour les differents reglages 
#define	RGL_SEUIL		1
#define	RGL_TEMPO		2

// stucture des reglages  
typedef struct strucReglages {
	char  On;
	long  Seuil;
	short Tempo;
} strucReglages;

static strucReglages Reglages;		


// stucture de donnees pour les reglages operationnels 
typedef struct strucParamFonc {
	/*------------ transposition des reglages E2Prom --------*/
	char  On;
	long  SeuilDecl;
	long  SeuilDega;
	long  Tempo;
   /*------------- variables de travail ------*/
	long  CptTempo;
	long  SeuilConst;	 
} strucParamFonc;

static strucParamFonc ParamFonc[5];

unsigned long  PtsCanIn2;// Pour prise en compte crête filtrée.
unsigned long  In2;// Pour prise en compte crête filtrée.

char  VA_ProtStatus[5];	// sorties des protections #############à dimensionner

/*****************************************************************************
*                                                                            *
* Fonction: calculParam   				                                     *
*                                                                            *
* Appel   : calculParam()	                               		             *
*                                                                            *
* Entrees : 					                                             *
* 	inst = instance de la protection (0) 		                             *
*                                                                            *
* Sorties : 					                                             *
* 	variables globales MaxI								                     *
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
   
   	
	// Seuil de déclenchement en points CAN	(V_TRS_In = courant nominal phase) :
	// *************************************************************************
	if (Reglages.Seuil < 500000L)
		// - Réglage seuil en dA (=> A*10). Ex. : val seuil = 5000 (1388h) pour 500A sur la DSM.
		// - PtsCanA10InH1 = 1943 points
		// - V_TRS_In = 630 pour un réglage In = 630A
		valeur = (Reglages.Seuil * PtsCanA10InH1) / (10L * 10L * V_TRS_In);
	else
		valeur = ((Reglages.Seuil/10L) * PtsCanA10InH1) / (10L * V_TRS_In);


	// seuil = reglage converti en Pts Can carres
	ParamFonc[inst].SeuilDecl = valeur * valeur;

			
	// Pour la prise en compte de la crête filtrée dans le cycle rapide :
	// ******************************************************************
	// Calcul de In en points CAN².
	// Note : Unité de CA_In en Ampères (max : 6250 A).
	//        Unité de PtsCanA10InH1 en points CAN (= 5004 points).
	PtsCanIn2 = diviseAuMieux((PtsCanA10InH1 * PtsCanA10InH1), 100L);

	// Seuil de dégagement :
	// *********************
	// Prendre la plus petite valeur entre les 93,5% du seuil de déclenchement
	// et les 1,5% de In.
	//
	// 1- Calcul du seuil dégagement = 93,5% du seuil de déclenchement =
	//    7/8 du seuil (carré) de déclenchement
	val = ParamFonc[inst].SeuilDecl;
	val = val - (val>>3);	// Seuil dégagement en pts can^2

	// 2- Calcul du seuil dégagement = 1,5% In
	DegaTalon = diviseAuMieux((PtsCanA10InH1 * 150000L), 100000000L);
	// Calcul (seuil Dégagement) = (seuil Déclenchement - 1,5% In)
	seuilDecl = valeur;
	
	if (seuilDecl > DegaTalon)
	{
		DegaTalon = seuilDecl - DegaTalon;
		DegaTalon *= DegaTalon;
	}
	else
		DegaTalon = 0;

	// Prendre la + petite valeur entre (93,5% seuil Décl.) et (Seuil Dcél. - 1,5% In)
	ParamFonc[inst].SeuilDega = (long)((val < DegaTalon) ? val : DegaTalon);
	

	// Pour satisfaire aux exigences ENEL, ajout de 6 points CAN au calcul du seuil de
	// dégagement pour un seuil de déclenchement < 0,15 In :
	// (seuil Dégagement) = (seuil Déclenchement + 6 points CAN)
	
	val = diviseAuMieux((PtsCanA10InH1 * 150000L), 10000000L);
	val = val * val;  // val = 15% In

	if (ParamFonc[inst].SeuilDecl < val)
	{
		dval = isqrt((double)ParamFonc[inst].SeuilDega);
		dval = dval + 6;
		ParamFonc[inst].SeuilDega = (long)(dval * dval);
	}


	// Tempo de declenchement
    // **********************
	valeur = (long)Reglages.Tempo * 1000L;
	ParamFonc[inst].Tempo = diviseAuMieux(valeur, 500L);

	// initialiser les compteurs de tempo
	ParamFonc[inst].CptTempo  = ParamFonc[inst].Tempo;
}


static void calculParam2(char inst) {

	long valeur;
	unsigned long	val, DegaTalon, seuilDecl;
	float	 dval;


	// ON-OFF
	ParamFonc[inst].On = (char)(Reglages.On);


	// Seuil de déclenchement en ampère	(V_TRS_In = courant nominal phase) :
	// *************************************************************************
	valeur = Reglages.Seuil;
	
	// seuil = reglage converti en Pts Can carres
	ParamFonc[inst].SeuilDecl = valeur * valeur;

	// Pour la prise en compte de la crête filtrée dans le cycle rapide :
	// ******************************************************************
	// Calcul de In
	// Note : Unité de In2 en Ampères carré.
	In2 = V_TRS_In * V_TRS_In;

	// Seuil de dégagement :
	// *********************
	// Prendre la plus petite valeur entre les 93,5% du seuil de déclenchement
	// et les 1,5% de In.
	//
	// 1- Calcul du seuil dégagement = 93,5% du seuil de déclenchement =
	//    7/8 du seuil (carré) de déclenchement
	val = ParamFonc[inst].SeuilDecl;
	val = val - (val>>3);	// Seuil dégagement en ampère carré

	// 2- Calcul du seuil dégagement = 1,5% In
	DegaTalon = diviseAuMieux((V_TRS_In * 150000L), 1000000L);
	// Calcul (seuil Dégagement) = (seuil Déclenchement - 1,5% In)
	seuilDecl = valeur;
	
	if (seuilDecl > DegaTalon)
	{
		DegaTalon = seuilDecl - DegaTalon;
		DegaTalon *= DegaTalon;
	}
	else
		DegaTalon = 0;

	// Prendre la + petite valeur entre (93,5% seuil Décl.) et (Seuil Dcél. - 1,5% In)
	ParamFonc[inst].SeuilDega = (long)((val < DegaTalon) ? val : DegaTalon);
	

//	// Pour satisfaire aux exigences ENEL, ajout de 6 points CAN au calcul du seuil de
//	// dégagement pour un seuil de déclenchement < 0,15 In :
//	// (seuil Dégagement) = (seuil Déclenchement + 6 points CAN)
//	
//	val = diviseAuMieux((PtsCanA10InH1 * 150000L), 10000000L);
//	val = val * val;  // val = 15% In
//
//	if (ParamFonc[inst].SeuilDecl < val)
//	{
//		dval = sqrt((double)ParamFonc[inst].SeuilDega);
//		dval = dval + 6;
//		ParamFonc[inst].SeuilDega = (long)(dval * dval);
//	}

	// Tempo de declenchement
    // **********************
//	valeur = (long)Reglages.Tempo * 1000L;
//	ParamFonc[inst].Tempo = diviseAuMieux(valeur, 500L);
	ParamFonc[inst].Tempo = (long)Reglages.Tempo;

	// initialiser les compteurs de tempo
	ParamFonc[inst].CptTempo  = ParamFonc[inst].Tempo + 1;
	
//	printf("Tempo = %d\n", ParamFonc[inst].CptTempo);
//	printf("Seuil Decl = %d\n", ParamFonc[inst].SeuilDecl);
//	printf("Seuil Dega = %d\n", ParamFonc[inst].SeuilDega);
}

/*****************************************************************************
*                                                                            *
* Fonction:	F51_Init						                                 *
*                                                                            *
* Appel   : F51_Init()                                                       *
*                                                                            *
* Entrees : 					                                             *
*                                                                            *
* Sorties : 					                                             *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle d'init de la protection F51                                        *
*                                                                            *
*****************************************************************************/
 

void F51_Init( char inst, long seuil, short tempo) {

  /*  mettre les reglages au format long */
	Reglages.On = 1;				// 	on
	Reglages.Seuil = seuil;  // 100A
	Reglages.Tempo = tempo;    // ms
	
	// calculer les parametres fonctionnels 
	calculParam2(inst);
}

/*****************************************************************************
*                                                                            *
* Fonction:	F51_Rapide  					                                           *
*                                                                            *
* Appel   : F51_Rapide()                                                     *
*                                                                            *
* Entrees : 					                                                       *
*                                                                            *
* Sorties : 					                                                       *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle rapide de la protection F51  = MaxI                                *
*                                                                            *
*****************************************************************************/
void F51_RapideExp(char inst, long V_mod2Imax) 
{
	int DefautPresent;	  // indicateur vrai si defaut present

  
	// detecter le defaut
	if      (V_mod2Imax > ParamFonc[inst].SeuilDecl) DefautPresent = 1;
	else if (V_mod2Imax < ParamFonc[inst].SeuilDega) DefautPresent = 0;

	// Si la protection est inactive
	// -----------------------------
	if (ParamFonc[inst].On == 0) 
	{
		// RAZ les sorties
		VA_ProtStatus[inst] &= ~(BIT_TEMPO | BIT_INST);

	// Sinon : la protection est active
	// -----------------------------
	} 
	else 
	{
		// si defaut present 
		// -----------------
		if ( DefautPresent ) 
		{
			// monter le bit instantane 
			VA_ProtStatus[inst] |= BIT_INST;

			// si la tempo de declenchement n'est pas echue ...
			if ( ParamFonc[inst].CptTempo > 0)
			{
				// decompter la tempo
				ParamFonc[inst].CptTempo--;

			} // fin si tempo en cours
			
			// Si la tempo est echue, monter le bit temporise
			if (ParamFonc[inst].CptTempo==0) 
			{	
				VA_ProtStatus[inst] |= BIT_TEMPO;
			}
		} 
		// si defaut absent 
		// -----------------
		else  
		{ 
			// RAZer les bits instantane et temporise
			VA_ProtStatus[inst] &= ~(BIT_INST|BIT_TEMPO); 
			
			// rearmer le compteur
			ParamFonc[inst].CptTempo = ParamFonc[inst].Tempo + 1;
		}

	} // Fin si protection active
}

/*****************************************************************************
*                                                                            *
* Fonction:	F51_Rapide  					                                           *
*                                                                            *
* Appel   : F51_Rapide()                                                     *
*                                                                            *
* Entrees : 					                                                       *
*                                                                            *
* Sorties : 					                                                       *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Cycle rapide de la protection F51  = MaxI                                *
*                                                                            *
*****************************************************************************/
void F51_RapideDemo(char inst, long V_mod2Imax, char *status, int *SeuilDecl) 
{
	int DefautPresent;	  // indicateur vrai si defaut present

  
	// detecter le defaut
	if      (V_mod2Imax > ParamFonc[inst].SeuilDecl) DefautPresent = 1;
	else if (V_mod2Imax < ParamFonc[inst].SeuilDega) DefautPresent = 0;

	*SeuilDecl = ParamFonc[inst].SeuilDecl;

	// Si la protection est inactive
	// -----------------------------
	if (ParamFonc[inst].On == 0) 
	{
		// RAZ les sorties
		VA_ProtStatus[inst] &= ~(BIT_TEMPO | BIT_INST);

	// Sinon : la protection est active
	// -----------------------------
	} 
	else 
	{
		// si defaut present 
		// -----------------
		if ( DefautPresent ) 
		{
			// monter le bit instantane 
			VA_ProtStatus[inst] |= BIT_INST;

			// si la tempo de declenchement n'est pas echue ...
			if ( ParamFonc[inst].CptTempo > 0)
			{
				// decompter la tempo
				ParamFonc[inst].CptTempo--;

			} // fin si tempo en cours
			
			// Si la tempo est echue, monter le bit temporise
			if (ParamFonc[inst].CptTempo==0) 
			{	
				VA_ProtStatus[inst] |= BIT_TEMPO;
			}
		} 
		// si defaut absent 
		// -----------------
		else  
		{ 
			// RAZer les bits instantane et temporise
			VA_ProtStatus[inst] &= ~(BIT_INST|BIT_TEMPO); 
			
			// rearmer le compteur
			ParamFonc[inst].CptTempo = ParamFonc[inst].Tempo + 1;
		}

	} // Fin si protection active
	*status = VA_ProtStatus[inst];
}
