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
  printf("ARGA/T=%d: i=%lu; I1=%d, I2=%d, I3=%d, Io=%d\n", current_time, vip4f->counter,
	 vip4f->DataBufferI[D_ACQ_VOIE_I1], vip4f->DataBufferI[D_ACQ_VOIE_I2], 
	 vip4f->DataBufferI[D_ACQ_VOIE_I3], vip4f->DataBufferI[D_ACQ_VOIE_Io]);
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

  vip4f->counter = 0;

  while (1) {
    while (id != owner);

    // If needed, display stuff
    display(vip4f, *timer_ptr);

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

  printf("Launching core 1, out of %d\n", get_cpucnt());
  corethread_create(1, &agRMS, NULL);

  printf("Launching core 2, out of %d\n", get_cpucnt());
  corethread_create(2, &agCreteMoyTRS, NULL);

  printf("Launching core 3, out of %d\n", get_cpucnt());
  corethread_create(3, &ag5051_51Inv, NULL);

  printf("Launching core 0, out of %d\n", get_cpucnt());  
  agARGA(NULL);

  return 0;
}
