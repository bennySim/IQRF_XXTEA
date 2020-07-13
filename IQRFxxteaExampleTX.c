// *********************************************************************
// *                      IQRFxxteaExampleTX                           *
// *                       XXTEA encryption                            *
// *********************************************************************
// 
// Intended for:
//    HW: IQRF TR modules TR-52DA and compatibles
//        CK-USB-04 development kit
//    OS: v3.08 or higher
//
// After pressing the button, the program encrypts 
// the contents of bufferRF and sends the message wirelessly

// System header files
#include "include/template-basic.h"

void XXTEA_encryptRF(uns16 keyAddress);

// mandatory, main function for IQRF transceivers
void APPLICATION()
{

    uns8 i;

    // key set to "0123456789012345"
    uns8 key[16];
    for (i=0; i < 16; i++ ) {
    	uns8 val = i%10;
    	writeToRAM(key+i, '0' + val);
    }

    while (1) {
    	// After pressing the button, an encrypted message of length DLEN is sent from bufferRF
		if (buttonPressed) {
			pulseLEDR();
			PIN = 0;
			DLEN = 19; // Set the message length
			// Set the message content
			bufferRF[0] = 'T';
			bufferRF[1] = 'o';
			bufferRF[2] = 'p';
			bufferRF[3] = ' ';
			bufferRF[4] = 's';
			bufferRF[5] = 'e';
			bufferRF[6] = 'c';
			bufferRF[7] = 'r';
			bufferRF[8] = 'e';
			bufferRF[9] = 't';
			bufferRF[10] = ' ';
			bufferRF[11] = 'm';
			bufferRF[12] = 'e';
			bufferRF[13] = 's';
			bufferRF[14] = 's';
			bufferRF[15] = 'a';
			bufferRF[16] = 'g';
			bufferRF[17] = 'e';
			bufferRF[18] = '!';

			XXTEA_encryptRF(key); // enryption of message in bufferRF of 'DLEN' length
			RFTXpacket();	      // send message wirelessly
        		waitDelay(25);
  		}
	}

}	
//#pragma origin __EXTENDED_FLASH

// ============================ HELPER FUNCTIONS FOR ENCRYPTION ==================

/**
 * \brief Initializes the entire array of 'sum' to 0
 *
 * \param[in] sumAddress Address of the first element of the 'sum' array
 */
void sumSetToZero(uns16 sumAddress) {
	uns8 i;
	for (i = 0; i< 4; i++) {
	 writeToRAM(sumAddress + i, 0);
	}
}

/**
 * \brief get lower 'shift' bits
 *
 * \param[in] address	 	 address of 8-bit value to shift to the right by 'shift' bits
 * \param[in] shift		 how many bits are moved to the right
 * \return			 return 'shift' lower bits of value on 'address'
 */
uns8 rshiftOverflow(uns16 address, uns8 shift) {
	uns8 overflow = 0;
	uns8 i, val, midres;
	for (i = 0; i < shift; i++) {
		val = readFromRAM(address) >> i;
		midres = (val & 1) << i;
		overflow += midres;
	}
	return overflow;
}

/**
 * \brief Bitwise right shift of 'shift' bits of value at 'address' and addition
 *  bits that have flowed from higher order values
 *
 * \param[in] address		the address of the value to shift to the right
 * \param[in] shift		the number of bits to shift
 * \param[in] overflow		bits that leaked from higher values
 * \param[out] result		the result is written here
 */
void rshiftWriteOne(uns16 address, uns8 shift, uns8 overflow, uns16 result) {
	uns8 co_shift = 8 - shift;
	overflow <<= co_shift;
	uns8 midres = readFromRAM(address) >> shift;
	midres ^=  overflow;
	writeToRAM(result, midres);
	
}

/**
 * \brief Bit shift to the right of the four 8 bit values ​​as if it were
 * one 32 bit value
 *
 * \param address[in]		address of the first element of the four-element array
 * \param shift[in]		the number of bits by how much to shift to the right
 * \param result[out]		the result of the shift is written here
 */
void rshift(uns16 address, uns8 shift, uns16 result) {
	uns8 i;
	uns8 overflow = 0;
	uns8 overflowTmp = 0;
	for (i = 0; i < 4; i++) {
		if (i != 3) {
			overflow = rshiftOverflow(address+i+1, shift);
		} else {
			overflow = 0;
		}
		rshiftWriteOne(address+i, shift, overflow, result+i);
	}
}


