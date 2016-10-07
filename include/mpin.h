/**
 * @file mpin.h
 * @author Mike Scott
 * @author Kealan McCusker
 * @date 2nd June 2015
 * @brief AMCL MPIN header functions
 *
 * @section LICENSE
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* AMCL MPIN header functions - Definition structures and functions */

#ifndef MPIN_H
#define MPIN_H

#include "amcl.h"

/* Field size is assumed to be greater than or equal to group size */

#define PGS MODBYTES  /**< MPIN Group Size */
#define PFS MODBYTES  /**< MPIN Field Size */
#define PAS 16        /**< MPIN Symmetric Key Size */

#define MPIN_OK                     0   /**< Function completed without error */
/*
#define MPIN_DOMAIN_ERROR          -11
#define MPIN_INVALID_PUBLIC_KEY    -12
#define MPIN_ERROR                 -13
*/
#define MPIN_INVALID_POINT         -14  /**< Point is NOT on the curve */
/*
#define MPIN_DOMAIN_NOT_FOUND      -15
#define MPIN_OUT_OF_MEMORY         -16
#define MPIN_DIV_BY_ZERO           -17
#define MPIN_WRONG_ORDER           -18
*/
#define MPIN_BAD_PIN               -19  /**< Bad PIN number entered */


/* Configure your PIN here */

#define MAXPIN 10000 /**< max PIN */
#define PBLEN 14   /**< max length of PIN in bits */

#define TIME_SLOT_MINUTES 1440 /**< Time Slot = 1 day */
#define HASH_TYPE_MPIN SHA256 /**< Choose Hash function */

/* MPIN support functions */

/* MPIN primitives */

/**	@brief Hash an M-Pin Identity to an octet string
 *
 *  Hash an M-Pin Identity to an octet string.
 *
 *	@param h is the hash type
 *	@param ID an octet containing the identity
 *	@param HID an octet containing the hashed identity
 */
void MPIN_HASH_ID(int h,octet *ID,octet *HID);

/**	@brief Get epoch time as unsigned integer
 *
 *  Get epoch time as unsigned integer.
 *
 *	@return current epoch time in seconds
 */
unsign32 MPIN_GET_TIME(void);

/**	@brief Generate Y=H(TimeValue,xCID), where TimeValue is epoch time, xCID is an octet, and H(.) is a hash function
 *
 *  Generate Y=H(TimeValue,O), where TimeValue is epoch time, O is an octet, and H(.) is a hash function.
 *
 * 	@param sha is the hash type
 *	@param TimeValue is epoch time in seconds
 *	@param xCID is an input octet
 *	@param Y is the output octet
 */
void MPIN_GET_Y(int sha,int TimeValue,octet *xCID,octet *Y);

/**	@brief Extract pin from TOKEN for client's identity CID
 *
 *  Extract pin from TOKEN for client's identity CID.
 *
 * 	@param sha is the hash type
 *	@param CID is the input client identity
 *	@param pin is an input PIN number
 *	@param TOKEN is the client secret from which the pin is to be extracted
 *	@return 0 or an error code
 */
int MPIN_EXTRACT_PIN(int sha,octet *CID,int pin,octet *TOKEN);

/**	@brief Perform client side of the one-pass version of the M-Pin protocol
 *
 *  Perform client side of the one-pass version of the M-Pin protocol.
 *
 *	@param h is the hash type
 *	@param d is input date, in days since the epoch. Set to 0 if Time permits disabled
 *	@param ID is the input client identity
 *	@param R is a pointer to a cryptographically secure random number generator
 *	@param x an output internally randomly generated if R!=NULL, otherwise must be provided as an input
 *	@param pin is the input PIN number
 *	@param T is the input M-Pin token (the client secret with PIN portion removed)
 *	@param V is output = -(x+y)(CS+TP), where CS is the reconstructed client secret, and TP is the time permit
 *	@param U is output = x.H(ID)
 *	@param UT is output = x.(H(ID)+H(d|H(ID)))
 *	@param TP is the input time permit
 *	@param MESSAGE is the message to be signed
 *	@param t is input epoch time in seconds - a timestamp
 *	@param y is output H(t|U) or H(t|UT) if Time Permits enabled
 *	@return 0 or an error code
 *  @note If Time Permits are disabled, set d = 0, and UT is not generated and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is OFF, U is not generated and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is ON, U and UT are both generated
 */
