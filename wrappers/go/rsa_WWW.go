/**
 * @file rsa.go
 * @author Alessandro Budroni
 * @brief Wrappers for RSA functions
 *
 * LICENSE
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

package amcl

/*
#cgo CFLAGS:  -std=c99 -O3 -I. -I@CMAKE_INSTALL_PREFIX@/include -DCMAKE
#cgo LDFLAGS: -L. -L@CMAKE_INSTALL_PREFIX@/lib -lamcl_rsa_WWW -lamcl_core
#include <stdio.h>
#include <stdlib.h>
#include "amcl.h"
#include "randapi.h"
#include "rsa.h"
#include "utils.h"
*/
import "C"

// RSA Constant
const MAX_RSA_BYTES int = int(C.MAX_RSA_BYTES) // MAX_RSA_BYTES is the maximum RSA level of security supported - 4096
const RFS_WWW int = int(C.RFS_WWW)                     // RFS_WWW is the RSA Public Key Size in bytes
const FFLEN_WWW int = int(C.FFLEN_WWW)                 // FFLEN_WWW consists in 2^n multiplier of BIGBITS to specify supported Finite Field size, e.g 2048=256*2^3 where BIGBITS=256

// RSAKeyPair generates an RSA key pair
func RSAKeyPair_WWW(RNG *RandNG, e int32, P []byte, Q []byte) (C.rsa_private_key_WWW, C.rsa_public_key_WWW) {
	PStr := string(P)
	POct := GetOctet(PStr)
	defer OctetFree(&POct)
	QStr := string(Q)
	QOct := GetOctet(QStr)
	defer OctetFree(&QOct)
	eVal := C.int32_t(e)
	var RSA_PubKey C.rsa_public_key_WWW
	var RSA_PrivKey C.rsa_private_key_WWW

	C.RSA_WWW_KEY_PAIR(RNG.csprng(), eVal, &RSA_PrivKey, &RSA_PubKey, &POct, &QOct)
	return RSA_PrivKey, RSA_PubKey
}

// PKCS15 (PKCS 1.5) - padding of a message to be signed
func PKCS15(hashType int, M []byte) (errorCode int, C []byte) {
	MStr := string(M)
	MOct := GetOctet(MStr)
	defer OctetFree(&MOct)
	COct := GetOctetZero(RFS_WWW)
	defer OctetFree(&COct)

	rtn := C.PKCS15(C.int(hashType), &MOct, &COct)
	errorCode = int(rtn)
	C = OctetToBytes(&COct)
	return errorCode, C[:]
}

// OAEPencode encodes the message for encryption
func OAEPencode(hashType int, M []byte, RNG *RandNG, P []byte) (errorCode int, F []byte) {
	MStr := string(M)
	MOct := GetOctet(MStr)
	defer OctetFree(&MOct)
	PStr := string(P)
	POct := GetOctet(PStr)
	defer OctetFree(&POct)
	FOct := GetOctetZero(RFS_WWW)
	defer OctetFree(&FOct)

	rtn := C.OAEP_ENCODE(C.int(hashType), &MOct, RNG.csprng(), &POct, &FOct)
	errorCode = int(rtn)
	F = OctetToBytes(&FOct)
	return errorCode, F[:]
}

// OAEPdecode decodes message M after decryption, F is the decoded message
func OAEPdecode(hashType int, P []byte, M []byte) (int, []byte) {
	MStr := string(M)
	MOct := GetOctet(MStr)
	defer OctetFree(&MOct)
	PStr := string(P)
	POct := GetOctet(PStr)
	defer OctetFree(&POct)

	rtn := C.OAEP_DECODE(C.int(hashType), &POct, &MOct)
	errorCode := int(rtn)
	M = OctetToBytes(&MOct)
	return errorCode, M[:]
}

// RSA_WWW_ENCRYPT encrypts F with the public key
func RSA_WWW_ENCRYPT(RSA_PubKey *C.rsa_public_key_WWW, F []byte) (G []byte) {
	FStr := string(F)
	FOct := GetOctet(FStr)
	defer OctetFree(&FOct)
	GOct := GetOctetZero(RFS_WWW)
	defer OctetFree(&GOct)

	C.RSA_WWW_ENCRYPT(RSA_PubKey, &FOct, &GOct)
	G = OctetToBytes(&GOct)
	return G[:]
}

// RSA_WWW_DECRYPT decrypts G with the private key
func RSA_WWW_DECRYPT(RSA_PrivKey *C.rsa_private_key_WWW, G []byte) (F []byte) {
	GStr := string(G)
	GOct := GetOctet(GStr)
	defer OctetFree(&GOct)
	FOct := GetOctetZero(RFS_WWW)
	defer OctetFree(&FOct)

	C.RSA_WWW_DECRYPT(RSA_PrivKey, &GOct, &FOct)
	F = OctetToBytes(&FOct)
	return F[:]
}

// RSA_PRIVATE_KEY_KILL_WWW destroys an RSA private Key
func RSA_PRIVATE_KEY_KILL_WWW(RSA_PrivKey *C.rsa_private_key_WWW) {
	C.RSA_PRIVATE_KEY_KILL_WWW(RSA_PrivKey)
}
