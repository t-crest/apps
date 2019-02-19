/*****************************************************************************
*                                                                            *
* Fonction: diviseAuMieux 				                                           *
*                                                                            *
* Appel   : q = diviseAuMieux(a, b)                           		           *
*                                                                            *
* Entrees : 					                                                       *
* 	a = numerateur 		                        							                 *
* 	b = denominateur	                        							                 *
*                                                                            *
* Sorties : 					                                                       *
* 	q = a/b arrondi										                                       *
*                                                                            *
* ************************************************************************** *
*                                                                            *
* Description : division 32/32 => 32 avec arrondi a l'entier le + proche     *
*                                                                            *
*****************************************************************************/

#include <stdio.h>
#include "Pts.h"
#include "../include/define.h"
#include "../include/typedef.h"

long diviseAuMieux(long A, long B) {

long Q, R;		// quotien et reste

	Q = A / B;			// quotient				
	R = A - B*Q;		// reste
	if ( R > B/2) Q++;	// arrondir a l'entier le + proche
	return(Q);
}
//
//int file_open(char *file_name) {
//	if (!filedesc)
//		filedesc = fopen(file_name, "r");
//	if (!filedesc)
//		return 1;
//	else
//		return 0;
//}
//
//void file_getdata(int *voie0, int *voie1, int *voie2, int *voie3, unsigned long *nb) 
//{
//	int end;
//	int count;
//	if (filedesc)
//		fscanf(filedesc, "%d, %d, %d, %d, %d, %d", &count, voie0, voie1, voie2, voie3, &end);
//	(*nb)++;
//}

int calculePtsCanIn2()
{
	// Pour la prise en compte de la crête filtrée dans le cycle rapide :
		// ******************************************************************
		// Calcul de In en points CAN. In = 100A
		// Note : Unité de CA_In en Ampères (max : 6250 A).
		//        Unité de PtsCanA10InH1 en points CAN (= 5004 points).
		return diviseAuMieux((PtsCanA10InH1 * PtsCanA10InH1), 100L);
}

int calculeIn2()
{
	// Pour la prise en compte de la crête filtrée dans le cycle rapide :
		// ******************************************************************
		// Calcul de In en points CAN. In = 100A
		// Note : Unité de CA_In en Ampères (max : 6250 A).
		//        Unité de PtsCanA10InH1 en points CAN (= 5004 points).
		return V_TRS_In * V_TRS_In;
}

void inverseTab(int *tab, int dim)
{
	int tmp, i;

	for(i=0;i<(dim>>1);i++) {
		tmp = tab[i];
		tab[i] = tab[dim-1-i];
		tab[dim-1-i] = tmp;
	}
}

unsigned long isqrt(unsigned long n) {
  unsigned long s, t;

#define sqrtBit(k) \
  t = s+(1UL<<(k-1)); t <<= k+1; if (n >= t) { n -= t; s |= 1UL<<k; }

  s = 0UL;
#ifdef __alpha
  if (n >= 1UL<<62) { n -= 1UL<<62; s = 1UL<<31; }
  sqrtBit(30); sqrtBit(29); sqrtBit(28); sqrtBit(27); sqrtBit(26);
  sqrtBit(25); sqrtBit(24); sqrtBit(23); sqrtBit(22); sqrtBit(21);
  sqrtBit(20); sqrtBit(19); sqrtBit(18); sqrtBit(17); sqrtBit(16);
  sqrtBit(15);
#else
  if (n >= 1UL<<30) { n -= 1UL<<30; s = 1UL<<15; }
#endif
  sqrtBit(14); sqrtBit(13); sqrtBit(12); sqrtBit(11); sqrtBit(10);
  sqrtBit(9); sqrtBit(8); sqrtBit(7); sqrtBit(6); sqrtBit(5);
  sqrtBit(4); sqrtBit(3); sqrtBit(2); sqrtBit(1);
  if (n > s<<1) s |= 1UL;

#undef sqrtBit

  return s;
} /* end isqrt */