int MPIN_CLIENT(int h,int d,octet *ID,csprng *R,octet *x,int pin,octet *T,octet *V,octet *U,octet *UT,octet *TP, octet* MESSAGE, int t, octet *y);

/**	@brief Perform first pass of the client side of the 3-pass version of the M-Pin protocol
 *
 *  Perform first pass of the client side of the 3-pass version of the M-Pin protocol.
 *
 *	@param h is the hash type
 *	@param d is input date, in days since the epoch. Set to 0 if Time permits disabled
 *	@param ID is the input client identity
 *	@param R is a pointer to a cryptographically secure random number generator
 *	@param x an output internally randomly generated if R!=NULL, otherwise must be provided as an input
 *	@param pin is the input PIN number
 *	@param T is the input M-Pin token (the client secret with PIN portion removed)
 *	@param S is output = CS+TP, where CS=is the reconstructed client secret, and TP is the time permit
 *	@param U is output = x.H(ID)
 *	@param UT is output = x.(H(ID)+H(d|H(ID)))
 *	@param TP is the input time permit
 *	@return 0 or an error code
 *  @note If Time Permits are disabled, set d = 0, and UT is not generated and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is OFF, U is not generated and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is ON, U and UT are both generated
 */
int MPIN_CLIENT_1(int h,int d,octet *ID,csprng *R,octet *x,int pin,octet *T,octet *S,octet *U,octet *UT,octet *TP);

/**	@brief Generate a random group element, a random secret S
 *
 *  Generate a random group element, a random secret S.
 *
 *	@param RNG is a pointer to a cryptographically secure random number generator
 *	@param S is the output random octet
 *	@return 0 or an error code
 */

int MPIN_RANDOM_GENERATE(csprng *RNG,octet *S);

/**	@brief Perform second pass of the client side of the 3-pass version of the M-Pin protocol
 *
 *  Perform second pass of the client side of the 3-pass version of the M-Pin protocol.
 *
 * 	@param X an input, a locally generated random number
 *	@param Y an input random challenge from the server
 *	@param SEC on output = -(X+T).SEC
 *	@return 0 or an error code
 *  @note SEC=-(X+Y)*SEC
 */
int MPIN_CLIENT_2(octet *X,octet *Y,octet *SEC);

/**	@brief Perform server side of the one-pass version of the M-Pin protocol
 *
 *	Perform server side of the one-pass version of the M-Pin protocol.
 *
 *	@param h is the hash type
 *	@param d is input date, in days since the epoch. Set to 0 if Time permits disabled
 *	@param HID is output H(ID), a hash of the client ID
 *	@param HTID is output H(ID)+H(d|H(ID))
 *	@param y is output H(t|U) or H(t|UT) if Time Permits enabled
 *	@param SS is the input server secret
 *	@param U is input from the client = x.H(ID)
 *	@param UT is input from the client= x.(H(ID)+H(d|H(ID)))
 *	@param V is an input from the client
 *	@param E is an output to help the Kangaroos to find the PIN error, or NULL if not required
 *	@param F is an output to help the Kangaroos to find the PIN error, or NULL if not required
 *	@param ID is the input claimed client identity
 *	@param MESSAGE is the message to be signed
 *	@param t is input epoch time in seconds - a timestamp
 *	@return 0 or an error code
 *  @note If Time Permits are disabled, set d = 0, and UT and HTID are not generated and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is OFF, U and HID are not needed and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is ON, U, UT, HID and HTID are all required
 */
int MPIN_SERVER(int h,int d,octet *HID,octet *HTID,octet *y,octet *SS,octet *U,octet *UT,octet *V,octet *E,octet *F,octet *ID,octet *MESSAGE, int t);

