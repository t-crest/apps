/* A big structure for sharing stuff */
struct vip4f_t {
  // Produced by agARGA 
  /* For I0, I1, I2, I3 */
  SI32 DataBufferI[D_ACQ_NB_VOIES];
  long counter;

  // Produced by AgRMS
  /* Uniquement I1, I2 et I3 */
  long RMS[D_ACQ_NB_VOIES-1];
  /* I1, I2, I3 et Io */
  int I[D_ACQ_NB_VOIES];

  // Produced by AgMoy
  /* cumul Ã©chantillons pour filtre TRS */
  /* I1, I2, I3 et Io */
  int V_TRS_CumulFiltre [D_ACQ_NB_VOIES];

  // Produced by AgCrete
  S_TRS_DETC V_DETC[D_ACQ_NB_VOIES-1];

  // Produced by AgTRS
  long  V_mod2Imax;	/* grandeur caracteristique  = Imax en Pts Can carre */
  /* Voies I1, I2, I3 et Io */
  long long VS_Mod2 [D_ACQ_NB_VOIES];
  /* Voies I1, I2 et I3 */
  long VS_Mod2Crete [D_ACQ_NB_VOIES-1];  
} __attribute__((packed));

/* To define the periodicity of the ARGA task */
#define P_ARGA 55500

#define OVERHEAD_DELAY 4

/* Each function defines a task */
void agARGA(void *arg);
void agRMS(void *arg);
void agCreteMoyTRS(void *arg);
void ag5051_51Inv(void *arg);

volatile _UNCACHED extern int owner;