/**
 * \brief Bitwise left shift of one 8 bit value and addition of bits
 * overflowed from higher order
 *
 * \param address[in] 			address of 8 bit value
 * \param shift[in] 			the number of bits by how much is shifted to the left
 * \param overflow[in]			higher order bits
 * \param result[out]		 	the result is written here
 */
void lshiftWriteOne(uns16 address, uns8 shift, uns8 overflow, uns16 result) {
	uns8 midres = readFromRAM(address) << shift;
	writeToRAM(result, midres + overflow);
}

/**
 * \brief The bit shift to the left of the four 8 bit values ​​as if it were
 * one 32 bit value
 *
 * \param address[in] 			the address of the first element from the four-bit array
 * \param shift[in] 			the number of bits by how much is shifted to the left
 * \param result[out] 			the result is written here
 */
void lshift(uns16 address, uns8 shift, uns16 result) {
	uns8 co_shift = (8-shift);
	uns8 i, val;
	uns8 overflow = 0;
	uns8 overflowTmp = 0;
	
	for(i = 0; i < 4; i++) {
		val = readFromRAM(address+i);
	  	overflow = overflowTmp;
		overflowTmp = readFromRAM(address+i) >> co_shift;
		lshiftWriteOne(address+i, shift, overflow, result+i);
	}
}

/**
 * \brief xor two fields with four 8 bit values
 *
 * \param[in] address1		address of the first element of the first array
 * \param[in] address2		address of the first element of the second array
 * \param[out] result 		sem sa uloží výsledok tejto operácie
 */
void xor32(uns16 address1, uns16 address2, uns16 result) {
	uns8 i, tmp, midres;
	for (i=0; i<4; i++) {
		tmp = readFromRAM(address2+i);
		midres = tmp ^ readFromRAM(address1+i);
		writeToRAM(result+i, midres);
	}
}

/**
 * \brief Copy the contents of a four-element array to 'addressDest'
 *
 * \param addressDest[out]	the address of the first element of the four-element array to which it is copied
 * \param addressSource[in] 	the address of the first element of the four-element array from where it is copied
 */
void copy(uns16 addressDest, uns16 addressSource) {
	uns8 i, val;
	for (i = 0; i < 4; i++) {
		val = readFromRAM(i+addressSource);
		writeToRAM(i+addressDest, val);
	}	
}

//#pragma origin __EXTENDED_FLASH_NEXT_PAGE

/**
 * \brief Check if an overflow occurred while summing two 8-bit values
 *
 * \param val1[in]			8 bit value
 * \param val2[in]			8 bit value
 * \param overflow[in]			overflow from lower orders
 * \return				1 if an overflow occurred, 0 if no	
 */
uns8 isOverflowAdd(uns8 val1, uns8 val2, uns8 overflow) {
	uns16 midres = (uns16)val1 + overflow + val2;
	uns8 newoverflow = (midres >> 8);
	return newoverflow;
}

/**
 * \brief Adding two quadrature arrays of 8 bit values ​​as if it were one
 * 32 bit value
 *
 * \param[in,out] address1		address of the first element of the first array, the result is also saved here
 * \param[in] address2			the address of the first element of the second array
 */
void plus(uns16 address1, uns16 address2) {
	uns8 i, tmp2, tmp1;
	uns16 overflow = 0;
	uns16 overflowTmp = 0;
	for (i = 0; i < 4; i++) {
		tmp2 = readFromRAM(address2+i);
		tmp1 = readFromRAM(address1+i);
		overflow = overflowTmp;
		overflowTmp = isOverflowAdd(tmp1, tmp2, overflow);
		writeToRAM(address1+i, (tmp1 + tmp2 + overflow) & 0xFF);
	}
}

/**
 * \brief The main encryption function = (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
 *
 * \param zAddress[in]			address of a four - element array containing the value  z
 * \param yAddress[in]			address of a four - element array containing the value  y
 * \param sumAddress[in]		address of a four - element array containing the value  sum
 * \param p[in]				which round
 * \param e[in]				auxiliary value in the calculation
 * \param keyAddress[in]		address where the key is located
 * \param resultAddress[out]		the result is saved here
 */
