#include <stdio.h>
#include <string.h>
#include "cssl.h"
#include "fcntl.h"

#define DEMONSTRATION

#define TUBE_ENTREE		"/export/home/mathieu/Implems/OASIS/Applications/FlexEWare/tubes/entree"
#define TUBE_SORTIE		"/export/home/mathieu/Implems/OASIS/Applications/FlexEWare/tubes/sortie"

#define BUFFER_SIZE 50

#ifdef DEMONSTRATION
	int entree;
	int sortie;
#else
	cssl_t *gserial;
#endif /* DEMONSTRATION */

int OuvrePort(int direction) {
	
#ifdef DEMONSTRATION
	// Cree le tube nommé.
//	int ret_val_entree = mkfifo(TUBE_ENTREE, 0666);
//	int ret_val_sortie = mkfifo(TUBE_SORTIE, 0666);
	
//	if (ret_val_entree == -1) {
//	        perror("Error creating the named pipe entree");
//	        exit (1);
//	    }
//	if (ret_val_sortie == -1) {
//	        perror("Error creating the named pipe sortie");
//	        exit (1);
//	    }
	if (direction == 0 || direction == 2)
		entree = open(TUBE_ENTREE, O_RDONLY|O_NONBLOCK);
	if (direction == 1 || direction == 2)
		sortie = open(TUBE_SORTIE, O_RDWR|O_NONBLOCK);
	if ( entree == -1 )
		printf("ERREUR ouverture tubes nommés d'entrée !\n");
	if ( sortie == -1 )
		printf("ERREUR ouverture tubes nommés de sortie !\n");
	
	return sortie;
#else
	if (gserial == 0) {
		
		gserial = cssl_open("/dev/ttyS0",
                          0x00,
                          0,
                          115200,
                          8,
                          0,
                          1);
		//printf("gesrial = %d\n", gserial);
	}
#endif
}


void FermePort(int direction) {
#ifdef DEMONSTRATION	
	if (direction == 0)
		close(entree);
	if (direction == 1)
		close(sortie);
#else
    cssl_close(gserial);
    gserial = 0;
#endif /* DEMONSTRATION */
}

void EcrireLigne(char *aLigne) {
#ifdef DEMONSTRATION
	int i=0;
	if (sortie > 0) {
		write(sortie, aLigne, strlen(aLigne));
	}
#else
	
#endif /* DEMONSTRATION */
}


int LireOctet() {
#ifdef DEMONSTRATION
	int bytes;
	char buf;
	
	if (entree) {
		bytes = read(entree, &buf, 1);
		if ((buf >= 35 && buf <= 90) ||
			(buf >= 97 && buf <= 122)) return buf;
	}

#else
	if (gserial) {
		return cssl_getchar(gserial);
	}
	else
		return -1;
#endif /* DEMONSTRATION */
}