/**	@brief Perform first pass of the server side of the 3-pass version of the M-Pin protocol
 *
 *  Perform first pass of the server side of the 3-pass version of the M-Pin protocol.
 *
 *	@param h is the hash type
 *	@param d is input date, in days since the epoch. Set to 0 if Time permits disabled
 *	@param ID is the input claimed client identity
 *	@param HID is output H(ID), a hash of the client ID
 *	@param HTID is output H(ID)+H(d|H(ID)), if no time permit set HTID = NULL
 */
void MPIN_SERVER_1(int h,int d,octet *ID,octet *HID,octet *HTID);

/**	@brief Perform third pass on the server side of the 3-pass version of the M-Pin protocol
 *
 *  Perform third pass on the server side of the 3-pass version of the M-Pin protocol.
 *
 *	@param d is input date, in days since the epoch. Set to 0 if Time permits disabled
 *	@param HID is input H(ID), a hash of the client ID
 *	@param HTID is input H(ID)+H(d|H(ID))
 *	@param y is the input server's randomly generated challenge
 *	@param SS is the input server secret
 *	@param U is input from the client = x.H(ID)
 *	@param UT is input from the client= x.(H(ID)+H(d|H(ID)))
 *	@param V is an input from the client
 *	@param E is an output to help the Kangaroos to find the PIN error, or NULL if not required
 *	@param F is an output to help the Kangaroos to find the PIN error, or NULL if not required
 *	@return 0 or an error code
 *  @note If Time Permits are disabled, set d = 0, and UT and HTID are not needed and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is OFF, U and HID are not needed and can be set to NULL
 *  @note If Time Permits are enabled, and PIN error detection is ON, U, UT, HID and HTID are all required
 */
int MPIN_SERVER_2(int d,octet *HID,octet *HTID,octet *y,octet *SS,octet *U,octet *UT,octet *V,octet *E,octet *F);

/**	@brief Add two members from the group G1
 *
 *  Add two members from the group G1.
 *
 *	@param R1 an input member of G1
 *	@param R2 an input member of G1
 *	@param R an output member of G1
 *	@return 0 or an error code
 *	@note R = R1+R2
 */
int MPIN_RECOMBINE_G1(octet *R1,octet *R2,octet *R);

/**	@brief Add two members from the group G2
 *
 *	@param W1 an input member of G2
 *	@param W2 an input member of G2
 *	@param W an output member of G2
 *	@return 0 or an error code
 *  @note W=W1+W2
 */
int MPIN_RECOMBINE_G2(octet *W1,octet *W2,octet *W);

/**	@brief Pollards kangaroos used to return PIN error
 *
 *  Pollards kangaroos used to return PIN error.
 *
 *	@param E a member of the group GT
 *	@param F a member of the group GT =  E^e
 *	@return 0 if Kangaroos failed, or the PIN error e
 */
int MPIN_KANGAROO(octet *E,octet *F);

/**	@brief Encoding of a Time Permit to make it indistinguishable from a random string
 *
 *  Encoding of a Time Permit to make it indistinguishable from a random string.
 *
 *	@param RNG is a pointer to a cryptographically secure random number generator
 *	@param TP is the input time permit, obfuscated on output
 *	@return 0 or an error code
 *	@note Elliptic curve point E in format (0x04,x,y} is converted to form {0x0-,u,v}
 *	@note u and v are indistinguisible from random strings
 */
int MPIN_ENCODING(csprng *RNG,octet *TP);

/**	@brief Encoding of an obfuscated Time Permit
 *
 *  Encoding of an obfuscated Time Permit.
 *
 *	@param TP is the input obfuscated time permit, restored on output
 *	@return 0 or an error code
 */
int MPIN_DECODING(octet *TP);

/**	@brief Supply today's date as days from the epoch
 *
 *  Supply today's date as days from the epoch.
 *
 *	@return today's date, as number of days elapsed since the epoch
 */
