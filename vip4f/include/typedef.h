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







