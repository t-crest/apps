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
//vip4f_t vip4f_data = {0};

void display(volatile vip4f_t *vip4f, int current_time) 
{
  printf("ARGA/c=%ld, I1=%d, I2=%d, I3=%d, Io=%d, E=%d\n",
	 vip4f->counter_arga % D_TRS_NB_ECH_FILTRE,
	 (int) vip4f->DataBufferI[vip4f->counter_arga % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I1],
	 (int) vip4f->DataBufferI[vip4f->counter_arga % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I2], 
	 (int) vip4f->DataBufferI[vip4f->counter_arga % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_I3],
	 (int) vip4f->DataBufferI[vip4f->counter_arga % D_TRS_NB_ECH_FILTRE][D_ACQ_VOIE_Io],
	 vip4f->arga);

  printf("CRE/I1.S=%d, I2.S=%d, I3.S=%d, E=%d\n",
	 vip4f->V_DETC[D_ACQ_VOIE_I1].S, vip4f->V_DETC[D_ACQ_VOIE_I2].S,
  	 vip4f->V_DETC[D_ACQ_VOIE_I3].S, vip4f->crete);
  
  printf("RMS/I1=%d, I2=%d, I3=%d, 2I1=%lu, 2I2=%lu, 2I3=%lu, E=%d\n",
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I1], (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I2],
	 (int) vip4f->V_TRS_CumulRms[D_ACQ_VOIE_I3],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I1], vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I2],
	 vip4f->V_TRS_CumulRms2[D_ACQ_VOIE_I3],
	 vip4f->rms1);
  
  if (vip4f->counter_rms == D_TRS_NB_ECH_RMS) {
    printf("RMS/c=%ld, I1=%d, I2=%d, I3=%d, Io=%d, I1RMS=%d, I2RMS=%d, I3RMS=%d, E=%d\n",
	   vip4f->counter_rms,
	   vip4f->I[D_ACQ_VOIE_I1], vip4f->I[D_ACQ_VOIE_I2],
	   vip4f->I[D_ACQ_VOIE_I3], vip4f->I[D_ACQ_VOIE_Io],
	   (int) vip4f->RMS[D_ACQ_VOIE_I1], (int) vip4f->RMS[D_ACQ_VOIE_I2],
	   (int) vip4f->RMS[D_ACQ_VOIE_I3],
	   vip4f->rms2);
    vip4f->counter_rms = 0;          
  }

  if (vip4f->counter_moy == D_TRS_NB_ECH_FILTRE) {
    // We use the counter as an index, so decrement it
    printf("MOY/c=%ld, I1=%d, I2=%d, I3=%d, Io=%d, E=%d\n", vip4f->counter_trs,
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs - 1][D_ACQ_VOIE_I1],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs - 1][D_ACQ_VOIE_I2],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs - 1][D_ACQ_VOIE_I3],
	   vip4f->V_TRS_CumulFiltre[vip4f->counter_trs - 1][D_ACQ_VOIE_Io],
	   vip4f->moy);
    vip4f->counter_moy = 0;
  }
  
  if (vip4f->counter_trs == D_TRS_NB_BUF_I) {    
    printf("TRS/c=%ld, CreteFiltre=%ld, %ld, %ld\n",
	   vip4f->counter_trs,
	   (long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I1]),
	   (long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I2]),
	   (long)(vip4f->VS_Mod2Crete[D_ACQ_VOIE_I3]));
    printf("TRS/SinCosH1=%lld, %lld, %lld, %lld\n",
	   (long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I1]),
	   (long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I2]),
	   (long long)(vip4f->VS_Mod2[D_ACQ_VOIE_I3]),
	   (long long)(vip4f->VS_Mod2[D_ACQ_VOIE_Io]));
    printf("TRS/CreteNonFiltre=%ld, %ld, %ld, %ld\n", 
	   (long)(vip4f->VS_Mod2_S[D_ACQ_VOIE_I1]), (long)(vip4f->VS_Mod2_S[D_ACQ_VOIE_I2]), 
	   (long)(vip4f->VS_Mod2_S[D_ACQ_VOIE_I3]), (long)(vip4f->VS_Mod2_S[D_ACQ_VOIE_Io]));
    printf("TRS/V_mod2Imax=%ld, E=%d\n", (long)(vip4f->V_mod2Imax),
	    vip4f->trs);
    vip4f->counter_trs = 0;
    
    for (int i = 0; i < 5; i++) {
      if (vip4f->status[i] & BIT_TEMPO) {
	printf("5051/DECLENCHEMENT %d\n", i);     
      }
    }
    printf("PRO/E=%d\n", vip4f->protection);
    
    vip4f->cmpttrs = 0;
  }
}


/*=====================================================================================*/
/*-------------------------------  AgARGA  -----------------------------------------*/
/*=====================================================================================*/
void agARGA(void *arg)  {
  // Get access to the shared structure
  volatile vip4f_t *vip4f = (vip4f_t*) PATMOS_IO_OWNSPM;
  //memset(&vip4f, 0, sizeof(vip4f_t));
  
  // Compteur d échantillon dans un cycle rapide  
  U_LONG  cmpt = 0;

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  int val;

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  int current_time, start_time, end_time;

  int id = get_cpuid();

  vip4f->counter_arga = -1;

  while (1) {
    while (id != owner);

    if (vip4f->counter_arga >= 0) {
      
      // Here with display the results of the previous loop
      display(vip4f, *timer_ptr);
      
      // An we set a deadline 
      /* start_time = *timer_ptr; */
      /* *dead_ptr = P_ARGA - OVERHEAD_DELAY; */
      /* val = *dead_ptr; */
      /* end_time = *timer_ptr; */
      /* printf("Delay measurement %d %d\n", end_time-start_time, val); */
    }

    start_time = *timer_ptr;
    vip4f->counter_arga++;
    if (vip4f->counter_arga == D_TRS_NB_ECH_FILTRE) vip4f->counter_arga = 0;
#ifdef DATA30000    
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I1] = sinus_data30000A[cmpt][D_ACQ_VOIE_I1];
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I2] = sinus_data30000A[cmpt][D_ACQ_VOIE_I2];
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I3] = sinus_data30000A[cmpt][D_ACQ_VOIE_I3];
#else
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I1] = sinus_data100A[cmpt][D_ACQ_VOIE_I1];
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I2] = sinus_data100A[cmpt][D_ACQ_VOIE_I2];
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_I3] = sinus_data100A[cmpt][D_ACQ_VOIE_I3];
#endif    
    vip4f->DataBufferI[vip4f->counter_arga][D_ACQ_VOIE_Io] = 0;
    
    ++cmpt;    
    // Reboucle sur le début de la sinusoide
    if (cmpt>35) {
      cmpt=0;
    }
    inval_dcache(); //invalidate the data cache    
    
    end_time = *timer_ptr;
    vip4f->arga = end_time-start_time;
    inval_dcache(); //invalidate the data cache        

    // Going to agCreteMoyTRS task
    owner = 2; 
  }
}

int main() {

  owner = 0; // start with myself

  //vip4f = &vip4f_data;

  corethread_create(1, &agRMS, NULL);
  corethread_create(2, &agCreteMoyTRS, NULL);
  corethread_create(3, &ag5051_51Inv, NULL);
  agARGA(NULL);

  return 0;
}