unsign32 MPIN_today(void);

/**	@brief Initialise a Cryptographically Strong Random Number Generator from an octet of raw random data
 *
 *  Initialise a Cryptographically Strong Random Number Generator from an octet of raw random data.
 *
 *	@param R is a pointer to a cryptographically secure random number generator
 *	@param S is an input truly random seed value
 */
void MPIN_CREATE_CSPRNG(csprng *R,octet *S);

/**	@brief Kill a random number generator
 *
	Deletes all internal state
	@param R is a pointer to a cryptographically secure random number generator
 */
void MPIN_KILL_CSPRNG(csprng *R);

/**	@brief Find a random multiple of a point in G1
 *
 *  Find a random multiple of a point in G1.
 *
 *	@param RNG is a pointer to a cryptographically secure random number generator
 *	@param type determines type of action to be taken
 *	@param X an output internally randomly generated if RNG!=NULL, otherwise must be provided as an input
 *	@param G if type=0 a point in G1, else an octet to be mapped to G1
 *	@param W the output =x.G or x.M(G), where M(.) is a mapping
 *	@return 0 or an error code
 *  @note
 */
int MPIN_GET_G1_MULTIPLE(csprng *RNG,int type,octet *X,octet *G,octet *W);

/**	@brief Find a random multiple of a point in G2
 *
 *  Find a random multiple of a point in G2.
 *
 *	@param RNG is a pointer to a cryptographically secure random number generator
 *	@param type determines type of action to betaken
 *	@param X an output internally randomly generated if RNG!=NULL, otherwise must be provided as an input
 *	@param G a point in G2
 *	@param W the output =X.G or (1/x).G
 *	@return 0 or an error code
 */
int MPIN_GET_G2_MULTIPLE(csprng *RNG,int type,octet *x,octet *G,octet *W);

/** @brief Hash the session transcript
 *
 *  Hash the session transcript.
 *
 * 	@param h is the hash type
 *	@param I is the hashed input client ID = H(ID)
 *	@param U is the client output = x.H(ID)
 *	@param CU is the client output = x.(H(ID)+H(T|H(ID)))
 *	@param Y is the server challenge
 *	@param V is the client part response
 *	@param R is the client part response
 *	@param W is the server part response
 *	@param H the output is the hash of all of the above that apply
 */
void MPIN_HASH_ALL(int h,octet *I,octet *U,octet *CU,octet *Y,octet *V,octet *R,octet *W,octet *H);

/**	@brief Create a client secret in G1 from a master secret and the client ID
 *
 *  Create a client secret in G1 from a master secret and the client ID.
 *
 *	@param S is an input master secret
 *	@param ID is the input client identity
 *	@param CS is the full client secret = s.H(ID)
 *	@return 0 or an error code
 *  @note CID is hashed externally
 */
int MPIN_GET_CLIENT_SECRET(octet *S,octet *ID,octet *CS);

/**	@brief Create a Time Permit in G1 from a master secret and the client ID
 *
 *  Create a Time Permit in G1 from a master secret and the client ID.
 *
 * 	@param hash is the hash type
 *	@param date is input date, in days since the epoch.
 *	@param S is an input master secret
 *	@param ID is the input client identity
 *	@param TP is a Time Permit for the given date = s.H(date|H(ID))
 *	@return 0 or an error code
 */
int MPIN_GET_CLIENT_PERMIT(int hash,int date,octet *S,octet *ID,octet *TP);

/**	@brief Create a server secret in G2 from a master secret
 *
 *  Create a server secret in G2 from a master secret.
 *
 *	@param S is an input master secret
 *	@param SS is the server secret = S.Q where Q is a fixed generator of G2
 *	@return 0 or an error code
 */
int MPIN_GET_SERVER_SECRET(octet *S,octet *SS);

/* int MPIN_TEST_PAIRING(octet *,octet *); */

/* For M-Pin Full */

