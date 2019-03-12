#include "define.h"

/*============================================================================*/
/*                                  CLOCKS                                  */
/*============================================================================*/

/* For management of agents */
#define WITH_MOY
#define WITH_CRETE

#ifdef WITH_MOY
#ifdef WITH_CRETE
/* Can be commented or not */
#define WITH_TRS
#endif
#endif

#ifdef WITH_TRS
/* Can be commented or not */
#define PROTECTIONS
#endif

#define WITH_CUMUL_RMS

#ifdef WITH_CRETE
#ifdef WITH_TRS
#ifdef WITH_CUMUL_RMS
/* Can be commented or not */
#define WITH_RMS
#endif
#endif
#endif


/* Base tick */
#ifdef SLOW
clock US = gtc2(0,0,100); /* 100 usec */
#else
clock US = gtc2(0,0,5); /* 5 usec*/
#endif

/* Period of the ARGA agent */
clock HA_ARGA = ARGA_RHYTHM*US;

/* Period of the Crete agent */
clock HA_CRETE = HA_ARGA;

/* Period of the moy agent */
clock HA_MOY = D_TRS_NB_ECH_FILTRE*HA_ARGA;

/* Period of the RMS agent */
#ifdef WITH_CUMUL_RMS
clock HA_RMS = D_TRS_NB_ECH_RMS*HA_ARGA;
#endif

clock HA_PROT_BASE = D_TRS_NB_BUF_I*HA_ARGA;

/* Period of the TRS agent */
clock HA_TRS = HA_PROT_BASE;

/* Period of the 5051 and 51Inv agents */
/* Same period as the TRS agent */
/* Déclenchement de la protection tous les 24 échantillons F = 75Hz) */
clock HA_5051 = HA_PROT_BASE;
clock HA_51INV = HA_5051;

/* Period of the protection agents calculation termination */
clock HA_PROTB = D_TRS_NB_BUF_I*HA_ARGA - 3;

/* Period of the COMM agent F = 1Hz */
clock HA_COMM = HA_ARGA;

