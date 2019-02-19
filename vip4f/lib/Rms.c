#include "../include/typedef.h"
#include "../include/define.h"

// TEST
int ech = 0;
long Tab_Ech[20];

/***********************************************************
*                                                           
* Fonction: DiviseAuMieux 				                          
*                                                          
* Appel   : q = DiviseAuMieux(a, b)                       
*                                                          
* Entrees : 					                                     
* 	a = numerateur 		                        						
* 	b = denominateur	                        						
*                                                         
* Sorties : 					                                     
* 	q = a/b arrondi										                     
*                                                          
* ************************************************************************** *
*                                                                            *
* Description : division 32/32 => 32 avec arrondi a l'entier le + proche     *
*                                                                            *
*****************************************************************************/

long DiviseAuMieux(long A, long B) {

long Q, R;		// quotient et reste

	Q = A / B;			// quotient				
	R = A - B*Q;		// reste
	if ( R >= B/2) Q++;	// arrondir a l'entier le + proche
	return(Q);
}
  
/*****************************************************************************
*                                                                            *
* Fonction:	RacineLong						                                 *
*                                                                            *
* Appel   : racine = RacineLong(valeur)                                      *
*                                                                            *
* Entrees : valeur =  valeur dont on cherche la racine                       *
*  			                                                                 *
* Sorties : racine = racine carree de "valeur"                               *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Recherche par dichotomie une valeur l telle que l*l = valeur             *
*                                                                            *
*****************************************************************************/
long RacineLong(long valeur) {

long l1, l2;		// encadrement de la racine carree recherchee
long l;				// valeur approchee de la racine carree	  l1 < l < l2
long lcarre;		// l*l

	l1 = 0L;						// encadrement initial 						
	l2 = 65535L;
	l = (l1+l2) >> 1;				// taper au milieu de l'encadrement

	do {
		lcarre = l*l;
		if ( lcarre >= valeur ) l2 = l;		// reduire l'encadrement par le haut
		if ( lcarre <= valeur ) l1 = l;		// reduire l'encadrement par le bas

		l = (l1+l2) >> 1;					// taper au milieu du nouvel encadrement
	} while ( (l2-l1) > 1);					// arret si encadrement reduit a 1 seul digit

	return(l1);
}	



/*****************************************************************************
*                                                                            *
* Fonction:	CourantRMS_dA						                                         *
*                                                                            *
* Appel   : iRms01 = CourantRMS_dA( long long cumRMS2, long cumRMS, &iRms)   *
*                                                                            *
* Entrees : cumRMS2 =  cumul des Irms^2 (64 bits)                            *
*  			cumRMS  =  cumul des Irms   (32 bits)                                *
*  			                                                                     *
* Sorties : iRms01 = courant RMS en dA                                       *
*           iRms   = courant RMS en  A                                       *
*                                                                            *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description :                                                              *
*   Calcul un courant RMS a partir du cumul des echantillons et du cumul     *
*   des echantillons^2.                                                      *
*   ( utilise le nombre d'echantillons cumules (TRS_NbEchRMS)                *
*****************************************************************************/
long CourantRMS_dA( unsigned long cumRMS2, long cumRMS) {
long offset;
long valeurPtsCan2;
long petit;
long diviseur;
unsigned long valeur;
unsigned long valeur_calcul;			// valeur intermediaire pour le calcul de l'offset

	// rendre le cumul de Irms positif
	if (cumRMS < 0) cumRMS = 0 - cumRMS;

	// calcul de la moyenne de l'offset² (offset² = cumRMS² / V_TRS_NbEchRms²)
	valeur_calcul = cumRMS;
	valeur_calcul *= valeur_calcul;
	valeur_calcul /= (D_TRS_NB_ECH_RMS * D_TRS_NB_ECH_RMS);
	offset = valeur_calcul;


	// calculer la moyenne du Irms^2
	valeur_calcul = cumRMS2 / D_TRS_NB_ECH_RMS;

	// corriger la valeur du Irms^2	de la valeur de l'offset^2
	valeur_calcul -= offset;

	// calculer la racine	
	valeur = (unsigned long)(isqrt(valeur_calcul));
	
	//valeur_calcul = (unsigned long)(sqrt(cumRMS2));
	//valeur_calcul = valeur_calcul / 42;  // 42 = rac(1800)
	
	valeur = (valeur * V_TRS_In * 10L);
        valeur = valeur	/ 6875L;  // 6600 = 1.25 * PtsCanA10InH1 ; //en A
	
	Tab_Ech[ech] = (long)valeur;
	if (ech == 19)  
	ech = 0;
	else ech++;

	// renvoyer les valeurs calculees
	return((long)valeur);												 // Ix en A
}

