#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/typedef.h"
#include "include/define.h"
#include "include/shared.h"

/*=====================================================================================*/
/*-------------------------------  AgPrepRMS  -----------------------------------------*/
/*=====================================================================================*/
void agCumulRMS(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  long indexEchan, indexVoies;
  int cmpt;

  int id = get_cpuid();

  while (1) {
    while (id != owner);  

    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      // MAJ des cumuls echantillons
      vip4f->V_TRS_CumulRms[indexVoies] += (U_LONG)(vip4f->DataBufferI[indexVoies]);
      // MAJ des cumuls carre echantillons
      vip4f->V_TRS_CumulRms2[indexVoies] += (U_LONG)((U_LONG)vip4f->DataBufferI[indexVoies]*(U_LONG)vip4f->DataBufferI[indexVoies]);
    }
    
    cmpt++;
    
    if (cmpt>D_TRS_NB_ECH_RMS) {
      cmpt = 0;
      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
	// RAZ des cumuls échantillons
	vip4f->V_TRS_CumulRms[indexVoies] = 0;
	// RAZ des cumuls carrés échantillons	  
	vip4f->V_TRS_CumulRms2[indexVoies] = 0;				
      }
    }
    
    owner = 0;
  }
}

/*=====================================================================================*/
/*-------------------------------  AgRMS  -----------------------------------------*/
/*=====================================================================================*/
void agRMS(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  long indexEchan, indexVoies;
  long val;
  int cmpt = 0;

  int id = get_cpuid();

  while (1) {
    while (id != owner);
    
    cmpt++;
    /* Voies I1, I2 et I3 */
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      vip4f->RMS [indexVoies] = CourantRMS_dA(vip4f->V_TRS_CumulRms2[indexVoies],
					      vip4f->V_TRS_CumulRms[indexVoies]);  
      val = (unsigned long)(isqrt(vip4f->VS_Mod2[indexVoies]));
      vip4f->I[indexVoies] = ((int)val * 10)/55;
    }
    
    /* Voie Io */
    val = (unsigned long)(isqrt(vip4f->VS_Mod2[D_ACQ_VOIE_Io]));
    vip4f->I[D_ACQ_VOIE_Io] = ((int)val * 10)/55;
  }
}

/*=====================================================================================*/
/*-------------------------------  AgMoy  -----------------------------------------*/
/*=====================================================================================*/
void agMoy(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  long indexVoies, indexEchan;

  int id = get_cpuid();

  // TODO: CONSULT !!!
  // TODO: MORE ITEMS ARE NEEDED!!
  //  AgARGA : D_TRS_NB_ECH_FILTRE $DataBufferI;

  /* Initialisation des valeurs cummulées */
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
    vip4f->V_TRS_CumulFiltre [indexVoies] = 0;				
  }

  while (1) {
    while (id != owner);
    
    /* Cumule des valeurs par voies.     */
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
      for (indexEchan = 0; indexEchan < D_TRS_NB_ECH_FILTRE; ++indexEchan) {      
	// MAJ des cumuls échantillons
	// TODO: MORE ITEMS ARE NEEDED!
	//V_TRS_CumulFiltre [indexVoies] += AgARGA`indexEchan$DataBufferI [indexVoies];
	vip4f->V_TRS_CumulFiltre [indexVoies] += vip4f->DataBufferI [indexVoies];      
      }
      // Add 1 before truncature
      // 0/3 => truncate to 0, 1/3 => truncate to 0, and 2/3 => truncate to 1
      if (vip4f->V_TRS_CumulFiltre [indexVoies] >= 0) vip4f->V_TRS_CumulFiltre [indexVoies] += 1;
      else vip4f->V_TRS_CumulFiltre [indexVoies] -= 1;
    }
    
    // Ecriture des moyennes sur 3 
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {    
      vip4f->V_TRS_CumulFiltre [indexVoies] = (vip4f->V_TRS_CumulFiltre [indexVoies])/D_TRS_NB_ECH_FILTRE;
    }
  }
}


/*=====================================================================================*/
/*-------------------------------  AgCrete  -----------------------------------------*/
/*=====================================================================================*/
void agCrete(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  long indexVoies;

  int id = get_cpuid();  

  //Initialisation pour detection crete
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    vip4f->V_DETC[indexVoies].Maxi_ech = 0;
    vip4f->V_DETC[indexVoies].Mini_ech = 0;
    vip4f->V_DETC[indexVoies].Np = 0;
    vip4f->V_DETC[indexVoies].Nn = 0;
    vip4f->V_DETC[indexVoies].LastS = 0;
  }

  while (1) {
    while (id != owner);    
    
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      TRS_DetectionCrete (vip4f->DataBufferI [indexVoies], &(vip4f->V_DETC[indexVoies]));
    }
  }
}


