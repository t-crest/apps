/***********************************************************
*										
*	PROJET : VIP4F				
*										
*	MODULE : SinCosH1.c					
*										
*	AUTEUR : FR					
*										
*	CREATION : 06/03/2009
*										
************************************************************
*                                                           
*      DESCRIPTION       :SinCos H1									
*
************************************************************
*
*  Fx=1/8(f0+f1+f2/2-f4/2-f5-f6-f7-f8/2+f10/2+f11)          
*  Fy=1/8(f1/2+f2+f3+f4+f5/2-f7/2-f8-f9-f10-f11/2)          
         avec f0 dernier �chantillon acquis                 
*              f11 �chantillon le plus ancien               
*  Fx=1/8((f0+f1-f5-f6-f7+f11)+1/2(f2-f4-f8+f10))           
*  Fy=1/8((f2+f3+f4-f8-f9-f10)+1/2(f1+f5-f7-f11))          
*                                                           
*  Fx=1/8(((f1-f7)-(f5-f11)+f0-f6)+1/2((f2-f8)-(f4-f10)))   
*  Fy=1/8(((f2-f8)+(f4-f10)+f3-f9)+1/2((f1-f7)+(f5-f11))) 
*
*  module� = Fx� = Fy�	
*
***********************************************************/


//**********************************************************
//
//			INCLUDES
//
//**********************************************************

#include "../include/typedef.h"
#include "../include/define.h"

//**********************************************************
//
//			VARIABLES
//
//**********************************************************


void TRS_EchantillonSinCosH1(short *ptr1 , S_TRS_FXFY *ptr2)
{
	short Tmp1,Tmp2, Fx, Fy;
	//int i;
	
	// division des echantillons par 3 car on a fait un cumul de 3 �chantillons tous les Te/3 : 
	// il faut maintenent faire la moyenne
	//for (i = 0; i< 12; i++)
	//{
	//	*(ptr1+i)= *(ptr1+i)/3;
	//}
	//Fx=1/8(((f1-f7)-(f5-f11)+f0-f6)+1/2((f2-f8)-(f4-f10)))
	//Fy=1/8(((f2-f8)+(f4-f10)+f3-f9)+1/2((f1-f7)+(f5-f11)))
 
  /* Calcul de f2 - f8 */                             
  Tmp1 = *(ptr1+2)-*(ptr1+8);                       

  /* Calcul de f4 - f10 */                            
  Tmp2 = *(ptr1+4)-*(ptr1+10);                       
                                                            
  /* Calcul de 1/2(f2-f8)-(f4-f10)) pour fx */        
  Fx = (Tmp1 - Tmp2) >> 1;                            
                                                         
  /* Calcul de (f4-f10)+(f2-f8)) pour fy */       
  Fy = Tmp2 + Tmp1;                                
                                                           
  /* calcul de (f4-f10+f2-f8)+f3 pour fy */         
  Fy += *(ptr1+3);                                   
                                                          
  /* calcul de (f4-f10+f2-f8+f3) - f9 pour fy */    
  Fy -= *(ptr1+9);                                   
                                                         
  /* Calcul de f1 - f7 */                            
  Tmp1 = *(ptr1+1)-*(ptr1+7);                        
                                                          
  /* Calcul de f5 - f11 */                          
  Tmp2 = *(ptr1+5)-*(ptr1+11);                      
                                                         
  /* Calcul de (f1-f7)-(f5-f11)+(1/2(f2-f8)-(f4-f10)) pour fx */ 
  Fx = Tmp1 - Tmp2 + Fx;                              
                                                          
  /* Calcul de (f1-f7)+(f5-f11)+(1/2(f2-f8)-(f4-f10)) pour fy */ 
  Fy = ((Tmp1 + Tmp2)>>1) + Fy;                      
                                                           
  Fx = Fx + (*(ptr1)-*(ptr1+6));                    
                                                            
  /* Division de fx et fy par 8 */                   
  Fx = (Fx >> 3);                                    
  Fy = (Fy >> 3); 
	/* Moyenne avec la valeur pr�c�dentes de Fx et Fy */
	ptr2->ComposanteFx = (Fx >>1 ) + (ptr2->ComposanteFx_old >>1);     
	ptr2->ComposanteFy = (Fy >>1 ) + (ptr2->ComposanteFy_old >>1);     
	/* Sauvegarde pour cycle suivant */
	ptr2->ComposanteFx_old = Fx;		
	ptr2->ComposanteFy_old = Fy;

}

int m_CosTabs[12] = { -591, -683, -591, -341, 0,    341,  591,  683, 591, 341, 0,  -341 };
int m_SinTabs[12] = { 341,  0,    -341, -591, -683, -591, -341, 0  , 341, 591, 683, 591 };

/**
 * Result of the computation of e(t-i.Te).bxi
 */
long m_ResCos[15][4]; 

/**
 * Result of the computation of e(t-i.Te).byi
 */
long m_ResSin[15][4];

/**
 * Current index in the results table (m_ResCos, m_ResSin and m_ResQuality).
 */
int m_ResIndex[4];

int m_TabIndex[4];

