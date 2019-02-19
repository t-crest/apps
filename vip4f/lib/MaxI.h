// constantes propres a la fonction 

#define NB_REGL			4		// nombre de reglages par exemplaire et par jeu 

#define	RGL_ON_OFF		0		// indice de reglage pour les differents reglages 
#define	RGL_SEUIL		1
#define	RGL_TEMPO		2

// constantes pour decoder les champs de bits
#define BIT_INST		  0x01		// dans VA_ProtStatus
#define BIT_TEMPO		  0x02
#define BIT_ACTIVE		0x04
#define BIT_DEF_REGL	0x80

