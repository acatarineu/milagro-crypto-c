/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

// Use MPIN with only hashed IDs to the server

package main

import (
	"encoding/hex"
	"fmt"

	amclcgo "github.com/miracl/amcl-cgo"
	amclgo "github.com/miracl/amcl-go"
)

func main() {
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)
	fmt.Printf("ID: ")
	amclcgo.MPIN_printBinary(ID)
	fmt.Printf("\n")

	// Epoch time in days
	date := amclcgo.MPIN_today()

	// Epoch time in seconds
	timeValue := amclcgo.MPIN_GET_TIME()

	// PIN variable to create token
	PIN1 := -1
	// PIN variable to authenticate
	PIN2 := -1

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := amclgo.NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	// Generate Master Secret Share 1
	rtn, MS1 := amclcgo.MPIN_RANDOM_GENERATE_WRAP(rng)
	if rtn != 0 {
		fmt.Println("MPIN_RANDOM_GENERATE Error:", rtn)
		return
	}
	fmt.Printf("MS1: 0x")
	amclcgo.MPIN_printBinary(MS1[:])

	// Generate Master Secret Share 2
	rtn, MS2 := amclcgo.MPIN_RANDOM_GENERATE_WRAP(rng)
	if rtn != 0 {
		fmt.Println("MPIN_RANDOM_GENERATE Error:", rtn)
		return
	}
	fmt.Printf("MS2: 0x")
	amclcgo.MPIN_printBinary(MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := amclcgo.MPIN_HASH_ID(ID)

	// Generate server secret share 1
	rtn, SS1 := amclcgo.MPIN_GET_SERVER_SECRET_WRAP(MS1[:])
	if rtn != 0 {
		fmt.Println("MPIN_GET_SERVER_SECRET Error:", rtn)
		return
	}
	fmt.Printf("SS1: 0x")
	amclcgo.MPIN_printBinary(SS1[:])

	// Generate server secret share 2
	rtn, SS2 := amclcgo.MPIN_GET_SERVER_SECRET_WRAP(MS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_GET_SERVER_SECRET Error:", rtn)
		return
	}
	fmt.Printf("SS2: 0x")
	amclcgo.MPIN_printBinary(SS2[:])

	// Combine server secret shares
	rtn, SS := amclcgo.MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G2(SS1, SS2) Error:", rtn)
		return
	}
	fmt.Printf("SS: 0x")
	amclcgo.MPIN_printBinary(SS[:])

	// Generate client secret share 1
	rtn, CS1 := amclcgo.MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_SECRET Error:", rtn)
		return
	}
	fmt.Printf("Client Secret Share CS1: 0x")
	amclcgo.MPIN_printBinary(CS1[:])

	// Generate client secret share 2
	rtn, CS2 := amclcgo.MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_SECRET Error:", rtn)
		return
	}
	fmt.Printf("Client Secret Share CS2: 0x")
	amclcgo.MPIN_printBinary(CS2[:])

	// Combine client secret shares
	CS := make([]byte, amclcgo.G1S)
	rtn, CS = amclcgo.MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G1 Error:", rtn)
		return
	}
	fmt.Printf("Client Secret CS: 0x")
	amclcgo.MPIN_printBinary(CS[:])

	// Generate time permit share 1
	rtn, TP1 := amclcgo.MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_PERMIT Error:", rtn)
		return
	}
	fmt.Printf("TP1: 0x")
	amclcgo.MPIN_printBinary(TP1[:])

	// Generate time permit share 2
	rtn, TP2 := amclcgo.MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_PERMIT Error:", rtn)
		return
	}
	fmt.Printf("TP2: 0x")
	amclcgo.MPIN_printBinary(TP2[:])

	// Combine time permit shares
	rtn, TP := amclcgo.MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G1(TP1, TP2) Error:", rtn)
		return
	}

	// Client extracts PIN1 from secret to create Token
	for PIN1 < 0 {
		fmt.Printf("Please enter PIN to create token: ")
		fmt.Scan(&PIN1)
	}

	rtn, TOKEN := amclcgo.MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: EXTRACT_PIN rtn: %d\n", rtn)
		return
	}
	fmt.Printf("Client Token TK: 0x")
	amclcgo.MPIN_printBinary(TOKEN[:])

	//////   Client   //////

	// Precomputation
	rtn, G1, G2 := amclcgo.MPIN_PRECOMPUTE_WRAP(TOKEN[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_PRECOMPUTE(TOKEN[:], HCID) Error:", rtn)
		return
	}

	for PIN2 < 0 {
		fmt.Printf("Please enter PIN to authenticate: ")
		fmt.Scan(&PIN2)
	}

	// Send U, UT, V, timeValue and Message to server
	var X [amclcgo.EGS]byte
	fmt.Printf("X: 0x")
	amclcgo.MPIN_printBinary(X[:])
	rtn, XOut, Y1, V, U, UT := amclcgo.MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: CLIENT rtn: %d\n", rtn)
		return
	}
	fmt.Printf("Y1: 0x")
	amclcgo.MPIN_printBinary(Y1[:])
	fmt.Printf("XOut: 0x")
	amclcgo.MPIN_printBinary(XOut[:])

	// Send Z=r.ID to Server
	var R [amclcgo.EGS]byte
	fmt.Printf("R: 0x")
	amclcgo.MPIN_printBinary(R[:])
	rtn, ROut, Z := amclcgo.MPIN_GET_G1_MULTIPLE_WRAP(rng, 1, R[:], HCID[:])
	fmt.Printf("ROut: 0x")
	amclcgo.MPIN_printBinary(ROut[:])

	//////   Server   //////
	rtn, HID, HTID, Y2, E, F := amclcgo.MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: SERVER rtn: %d\n", rtn)
	}
	fmt.Printf("Y2: 0x")
	amclcgo.MPIN_printBinary(Y2[:])
	fmt.Printf("HID: 0x")
	amclcgo.MPIN_printBinary(HID[:])
	fmt.Printf("HTID: 0x")
	amclcgo.MPIN_printBinary(HTID[:])

	if rtn != 0 {
		fmt.Printf("Authentication failed Error Code %d\n", rtn)
		err := amclcgo.MPIN_KANGAROO(E[:], F[:])
		if err != 0 {
			fmt.Printf("PIN Error %d\n", err)
		}
		return
	} else {
		fmt.Printf("Authenticated ID: %s \n", IDstr)
	}

	// send T=w.ID to client
	var W [amclcgo.EGS]byte
	fmt.Printf("W: 0x")
	amclcgo.MPIN_printBinary(W[:])
	rtn, WOut, T := amclcgo.MPIN_GET_G1_MULTIPLE_WRAP(rng, 0, W[:], HTID[:])
	fmt.Printf("WOut: 0x")
	amclcgo.MPIN_printBinary(WOut[:])
	fmt.Printf("T: 0x")
	amclcgo.MPIN_printBinary(T[:])

	// Hash all values
	HM := amclcgo.MPIN_HASH_ALL_WRAP(HCID[:], U[:], UT[:], Y2[:], V[:], Z[:], T[:])

	rtn, AES_KEY_SERVER := amclcgo.MPIN_SERVER_KEY_WRAP(Z[:], SS[:], WOut[:], HM[:], HID[:], U[:], UT[:])
	fmt.Printf("Server Key =  0x")
	amclcgo.MPIN_printBinary(AES_KEY_SERVER[:])

	rtn, AES_KEY_CLIENT := amclcgo.MPIN_CLIENT_KEY_WRAP(PIN2, G1[:], G2[:], ROut[:], XOut[:], HM[:], T[:])
	fmt.Printf("Client Key =  0x")
	amclcgo.MPIN_printBinary(AES_KEY_CLIENT[:])

	//////   Server   //////

	// Initialization vector
	IV := amclgo.GENERATE_RANDOM(rng, 12)
	fmt.Printf("IV: 0x")
	amclcgo.MPIN_printBinary(IV[:])

	// header
	HEADER := amclgo.GENERATE_RANDOM(rng, 16)
	fmt.Printf("HEADER: 0x")
	amclcgo.MPIN_printBinary(HEADER[:])

	// Input plaintext
	plaintextStr := "A test message"
	PLAINTEXT1 := []byte(plaintextStr)
	fmt.Printf("String to encrypt: %s \n", plaintextStr)
	fmt.Printf("PLAINTEXT1: 0x")
	amclcgo.MPIN_printBinary(PLAINTEXT1[:])

	// AES-GCM Encryption
	CIPHERTEXT, TAG1 := amclcgo.MPIN_AES_GCM_ENCRYPT(AES_KEY_SERVER[:], IV[:], HEADER[:], PLAINTEXT1[:])
	fmt.Printf("CIPHERTEXT:  0x")
	amclcgo.MPIN_printBinary(CIPHERTEXT[:])
	fmt.Printf("TAG1:  0x")
	amclcgo.MPIN_printBinary(TAG1[:])

	// Send IV, HEADER, CIPHERTEXT and TAG1 to client

	// AES-GCM Decryption
	PLAINTEXT2, TAG2 := amclcgo.MPIN_AES_GCM_DECRYPT(AES_KEY_CLIENT[:], IV[:], HEADER[:], CIPHERTEXT[:])
	fmt.Printf("PLAINTEXT2:  0x")
	amclcgo.MPIN_printBinary(PLAINTEXT2[:])
	fmt.Printf("TAG2:  0x")
	amclcgo.MPIN_printBinary(TAG2[:])
	fmt.Printf("Decrypted string: %s \n", string(PLAINTEXT2))
}
