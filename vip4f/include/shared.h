/* A big structure for sharing stuff */
typedef struct {
  // Produced by agARGA 
  /* For I0, I1, I2, I3 */
  SI32 DataBufferI[D_TRS_NB_ECH_FILTRE][D_ACQ_NB_VOIES];

  // Produced by AgRMS
  /* Uniquement I1, I2, I3 */  
  /* cumul echantillons pour offset RMS */
  long V_TRS_CumulRms[D_ACQ_NB_VOIES-1];
  /* Uniquement I1, I2, I3 */  
  /* cumul carre echantillons pour calcul RMS */
  U_LONG V_TRS_CumulRms2[D_ACQ_NB_VOIES-1];  
  /* Uniquement I1, I2 et I3 */
  long RMS[D_ACQ_NB_VOIES-1];
  /* I1, I2, I3 et Io */
  int I[D_ACQ_NB_VOIES];  

  // Produced by AgCreteMoyTRS
  /* cumul echantillons pour filtre TRS */
  /* I1, I2, I3 et Io */
  int V_TRS_CumulFiltre[D_TRS_NB_BUF_I][D_ACQ_NB_VOIES];
  S_TRS_DETC V_DETC[D_ACQ_NB_VOIES-1];
  long  V_mod2Imax;	/* grandeur caracteristique  = Imax en Pts Can carre */
  /* Voies I1, I2, I3 et Io */
  long long VS_Mod2 [D_ACQ_NB_VOIES];
  /* Voies I1, I2 et I3 */
  long VS_Mod2Crete [D_ACQ_NB_VOIES-1];
  /* Uniquement S1, S2 et S3 */
  long VS_Mod2_S [D_ACQ_NB_VOIES-1];  

  long counter_arga;
  long counter_rms;
  long counter_moy;
  long counter_trs;
  long cmpttrs;

  // Produce by protection algorithms
  char status[5];

  // Exec time measurement
  int arga;
  int rms1;
  int rms2;
  int crete;
  int moy;
  int trs;
  int protection;
  
} vip4f_t;

/* To define the periodicity of the ARGA task */
#define P_ARGA 55500

#define OVERHEAD_DELAY 4

/* Each function defines a task */
void agARGA(void *arg);
void agRMS(void *arg);
void agCreteMoyTRS(void *arg);
void ag5051_51Inv(void *arg);

//extern vip4f_t vip4f_data;
//vip4f_t* vip4f;

volatile _UNCACHED extern int owner;
volatile _UNCACHED extern long counter;
