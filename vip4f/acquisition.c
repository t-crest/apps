#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "../../patmos/c/libcorethread/corethread.h"

#include "include/typedef.h"
#include "include/define.h"
#include "include/shared.h"
#include "include/datas.h"

volatile _UNCACHED int owner;

/* Compared to the version published at SIES, the cumulRMS task has
   been merged with agARGA in order to reduce the number of tasks for
   patemu and as it has the same temporal behavior */
void agCumulRMS(struct vip4f_t *vip4f) {
  long indexEchan, indexVoies;
  
  for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
    // MAJ des cumuls echantillons
      vip4f->V_TRS_CumulRms[indexVoies] += (U_LONG)(vip4f->DataBufferI[indexVoies]);
      // MAJ des cumuls carre echantillons
      vip4f->V_TRS_CumulRms2[indexVoies] += (U_LONG)((U_LONG)vip4f->DataBufferI[indexVoies]*(U_LONG)vip4f->DataBufferI[indexVoies]);
    }
    
  vip4f->cmpt_rms++;
    
  if (vip4f->cmpt_rms > D_TRS_NB_ECH_RMS) {
    vip4f->cmpt_rms = 0;
    for (indexVoies = 0; indexVoies < D_ACQ_NB_VOIES-1; ++indexVoies) {
      // RAZ des cumuls échantillons
      vip4f->V_TRS_CumulRms[indexVoies] = 0;
      // RAZ des cumuls carrés échantillons	  
      vip4f->V_TRS_CumulRms2[indexVoies] = 0;				
    }
  }
}

/*=====================================================================================*/
/*-------------------------------  AgARGA  -----------------------------------------*/
/*=====================================================================================*/
void agARGA(void *arg)  {
  // Get access to the shared structure
  struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  // Compteur d échantillon dans un cycle rapide  
  U_LONG  cmpt = 0;

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  int val;

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  int current_time, start_time, end_time;

  int id = get_cpuid();
  printf("ARGA, cpuid=%d\n",id);

  vip4f->cmpt_rms = 0;  

  while (1) {
    while (id != owner);

    start_time = *timer_ptr;
    *dead_ptr = P_ARGA - OVERHEAD_DELAY;
    val = *dead_ptr;
    end_time = *timer_ptr;
    printf("Delay measurement %d %d\n", end_time-start_time, val);
    
    start_time = *timer_ptr;
    vip4f->DataBufferI[D_ACQ_VOIE_I1] = sinus_data100A[cmpt][D_ACQ_VOIE_I1];
    vip4f->DataBufferI[D_ACQ_VOIE_I2] = sinus_data100A[cmpt][D_ACQ_VOIE_I2];
    vip4f->DataBufferI[D_ACQ_VOIE_I3] = sinus_data100A[cmpt][D_ACQ_VOIE_I3];
    vip4f->DataBufferI[D_ACQ_VOIE_Io] = 0;

    ++cmpt;    
    // Reboucle sur le début de la sinusoide
    if (cmpt>35) {
      cmpt=0;
    }

    agCumulRMS(vip4f);
    
    end_time = *timer_ptr;
    printf("ARGA/T=%d: i=%lu; I1=%d, I2=%d, I3=%d, Io=%d\n", current_time, cmpt,
	   vip4f->DataBufferI[D_ACQ_VOIE_I1], vip4f->DataBufferI[D_ACQ_VOIE_I2], 
	   vip4f->DataBufferI[D_ACQ_VOIE_I3], vip4f->DataBufferI[D_ACQ_VOIE_Io]);
    printf("Cumul RMS /T=%d: I1=%d, I2=%d, I3=%d, 2I1=%lu, 2I2=%lu, 2I3=%lu\n", current_time,
	   vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I1], vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I2], vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I3],
	   vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I1], vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I2], vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I3]);    

    owner = 0;
  }
}

int main() {

  owner = 0; // start with myself  

  /* printf("Launching core 1, out of %d\n", get_cpucnt()); */
  /* corethread_create(1, &agRMS, NULL); */

  /* printf("Launching core 2, out of %d\n", get_cpucnt()); */
  /* corethread_create(2, &agCumulRMS, NULL); */

  /* printf("Launching core 3, out of %d\n", get_cpucnt()); */
  /* corethread_create(3, &ag5051_51Inv, NULL);      */

  printf("Launching core 0, out of %d\n", get_cpucnt());  
  agARGA(NULL);

  return 0;
}