void TRS_EchantillonSinCosH12(int *ptr1 , long long *ptr2, int aVoie)
{
	int i;
	
	// First step, we compute the new 8 values
	//    => e(t-i.Te).bxi
	//    => e(t-i.Te).byi
	//inverseTab(ptr1, 12);

	m_TabIndex[aVoie] += 8;
	if ( m_TabIndex[aVoie] > 12-1 ) m_TabIndex[aVoie] = m_TabIndex[aVoie] -12;

	if (m_ResIndex[aVoie] < 0) m_ResIndex[aVoie] = 15-1;

	for ( i = 0; i < 8; i++ )
	{
		// e(t-i.Te).bxi
		m_ResCos[m_ResIndex[aVoie]][aVoie] = (int)(ptr1[i]) * m_CosTabs[m_TabIndex[aVoie]];
		// e(t-i.Te).byi
		m_ResSin[m_ResIndex[aVoie]][aVoie] = (int)(ptr1[i]) * m_SinTabs[m_TabIndex[aVoie]];

		// YTE We have to compute from newest to the oldest
		// 0 = most recent sample
		m_TabIndex[aVoie]++;// -- because we compute from the newest to the oldest
		if ( m_TabIndex[aVoie] > 12-1 ) m_TabIndex[aVoie] = 0;
		
		m_ResIndex[aVoie]++;
		if ( m_ResIndex[aVoie] >= 15 ) m_ResIndex[aVoie] = 0; // cf. sizeof(m_ResXxx} = [15][m_NbCoord]
	}

	// Computation of the H1 harmonic value
	long long FinalResCos[4] = { 0, 0, 0, 0 }; // Cf m_NbCoord <= 4
	long long FinalResSin[4] = { 0, 0, 0, 0 }; // Cf m_NbCoord <= 4
	
    // Second step, we compute the sums
	//c(t) = 1 / 2 * [
	//        e(t-14Te).b2 + e(t-13Te).b1 + e(t-12Te).b0 +
	//        2 * (e(t-11Te).b11 + e(t-10Te).b10 + ... + e(t-4Te).b4 + e(t-3Te).b3) +
	//        e(t-2Te).b2 + e(t-Te).b1 + e(t).b0
	int Index = m_ResIndex[aVoie] - 8; // -- because m_ResIndex point to the next available res
	if (Index < 0) Index = 15 + Index;
	m_ResIndex[aVoie] -= 1;

	// - from t-2Te to t
	for ( i = 0; i <= 2; i++ )
	{  
		// 1/2 * [ e(t-2Te).b2 + e(t-Te).b1 + e(t).b0 ]
		//quality |= m_ResQuality[Index];
		FinalResCos[aVoie] += m_ResCos[Index][aVoie]>>1;
		FinalResSin[aVoie] += m_ResSin[Index][aVoie]>>1;

		Index++;
		if (Index >= 15) Index = 0;
	}
	// - from t-11Te to t-3Te

	for ( i = 3; i <= 11; i++ )
	{  
		// 1/2 * [ 2 * (e(t-11Te).b11 + e(t-10Te).b10 + ... + e(t-4Te).b4 + e(t-3Te).b3) ]
		//quality |= m_ResQuality[Index];
		FinalResCos[aVoie] += m_ResCos[Index][aVoie];
		FinalResSin[aVoie] += m_ResSin[Index][aVoie];

		Index++;
		if (Index >= 15) Index = 0;
	}
	// - from t-14Te to t-12Te
	for ( i = 12; i <= 14; i++ )
	{  
		// 1/2 * [ e(t-14Te).b2 + e(t-13Te).b1 + e(t-12Te).b0 ]
		//quality |= m_ResQuality[Index];
		FinalResCos[aVoie] += m_ResCos[Index][aVoie]>>1; // >>1 for 1/2
		FinalResSin[aVoie] += m_ResSin[Index][aVoie]>>1; // >>1 for 1/2

		Index++;
		if (Index >= 15) Index = 0;
	}

	// TODO Check format :  tPrjSInt64* pSampleValues = (tPrjSInt64*) &m_Values[ m_NbCoord * NextSampleValue << ( m_Format&0x0f )];
	// XxxTab has been size in order to have e(t-iTe).bi on 32 bits
    // FinalResCos is the sum of 9 e(t-iTe).bi + 6 e(t-iTe).bi / 2 = 12 e(t-iTe).bi
    // So FinalResCos is coded on 36 bits + 1 bit for the sum cx2 + cy2
    // We need then finally to save 5 bits to be able to make the final computation

	// Due to Visual .Net compiler bug we can not keep this code
	//tPrjSInt32 cx = (tPrjSInt32) (FinalResCos[k]>>5);
	//tPrjSInt64 cx2 = cx*cx;
	//tPrjSInt32 cy = (tPrjSInt32) (FinalResSin[k]>>5);
	//tPrjSInt64 cy2 = cy*cy;
      
    long long cx2 = FinalResCos[aVoie]>>5;
    cx2 *= cx2;

	long long cy2 = FinalResSin[aVoie]>>5;
	cy2 *= cy2;

	// TODO Check the type (64bit).
	*ptr2 = (long long) ( (cx2+cy2+((long long)1<<(2*(12-5)-1))) >> 2*(12-5) ); // 12 for 2^12 of the XxxTab
}

