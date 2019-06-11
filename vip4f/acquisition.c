#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "../../patmos/c/libcorethread/corethread.h"

#include "include/typedef.h"
#include "include/define.h"
#include "include/shared.h"
#include "include/datas.h"
#include "lib/MaxI.h"

volatile _UNCACHED int owner;
vip4f_t vip4f_data = {0};

void display(volatile vip4f_t *vip4f, int current_time) 
{
  static int cmptrms = -1;
  static int cmpttrs = -1;  

  printf("ARGA/c=%ld, I1=%d, I2=%d, I3=%d, Io=%d\n", vip4f->counter % D_TRS_NB_ECH_FILTRE,
	 (int) vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I1],
	 (int) vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I2], 
	 (int) vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I3],
	 (int) vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_Io]);

  printf("CRE/I1.S=%d, I2.S=%d, I3.S=%d\n",
          vip4f->V_DETC[D_ACQ_VOIE_I1].S, vip4f->V_DETC[D_ACQ_VOIE_I2].S,
  	 vip4f->V_DETC[D_ACQ_VOIE_I3].S);
  
  printf("RMS/I1=%d, I2=%d, I3=%d, 2I1=%lu, 2I2=%lu, 2I3=%lu\n",
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I1], (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I2],
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I3],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I1], vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I2],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I3]);
  
  cmptrms++;
  if (cmptrms == D_TRS_NB_ECH_RMS) {
    printf("RMS/I1=%d, I2=%d, I3=%d, Io=%d, I1RMS=%d, I2RMS=%d, I3RMS=%d\n",
  	 vip4f->I[D_ACQ_VOIE_I1], vip4f->I[D_ACQ_VOIE_I2],
  	 vip4f->I[D_ACQ_VOIE_I3], vip4f->I[D_ACQ_VOIE_Io],
  	 (int) vip4f->RMS[D_ACQ_VOIE_I1], (int) vip4f->RMS[D_ACQ_VOIE_I2],
  	 (int) vip4f->RMS[D_ACQ_VOIE_I3]);
    cmptrms = 0;
  }

  if (vip4f->counter_moy == D_TRS_NB_ECH_FILTRE) {
    printf("MOY/c=%ld, I1=%d, I2=%d, I3=%d, Io=%d\n", vip4f->counter_trs,
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][D_ACQ_VOIE_I1],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][D_ACQ_VOIE_I2],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][D_ACQ_VOIE_I3],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs % D_TRS_NB_BUF_I][D_ACQ_VOIE_Io]);
    vip4f->counter_trs++;
    vip4f->counter_moy = 0;
  }
  
  cmpttrs++;
  if (cmpttrs == D_TRS_NB_BUF_I) {
    for (int i= 0; i < D_TRS_NB_BUF_I; i++) {
      printf("CumulFiltre=%d,", vip4f->V_TRS_CumulFiltre[i][D_ACQ_VOIE_I1]);
    }
    printf("\nTRS/CreteFiltre=%Lu, %Lu, %Lu\n",
	   (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I1]),
	   (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I2]),
	   (unsigned long long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I3]));
    printf("TRS/SinCosH1=%Lu, %Lu, %Lu, %Lu\n",
	   (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I1]),
	   (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I2]),
	   (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I3]),
	   (unsigned long long)(vip4f->VS_Mod2[D_ACQ_VOIE_Io]));
    printf("TRS/V_mod2Imax=%Lu\n", (unsigned long long)(vip4f->V_mod2Imax));
    
    for (int i = 0; i < 5; i++) {
      if (vip4f->status[i] & BIT_TEMPO) {
	printf("5051/DECLENCHEMENT %d\n", i);     
      }
    }
    
    cmpttrs = 0;
  }
}


/*=====================================================================================*/
/*-------------------------------  AgARGA  -----------------------------------------*/
/*=====================================================================================*/
void agARGA(void *arg)  {
  // Get access to the shared structure
  //  volatile struct vip4f_t *vip4f = (struct vip4f_t*) PATMOS_IO_OWNSPM;
  
  // Compteur d échantillon dans un cycle rapide  
  U_LONG  cmpt = 0;

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  int val;

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  int current_time, start_time, end_time;

  int id = get_cpuid();

  vip4f->counter = -1;

  while (1) {
    while (id != owner);

    /* start_time = *timer_ptr; */
    /* *dead_ptr = P_ARGA - OVERHEAD_DELAY; */
    /* val = *dead_ptr; */
    /* end_time = *timer_ptr; */
    /* printf("Delay measurement %d %d\n", end_time-start_time, val); */
    display(vip4f, *timer_ptr);

    start_time = *timer_ptr;
    vip4f->counter++;                
    vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I1] = sinus_data100A[cmpt][D_ACQ_VOIE_I1];
    vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I2] = sinus_data100A[cmpt][D_ACQ_VOIE_I2];
    vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I3] = sinus_data100A[cmpt][D_ACQ_VOIE_I3];
    vip4f->DataBufferI[vip4f->counter % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_Io] = 0;
    
    ++cmpt;    
    // Reboucle sur le début de la sinusoide
    if (cmpt>35) {
      cmpt=0;
    }
    inval_dcache(); //invalidate the data cache    
    
    end_time = *timer_ptr;

    // Going to agCreteMoyTRS task
    owner = 2; 
  }
}

int main() {

  owner = 0; // start with myself

  vip4f = &vip4f_data;

  corethread_create(1, &agRMS, NULL);
  corethread_create(2, &agCreteMoyTRS, NULL);
  corethread_create(3, &ag5051_51Inv, NULL);
  agARGA(NULL);

  return 0;
}