/*=====================================================================================*/
/*-------------------------------  AgTRS  -----------------------------------------*/
/*=====================================================================================*/
void agTRS(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;

  int id = get_cpuid();    
  
  int i,j;
  
  long indexEchan, indexVoies;
  unsigned long long temps;
  
  /* Voies I1, I2, I3, Io */
  int echantillon[D_ACQ_NB_VOIES][D_TRS_NB_BUF_I];
  
  /* Voies I1, I2, I3 et Io */
  S_TRS_FXFY	V_TRS_H1 [D_ACQ_NB_VOIES];
  
  /* Uniquement S1, S2 et S3 */
  long    VS_Mod2_S [D_ACQ_NB_VOIES-1];
  
  int cmpt;
  int index_source;

  // TODO: CONSULT !!!
  // TODO: more items are needed
  //AgMoy  : D_TRS_NB_BUF_I $V_TRS_CumulFiltre;

  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
    V_TRS_H1[indexVoies].ComposanteFx		= 0;
    V_TRS_H1[indexVoies].ComposanteFy		= 0;
    V_TRS_H1[indexVoies].ComposanteFx_old	= 0;
    V_TRS_H1[indexVoies].ComposanteFy_old	= 0;
    vip4f->VS_Mod2[indexVoies]			= 0;
  }
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    vip4f->VS_Mod2Crete[indexVoies] = 0;
  }

  while (1) {
    while (id != owner);    
    
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
      for (indexEchan = 0; indexEchan < D_TRS_NB_BUF_I; ++indexEchan) {
	// TODO: more items are needed
	//echantillon[indexVoies][indexEchan] = AgMoy`indexEchan $V_TRS_CumulFiltre [indexVoies];      
	echantillon[indexVoies][indexEchan] = vip4f->V_TRS_CumulFiltre [indexVoies];
      }
    }
    
    /* Mesure de phaseur (voies I1, I2, I3 et Io),crete filtree */
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
      TRS_EchantillonSinCosH12(&echantillon[indexVoies], &(vip4f->VS_Mod2[indexVoies]), indexVoies);
    }
    
    /* Uniquement pour voies I1, I2 et I3 */
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {    
      TRS_EchantillonCreteFiltree2(&echantillon[indexVoies],&(vip4f->VS_Mod2Crete[indexVoies]));
    }
    
    /* Calculer la grandeur caracteristique */
    /* max des 3 courants phase issus du SINCOS */
    /* et des 3 courants phases issus de la detection de crete */
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      VS_Mod2_S[indexVoies] = (long)((long)vip4f->V_DETC[indexVoies].S *
				     (long)vip4f->V_DETC[indexVoies].S);
    }
    
    // Prends le Max par rapport à la valeur de H1.
    vip4f->V_mod2Imax = vip4f->VS_Mod2 [D_ACQ_VOIE_I1];
    
    if (vip4f->VS_Mod2[D_ACQ_VOIE_I2] > vip4f->V_mod2Imax)
      vip4f->V_mod2Imax = vip4f->VS_Mod2[D_ACQ_VOIE_I2];
    if (vip4f->VS_Mod2[D_ACQ_VOIE_I3] > vip4f->V_mod2Imax)
      vip4f->V_mod2Imax = vip4f->VS_Mod2[D_ACQ_VOIE_I3];
    
    /* Prise en compte de la valeur crête filtrée dés qu'un phaseur */
    /* dépasse In */
    if (vip4f->V_mod2Imax > calculeIn2())
      {
	if (vip4f->VS_Mod2Crete[D_ACQ_VOIE_I1] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = vip4f->VS_Mod2Crete[D_ACQ_VOIE_I1];
	if (vip4f->VS_Mod2Crete[D_ACQ_VOIE_I2] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = vip4f->VS_Mod2Crete[D_ACQ_VOIE_I2];
	if (vip4f->VS_Mod2Crete[D_ACQ_VOIE_I3] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = vip4f->VS_Mod2Crete[D_ACQ_VOIE_I3];
      }
    
    /* Prise en compte de la valeur crête non filtrée dès qu'un phaseur */
    /* dépasse 10In */
    if (vip4f->V_mod2Imax > (10 * calculeIn2()))
      {
	if (VS_Mod2_S[D_ACQ_VOIE_I1] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = VS_Mod2_S[D_ACQ_VOIE_I2];
	if (VS_Mod2_S[D_ACQ_VOIE_I2] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = VS_Mod2_S[D_ACQ_VOIE_I2];
	if (VS_Mod2_S[D_ACQ_VOIE_I3] > vip4f->V_mod2Imax)
	  vip4f->V_mod2Imax = VS_Mod2_S[D_ACQ_VOIE_I3];
      }
  }
  
}
