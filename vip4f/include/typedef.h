/*============================================================================*/
/*                                  TYPEDEFS                                  */
/*============================================================================*/
typedef unsigned char 	UI8;	///< Definition of 8-bit unsigned int data.
typedef unsigned short 	UI16;	///< Definition of 16-bit unsigned int data.
typedef unsigned long 	UI32;	///< Definition of 32-bit unsigned int data.

typedef signed char 	SI8;	///< Definition of 8-bit signed int data.
typedef signed short 	SI16;	///< Definition of 16-bit signed int data.
typedef signed long 	SI32;	///< Definition of 32-bit signed int data.

typedef unsigned long   U_LONG;

#ifdef POSIX
typedef unsigned char  BOOL;   ///< Definition of bool.
typedef unsigned char   CHAR;  ///< Definition of charater.
#endif


// structure pour la detection crete
typedef struct S_TRS_DETC {
	short Maxi_ech;
	short Mini_ech;
	short Np;
	short Nn;
	short S;
	short LastS;
}S_TRS_DETC;


/* structure pour les phaseurs */
typedef struct  {
	short ComposanteFx;
	short ComposanteFy;
	short ComposanteFx_old;
	short ComposanteFy_old;
}S_TRS_FXFY;

long diviseAuMieux(long A, long B);
unsigned long isqrt(unsigned long n);
int calculeIn2();

void TRS_DetectionCrete (int ech, S_TRS_DETC *ptr2);
void TRS_EchantillonCreteFiltree (short * Table_Echantillons, long *Ptr_Crete_voie);
void TRS_EchantillonCreteFiltree2 (int * Table_Echantillons, long *Ptr_Crete_voie);
void TRS_EchantillonSinCosH1(short *ptr1 , S_TRS_FXFY *ptr2);
void TRS_EchantillonSinCosH12(int *ptr1 , long long *ptr2, int aVoie);

long CourantRMS_dA( unsigned long cumRMS2, long cumRMS);

void F51_Init( char inst, long seuil, short tempo);
void F51_RapideExp(char inst, long V_mod2Imax);

void F51_Inv_Init(char inst, long seuil, short tempo);
void F51_Inv_RapideExp(char inst, long V_mod2Imax);






