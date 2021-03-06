#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/typedef.h"
#include "include/define.h"
#include "include/shared.h"

/*=====================================================================================*/
/*-------------------------------  Ag5051  --------------------------------------------*/
/*----------------------------Détection à temps constant-------------------------------*/
/*----------------------------Détection à temps dépendant------------------------------*/
/*=====================================================================================*/
void ag5051_51Inv(void *arg)
{
  // Get access to the shared structure
  vip4f_t *vip4f = (vip4f_t*) PATMOS_IO_OWNSPM;

  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  int start_time, end_time;    
 
  int i;

  int SeuilDecl[5];  
  
  int id = get_cpuid();    
  
  /* Init F51 */
  /* Paramètres : */
  /* param1 = numéro de la protection */
  /* param2 = seuil de déclenchement en ampère */
  /* param3 = temporisation en nombre de cycle rapide (N*13,3ms) */
  F51_Init(0, 200,0);
  F51_Init(1, 300,1);
  F51_Init(2, 500,2);
  
  F51_Inv_Init(3, 200, 0); /* 0 ms */
  F51_Inv_Init(4, 1000, 5); /* 10ms */

  // Periodic part
  while (1) {
    while (id != owner);

    start_time = *timer_ptr;    
    
    /* F51_RapideExp(0, vip4f->V_mod2Imax); */
    /* F51_RapideExp(1, vip4f->V_mod2Imax); */
    /* F51_RapideExp(2, vip4f->V_mod2Imax); */

    F51_RapideDemo(0, vip4f->V_mod2Imax, &(vip4f->status[0]), &SeuilDecl[0]);
    F51_RapideDemo(1, vip4f->V_mod2Imax, &(vip4f->status[1]), &SeuilDecl[1]);
    F51_RapideDemo(2, vip4f->V_mod2Imax, &(vip4f->status[2]), &SeuilDecl[2]);    
    
    /* F51_Inv_RapideExp(3, vip4f->V_mod2Imax); */
    /* F51_Inv_RapideExp(4, vip4f->V_mod2Imax); */

    F51_Inv_RapideDemo(3, vip4f->V_mod2Imax, &(vip4f->status[3]), &SeuilDecl[3]);
    F51_Inv_RapideDemo(4, vip4f->V_mod2Imax, &(vip4f->status[4]), &SeuilDecl[4]);

    inval_dcache(); //invalidate the data cache        
    end_time = *timer_ptr;
    vip4f->protection = end_time-start_time;
    
    inval_dcache(); //invalidate the data cache            

    // Going back to agRMS task
    owner = 1;
  }
}

