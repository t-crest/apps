#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/typedef.h"
#include "include/define.h"
#include "include/shared.h"


/*=====================================================================================*/
/*-------------------------------  AgRMS  -----------------------------------------*/
/*=====================================================================================*/
void agRMS(void *arg)
{
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;

  /* Uniquement I1, I2, I3 */  
  /* cumul echantillons pour offset RMS */
  long V_TRS_CumulRms[D_ACQ_NB_VOIES-1];
  /* Uniquement I1, I2, I3 */  
  /* cumul carre echantillons pour calcul RMS */
  U_LONG V_TRS_CumulRms2[D_ACQ_NB_VOIES-1];  
  
  long indexEchan, indexVoies;
  long val;
  int cmpt = 0;

  int id = get_cpuid();

  // Initialization part
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    vip4f->V_TRS_CumulRms[indexVoies] = 0;
    vip4f->V_TRS_CumulRms2[indexVoies] = 0;
  }

  // Periodic part  
  while (1) {
    while (id != owner);

    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      // MAJ des cumuls echantillons
      vip4f->V_TRS_CumulRms[indexVoies] += (U_LONG)(vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][indexVoies]);
      // MAJ des cumuls carre echantillons
      vip4f->V_TRS_CumulRms2[indexVoies] += (U_LONG)((U_LONG)vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][indexVoies]*
					      (U_LONG)vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][indexVoies]);
    }
    
    if (cmpt == D_TRS_NB_ECH_RMS) {/* Voies I1, I2 et I3 */
      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
	vip4f->RMS [indexVoies] = CourantRMS_dA(vip4f->V_TRS_CumulRms2[indexVoies],
						vip4f->V_TRS_CumulRms[indexVoies]);  
	val = (unsigned long)(isqrt(vip4f->VS_Mod2[indexVoies]));
	vip4f->I[indexVoies] = ((int)val * 10)/55;
      }
      
      /* Voie Io */
      val = (unsigned long)(isqrt(vip4f->VS_Mod2[D_ACQ_VOIE_Io]));
      vip4f->I[D_ACQ_VOIE_Io] = ((int)val * 10)/55;

      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
	// RAZ des cumuls échantillons
	vip4f->V_TRS_CumulRms[indexVoies] = 0;
	// RAZ des cumuls carrés échantillons	  
	vip4f->V_TRS_CumulRms2[indexVoies] = 0;				    
      }

      cmpt = 0;
    }
    cmpt++;    

    // Going back to agARGA
    owner = 0;
  }
}


/*=====================================================================================*/
/*-------------------------------  AgCreteMoyTRS  -------------------------------------*/
/*=====================================================================================*/
void init_crete(struct vip4f_t *vip4f)
{
  //Initialisation pour detection crete
  for (long indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    vip4f->V_DETC[indexVoies].Maxi_ech = 0;
    vip4f->V_DETC[indexVoies].Mini_ech = 0;
    vip4f->V_DETC[indexVoies].Np = 0;
    vip4f->V_DETC[indexVoies].Nn = 0;
    vip4f->V_DETC[indexVoies].LastS = 0;
  }
}

void init_moy(struct vip4f_t *vip4f)
{
  /* Initialisation des valeurs cummulées */
  for (long indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
    for (long indexEchan = 0; indexEchan < D_TRS_NB_BUF_I; ++indexEchan) {
      vip4f->V_TRS_CumulFiltre[indexEchan][indexVoies] = 0;
    }
  }
  vip4f->counter_trs = 0;
}

void agMoy(struct vip4f_t *vip4f)
{
  long indexVoies, indexEchan;
  
  /* Cumule des valeurs par voies.     */
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
    for (indexEchan = 0; indexEchan < D_TRS_NB_ECH_FILTRE; ++indexEchan) {      
      // MAJ des cumuls échantillons
      vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies] += vip4f->DataBufferI[indexEchan][indexVoies];      
    }
    // Add 1 before truncature
    // 0/3 => truncate to 0, 1/3 => truncate to 0, and 2/3 => truncate to 1
    if (vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies] >= 0)
      vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies] += 1;
    else vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies] -= 1;
  }
  
  // Ecriture des moyennes sur 3 
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {    
    vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies] = (vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][indexVoies])/D_TRS_NB_ECH_FILTRE;
  }

  vip4f->counter_trs++;
}

void agCrete(struct vip4f_t *vip4f)
{
  long indexVoies;
  
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    TRS_DetectionCrete (vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][indexVoies], &(vip4f->V_DETC[indexVoies]));
  }
}

void agCreteMoyTRS(void *arg)
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
  
  int cmpt_moy = 0;
  int cmpt_trs = 0;  

  // Initialization part
  init_crete(vip4f);
  init_moy(vip4f);    
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

  // Periodic part
  while (1) {
    while (id != owner);

    agCrete(vip4f);
    if (cmpt_moy == D_TRS_NB_ECH_FILTRE) {
      agMoy(vip4f);
      cmpt_moy = 0;
    }
    cmpt_moy++;    

    if (cmpt_trs == D_TRS_NB_BUF_I) {
      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
	// Patch to put in echantillon array the last values (and thus highest indexEchan)
	// at the beginning of the array (and thus the lowest index)
	long index = 0;
	for (indexEchan = D_TRS_NB_BUF_I - 1; indexEchan >= 0; --indexEchan) {
	  echantillon[indexVoies][index] = vip4f->V_TRS_CumulFiltre[indexEchan][indexVoies];
	  index++;
	}
      }
      
      /* Mesure de phaseur (voies I1, I2, I3 et Io),crete filtree */
      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES; ++indexVoies) {
	TRS_EchantillonSinCosH12((int *)&echantillon[indexVoies], &(vip4f->VS_Mod2[indexVoies]), indexVoies);
      }
    
      /* Uniquement pour voies I1, I2 et I3 */
      for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {    
	TRS_EchantillonCreteFiltree2((int *)&echantillon[indexVoies],&(vip4f->VS_Mod2Crete[indexVoies]));
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

      cmpt_trs = 0;

      // In that case, the next task to be executed is ag5051_51Inv
      owner = 3;
      continue;
    }
    cmpt_trs++;    

    // Going to agRMS task
    owner = 1;
  }
}

