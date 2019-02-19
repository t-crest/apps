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

void display(struct vip4f_t *vip4f, int current_time) 
{
  printf("Counter=%lu, Time=%d\n", vip4f->counter, current_time);
  printf("ARGA/I1=%d, I2=%d, I3=%d, Io=%d\n", 
	 (int) vip4f->DataBufferI[D_ACQ_VOIE_I1], (int) vip4f->DataBufferI[D_ACQ_VOIE_I2], 
	 (int) vip4f->DataBufferI[D_ACQ_VOIE_I3], (int) vip4f->DataBufferI[D_ACQ_VOIE_Io]);
  
  printf("RMS/AI1=%d, AI2=%d, AI3=%d, 2I1=%lu, 2I2=%lu, 2I3=%lu\n",
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I1], (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I2],
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I3],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I1], vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I2],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I3]);
  /* printf("RMS/I1=%d, I2=%d, I3=%d, Io=%d, I1RMS=%d, I2RMS=%d, I3RMS=%d\n",  */
  /* 	 vip4f->I[D_ACQ_VOIE_I1], vip4f->I[D_ACQ_VOIE_I2], */
  /* 	 vip4f->I[D_ACQ_VOIE_I3], vip4f->I[D_ACQ_VOIE_Io], */
  /* 	 (int) vip4f->RMS[D_ACQ_VOIE_I1], (int) vip4f->RMS[D_ACQ_VOIE_I2], */
  /* 	 (int) vip4f->RMS[D_ACQ_VOIE_I3]); */
  
  printf("MOY/I1=%d, I2=%d, I3=%d, Io=%d\n", 
	 vip4f->V_TRS_CumulFiltre [D_ACQ_VOIE_I1], vip4f->V_TRS_CumulFiltre [D_ACQ_VOIE_I2], 
	 vip4f->V_TRS_CumulFiltre [D_ACQ_VOIE_I3], vip4f->V_TRS_CumulFiltre [D_ACQ_VOIE_Io]);
  /* printf("CRE/I1.S=%d, I2.S=%d, I3.S=%d\n",  */
  /*         vip4f->V_DETC[D_ACQ_VOIE_I1].S, vip4f->V_DETC[D_ACQ_VOIE_I2].S, */
  /* 	 vip4f->V_DETC[D_ACQ_VOIE_I3].S); */
  /* printf("TRS/Crete filtre=%Lu, %Lu, %Lu\n",  */
  /* 	 (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I1]), */
  /* 	 (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I2]),  */
  /* 	 (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I3])); */
  /* printf("TRS/SinCosH1=%Lu, %Lu, %Lu, %Lu\n",  */
  /* 	 (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I1]), */
  /* 	 (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I2]),  */
  /* 	 (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I3]), */
  /* 	 (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_Io])); */
  /* printf("TRS/V_mod2Imax=%Lu\n", (unsigned long long)(vip4f->V_mod2Imax)); */

  // TODO: add display when protection is triggered
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

  vip4f->counter = 0;

  while (1) {
    while (id != owner);

    // If needed, display stuff
    display(vip4f, *timer_ptr);

    /* start_time = *timer_ptr; */
    /* *dead_ptr = P_ARGA - OVERHEAD_DELAY; */
    /* val = *dead_ptr; */
    /* end_time = *timer_ptr; */
    /* printf("Delay measurement %d %d\n", end_time-start_time, val); */
    
    start_time = *timer_ptr;
    vip4f->DataBufferI[D_ACQ_VOIE_I1] = sinus_data100A[cmpt][D_ACQ_VOIE_I1];
    vip4f->DataBufferI[D_ACQ_VOIE_I2] = sinus_data100A[cmpt][D_ACQ_VOIE_I2];
    vip4f->DataBufferI[D_ACQ_VOIE_I3] = sinus_data100A[cmpt][D_ACQ_VOIE_I3];
    vip4f->DataBufferI[D_ACQ_VOIE_Io] = 0;
    vip4f->counter++;

    ++cmpt;    
    // Reboucle sur le début de la sinusoide
    if (cmpt>35) {
      cmpt=0;
    }
    
    end_time = *timer_ptr;

    // Going to agRMS task
    owner = 1;
  }
}

int main() {

  owner = 0; // start with myself  

  corethread_create(1, &agRMS, NULL);
  corethread_create(2, &agCreteMoyTRS, NULL);
  corethread_create(3, &ag5051_51Inv, NULL);
  agARGA(NULL);

  return 0;
}
