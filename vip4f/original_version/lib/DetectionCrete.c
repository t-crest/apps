/********************************************************************************
*										*
*	PROJET : VIP4F				*
*										*
*	MODULE : DetectionCrete.c					*
*										*
*	AUTEUR : FR					*
*										*
*	CREATION : 26/03/2009
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

void TRS_DetectionCrete (int ech, S_TRS_DETC *ptr2)
{
  long Valeur;
  short E;
	short Tau;

  // comparaison au maxi
  //====================
		
	if (ech > ptr2->Maxi_ech)
  {
    ptr2->Maxi_ech = ech;
    ptr2->Np = 0;
  }
  else  // ech <= Maxi_ech
  {
    ptr2->Np++;
    if (ptr2->Np < 40)
    {
      ptr2->Maxi_ech = ptr2->Maxi_ech - (ptr2->Maxi_ech >> 7); // div par 128
    }
    else  // Np >= 40
    {
      ptr2->Maxi_ech = ptr2->Maxi_ech >>1;
      ptr2->Np = 50;
    }
  }//Fin si ech > Maxi_ech

  // comparaison au mini
  //====================
 	if (ech < ptr2->Mini_ech)
  {
    ptr2->Mini_ech = ech;
    ptr2->Nn = 0;
  }
  else  // ech >= Mini_ech
  {
    ptr2->Nn++;
    if (ptr2->Nn < 40)
    {
      ptr2->Mini_ech = ptr2->Mini_ech - (ptr2->Mini_ech >> 7); // div par 128
    }
    else  // Nn >= 40
    {
      ptr2->Mini_ech = ptr2->Mini_ech >>1;
      ptr2->Nn = 50;
    }
  } //Fin si ech < Mini_ech

  // Calcul de S
  //============
  Valeur = 144*(long)(ptr2->Maxi_ech - ptr2->Mini_ech);
  E = (short)(Valeur >> 8);  // div par 256
  
  if ((ptr2->Np >= 40) || (ptr2->Nn >= 40))
    Tau = 1; // pour div par 2
  else
    Tau =  5; // pour div par 32

  ptr2->S = ptr2->LastS + ((E - ptr2->LastS) >> Tau);
  ptr2->LastS = ptr2->S;
}