/**	@brief Precompute values for use by the client side of M-Pin Full
 *
 *  Precompute values for use by the client side of M-Pin Full.
 *
 *	@param T is the input M-Pin token (the client secret with PIN portion removed)
 *	@param ID is the input client identity
 *	@param CP is Public Key (or NULL)
 *	@param g1 precomputed output
 *	@param g2 precomputed output
 *	@return 0 or an error code
 */
int MPIN_PRECOMPUTE(octet *T,octet *ID,octet *CP,octet *g1,octet *g2);

/**	@brief Calculate Key on Server side for M-Pin Full
 *
 *  Calculate Key on Server side for M-Pin Full.
 *
 *	Uses xCID internally for the key calculation, unless not available in which case xID is used
 *	@param sha is the hash type
 *	@param Z is the input Client-side Diffie-Hellman component
 *	@param SST is the input server secret
 *	@param W is an input random number generated by the server
 *	@param H is an input, hash of the protocol transcript
 *	@param HID is the hashed input client HID = H(ID)
 *	@param xID is input from the client = x.H(ID)
 *	@param xCID is input from the client= x.(H(ID)+H(d|H(ID)))
 *	@param SK is the output calculated shared key
 *	@return 0 or an error code
 */
int MPIN_SERVER_KEY(int sha,octet *Z,octet *SST,octet *W,octet *H,octet *HID,octet *xID,octet *xCID,octet *SK);

/**	@brief Calculate Key on Client side for M-Pin Full
 *
 *  Calculate Key on Client side for M-Pin Full.
 *
 * 	@param h is the hash type
 *	@param g1 precomputed input
 *	@param g2 precomputed input
 *	@param pin is the input PIN number
 *	@param r is an input, a locally generated random number
 *	@param x is an input, a locally generated random number
 *	@param p is an input, hash of the protocol transcript
 *	@param T is the input Server-side Diffie-Hellman component
 *	@param K is the output calculated shared key
 *	@return 0 or an error code
 *  @note wCID = w.(A+AT)
 */
int MPIN_CLIENT_KEY(int h,octet *g1,octet *g2,int pin,octet *r,octet *x,octet *p,octet *T,octet *K);

/**	@brief AES-GCM Encryption of octets
 *
 *  AES-GCM Encryption of octets.
 *
 *	@param K  AES key
 *	@param IV Initialization vector
 *	@param H Header
 *	@param P Plaintext
 *	@param C Ciphertext
 *	@param T Checksum
 */
void MPIN_AES_GCM_ENCRYPT(octet *K,octet *IV,octet *H,octet *P,octet *C,octet *T);

/**	@brief AES-GCM Decryption
 *
 *  AES-GCM Decryption.
 *
 *	@param K  AES key
 *	@param IV Initialization vector
 *	@param H Header
 *	@param P Plaintext
 *	@param C Ciphertext
 *	@param T Checksum
 */
void MPIN_AES_GCM_DECRYPT(octet *K,octet *IV,octet *H,octet *C,octet *P,octet *T);

/**	@brief Return the field size
 *
 *  Return the field size.
 *
 *	@return Field size
 */
int MPIN_FS();

/**	@brief Return the group size
 *
 *  Return the group size.
 *
 *	@return Group size
 */
int MPIN_GS();

/**	@brief HMAC of message M using key K to create tag of length len in octet tag
 *
	IEEE-1363 MAC1 function. Uses SHA256 internally.
	@param M input message octet
	@param K input encryption key
	@param len is output desired length of HMAC tag
	@param tag is the output HMAC
	@return 0 for bad parameters, else 1
 */
//int HMAC(octet *M,octet *K,int len,octet *tag);

/**	@brief Password Based Key Derivation Function - generates key K from password, salt and repeat counter
 *
	PBKDF2 Password Based Key Derivation Function. Uses SHA256 internally.
	@param P input password
	@param S input salt
	@param rep Number of times to be iterated.
	@param len is output desired length of key
	@param K is the derived key
 */
//void PBKDF2(octet *P,octet *S,int rep,int len,octet *K);

#endif
