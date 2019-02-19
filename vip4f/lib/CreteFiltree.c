/********************************************************************************
*										*
*	PROJET : VIP4F				*
*										*
*	MODULE : CreteFiltree.c					*
*										*
*	AUTEUR : FR					*
*										*
*	CREATION : 06/03/2009
*										*
*********************************************************************************
*                                                                               *
*      DESCRIPTION       :CALCUL CRETE FILTREE									*
*
*********************************************************************************/


//*******************************************************************************
//
//			INCLUDES
//
//*******************************************************************************
#include "../include/typedef.h"
#include "../include/define.h"

#include "CreteFiltree.h"

void TRS_EchantillonCreteFiltree (short * Table_Echantillons, long *Ptr_Crete_voie)
{
	// remplacer Facteur de mise � l'�chelle : modifi� 0.8 *0.75 *2exp12 = 2457 
	// par
	// D_TRS_K2 1229	
	// Pour calcul crete filtree
	// 0.8*0.375*2exp12=1228.8


	short Nombre_echantillons;
	long Moyenne_echantillons,crete_filtree;

	short Signe_Crete,Signe_Moyenne;
	short Valeur_Abs_Crete,Vabs_Echantillons;
	
	crete_filtree		= 0;
	Nombre_echantillons = 0;
	Valeur_Abs_Crete	= 0;
	Moyenne_echantillons= 0;

	do
	{
		Vabs_Echantillons=Table_Echantillons[Nombre_echantillons];
		
		if (Vabs_Echantillons  < 0) Vabs_Echantillons = - Vabs_Echantillons; // Vabs_Echantillons = Valeur absolue de l'échantillon

		Moyenne_echantillons =	Moyenne_echantillons + Table_Echantillons[Nombre_echantillons]; // Moyenne_echantillons = Somme des echantillons

		if (Vabs_Echantillons >Valeur_Abs_Crete )
		{
			Valeur_Abs_Crete = Vabs_Echantillons;// on sauvegarde valeur absolue de l'echantillons dans
												 //	valeur crete max
			if ( (Table_Echantillons[Nombre_echantillons]) < 0)//on sauvegarde signe valeur crete max
			{
				Signe_Crete =0;//Valeur n�gative
			}
			else
			{
				Signe_Crete =1;//Valeur positive
			}
		}
		Nombre_echantillons++;
	}
	while( Nombre_echantillons < 12 );
	

	if ( Moyenne_echantillons < 0)//on sauvegarde signe valeur moyenne
	{
		Signe_Moyenne =0;//Valeur n�gative
	}
	else
	{
		Signe_Moyenne =1;//Valeur positive
	}

	if (Signe_Crete == Signe_Moyenne )
	{
		//modif prendre valeur absolue de la moyenne des echantillons
		crete_filtree = (( Valeur_Abs_Crete - abs(Moyenne_echantillons/8))* D_TRS_K2)>>12;

		if ( crete_filtree < 0 )
		{
			crete_filtree = 0;
		}
	}
	else
	{
		crete_filtree = ( Valeur_Abs_Crete * D_TRS_K2)>>12;

	}

	*Ptr_Crete_voie = crete_filtree*crete_filtree;



}

void TRS_EchantillonCreteFiltree2 (int * Table_Echantillons, long *Ptr_Crete_voie)
{
	
	// remplacer Facteur de mise � l'�chelle : modifi� 0.8 *0.75 *2exp12 = 2457 
	// par
	// D_TRS_K2 1229	
	// Pour calcul crete filtree
	// 0.8*0.375*2exp12=1228.8


	short Nombre_echantillons;
	long Moyenne_echantillons,crete_filtree;
	int Res;

	short Signe_Crete;
	short Valeur_Abs_Crete,Vabs_Echantillons;
	
	crete_filtree		= 0;
	Nombre_echantillons = 0;
	Valeur_Abs_Crete	= 0;
	Moyenne_echantillons= 0;

	do
	{
		Vabs_Echantillons=Table_Echantillons[Nombre_echantillons];
		
		if (Vabs_Echantillons  < 0) Vabs_Echantillons = - Vabs_Echantillons; // Vabs_Echantillons = Valeur absolue de l'échantillon

		Moyenne_echantillons =	Moyenne_echantillons + Table_Echantillons[Nombre_echantillons]; // Moyenne_echantillons = Somme des echantillons

		if (Vabs_Echantillons >Valeur_Abs_Crete )
		{
			Valeur_Abs_Crete = Vabs_Echantillons;// on sauvegarde valeur absolue de l'echantillons dans
												 //	valeur crete max
			if ( (Table_Echantillons[Nombre_echantillons]) < 0)//on sauvegarde signe valeur crete max
			{
				Signe_Crete =0;//Valeur n�gative
			}
			else
			{
				Signe_Crete =1;//Valeur positive
			}
		}
		Nombre_echantillons++;
	}
	while( Nombre_echantillons < 12 );
	
	// If average and crest are not the same sign we set average to zero
	if ( ( Table_Echantillons[Nombre_echantillons] >= 0 ) && ( Moyenne_echantillons < 0 ) ) Moyenne_echantillons = 0;
	else if ( ( Table_Echantillons[Nombre_echantillons] < 0 ) && ( Moyenne_echantillons > 0 ) ) Moyenne_echantillons = 0;
	// filtred crest = (|crest| - 1.5 * |average|) * 0.8
	// filtred crest = ( |crest| - 3 / 2 * |Sum|/12) * 8 / 10
	// filtred crest = ( 8*|crest| - 8 * 3 / 2 / 12 * |Sum| ) / 10
	// filtred crest = ( 8*|crest| - |Sum| ) / 10
	
	if (Moyenne_echantillons < 0) Moyenne_echantillons = -Moyenne_echantillons;
	Res = (Valeur_Abs_Crete<<3) - Moyenne_echantillons;   // Cf AbsCrest on 21 bits
	if ( Res < 0 ) *Ptr_Crete_voie  = 0;
        else *Ptr_Crete_voie = ( Res + 5 ) / 10;
	
	*Ptr_Crete_voie = (*Ptr_Crete_voie) * (*Ptr_Crete_voie);
}
