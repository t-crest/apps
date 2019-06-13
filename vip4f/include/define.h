/*============================================================================*/
/*                                  DEFINES                                  */
/*============================================================================*/

#define D_ACQ_VOIE_I1		0	/* Courant I1 */
#define D_ACQ_VOIE_I2		1	/* Courant I2 */
#define D_ACQ_VOIE_I3		2	/* Courant I3 */
#define D_ACQ_VOIE_Io		3	/* Courant I0 */

#define D_ACQ_NB_VOIES		4	/* nombre de voies acquises */

/* nombre de sommation pour le filtre TRS (AgMoy) */
#define D_TRS_NB_ECH_FILTRE	3

/* Nombre d'échantillons à cumuler pour le calcul RMS */
#define D_TRS_NB_ECH_RMS 24

/* nb d'échantillons filtrés utiles au calcul TRS */
#define D_TRS_NB_BUF_I		12				

/* Used by MaxI.c, F51_Inv.c, utils.c and Rms.c */
#define V_TRS_In 100

//#define DATA30000 1

#define TYPE_PERT_RESET  64
#define TYPE_PERT_MIEUX  65 /* A */
#define TYPE_PERT_PIRE   66 /* B */
#define TYPE_PERT_CHARGE 67 /* C */