void mx(uns16 zAddress, uns16 yAddress, uns16 sumAddress, uns8 p, uns8 e, uns16 resultAddress, uns16 keyAddress) {
	#pragma rambank = 12
	uns8 tmpresult[4], tmpresult1[4];

	rshift(zAddress, 5, resultAddress);

	lshift(yAddress, 2, (uns16)tmpresult);

	xor32(resultAddress, (uns16)tmpresult, resultAddress);

	rshift(yAddress, 3, (uns16)tmpresult);

	lshift(zAddress, 4, (uns16)tmpresult1);

	xor32((uns16) tmpresult, (uns16)tmpresult1, (uns16) tmpresult);

	plus(resultAddress, (uns16)tmpresult);

	xor32(sumAddress, yAddress, (uns16)tmpresult);

	uns8 index = (p&3) ^ e;
	copy((uns16)tmpresult1, (uns24)keyAddress + index*4);
	xor32((uns16)tmpresult1, zAddress, (uns16)tmpresult1);
    	
	plus((uns16)tmpresult, (uns16)tmpresult1);

	xor32(resultAddress, (uns16)tmpresult, resultAddress);
}

// ====================== END OF HELPER FUNCTIONS =============================

/**
 * \brief Encrypt 'DLEN' values from bufferRF using XXTEA encryption algorithm - interval <0, DLEN)
 * 
 * \note  If the message length is not divisible by 4, the DLEN value is adjusted to the nearest multiple of 4.
 *        This length cannot be changed before sending an encrypted message, as this would 
 *        result in incorrect decryption.
 *        If the message is less than 8 bytes, the DLEN is set to 8 - the minimum number
 *        of bytes that can be encrypted using the XXTEA algorithm.
 *
 * \param keyAddress[in]	address where is key located <keyAddress, keyAddress + 16)
 */
void XXTEA_encryptRF(uns16 keyAddress) {
        // Check that the 'DLEN' meets the minimum message length
        if (DLEN < 8) {
		DLEN = 8;
	}

	// Check that the 'DLEN' meets the maximum message length
	if (DLEN > 64) {
		DLEN = 64;
	}	
	
	// If 'DLEN' is not divisible by 4, value is adjusted to the nearest multiple of 4
	if (DLEN % 4 != 0) {
		DLEN = DLEN + (4 - DLEN % 4);
	}

	// Conversion from number of 8 bit values to number of 32 bit values
	uns8 tmpn = DLEN / 4;

	uns16 midres; // in midres are intermediate data
	uns8 z[4], y[4], sum[4], mxResult[4], delta[4];
	uns8 p, rounds, e;
	delta[0] = 0xb9;
	delta[1] = 0x79;
	delta[2] = 0x37;
	delta[3] = 0x9e;
	
	// 'rounds' computation
	midres = 52/tmpn;
	rounds = 6 + midres;

	// 'sum' set to zero	
	sumSetToZero((uns16)sum);

	// z = bufferRF <DLEN-4; DLEN)
	copy((uns16)z, (uns16)bufferRF+DLEN-4);
		do {
			// sum += delta
			plus((uns16)sum, (uns16)delta);
			e = (sum[0] >> 2) & 3;
			// In each iteration, one round of the algorithm runs
			// These values are usend in each round - bufferRF[p], bufferRF[p+1], bufferRF[p-1]
			for (p=0; p < tmpn-1; p++) {
				// y = bufferRF <(p+1)*4; (p+1)*4+4)
				copy((uns16)y, (uns24)bufferRF+(p+1)*4); 
				// The main function of the algorithm - the result is added to bufferRF[p] value
                		mx((uns16)z, (uns16)y, (uns16)sum, p, e, (uns16)mxResult, keyAddress);
				// bufferRF p*4-p*4+4 += mx
				plus((uns24)bufferRF+p*4, (uns16)mxResult);
				// z = bufferRF <p*4; p*4+4)
				copy((uns16)z, (uns24)bufferRF+p*4);
			}
                        // y = bufferRF <0; 4)
			copy((uns16)y, (uns8)bufferRF);
			mx((uns16)z, (uns16)y, (uns16)sum, p, e, (uns16)mxResult, keyAddress);
			plus((uns24)bufferRF+(tmpn-1)*4, (uns16)mxResult);
			// z = bufferRF <(tmpn-1)*4; (tmpn-1)*4+4)
			copy((uns16)z, (uns24)bufferRF+(tmpn-1)*4);
		} while (--rounds);
} 
                                   