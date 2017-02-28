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

package main

import (
	"encoding/hex"
	"fmt"
	"sync"
	"time"

	"github.com/miracl/amcl-go-wrapper"
)

const numRoutines = 1000

var (
	HASH_TYPE_MPIN = mpin.SHA256
	wg             sync.WaitGroup
)

func run(rng *mpin.MPinRNG) {

	// // Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := mpin.Today()

	// // Epoch time in seconds
	timeValue := mpin.GetTime()

	// // PIN variable to create token
	PIN := 1111

	// // Message to sign
	// var MESSAGE []byte
	MESSAGE := []byte("test sign message")

	// Generate Master Secret Share 1
	rtn, MS1 := mpin.RandomGenerate(rng)
	if rtn != 0 {
		fmt.Println("RandomGenerate Error:", rtn)
		return
	}
	// fmt.Printf("START %v; MS1: %x\n", x, MS1)

	// Generate Master Secret Share 2
	rtn, MS2 := mpin.RandomGenerate(rng)
	if rtn != 0 {
		fmt.Println("RandomGenerate Error:", rtn)
		return
	}

	// // Either Client or TA calculates Hash(ID)
	HCID := mpin.HashId(HASH_TYPE_MPIN, ID)

	// // Generate server secret share 1
	rtn, SS1 := mpin.GetServerSecret(MS1[:])
	if rtn != 0 {
		fmt.Println("GetServerSecret Error:", rtn)
		return
	}

	// // Generate server secret share 2
	rtn, SS2 := mpin.GetServerSecret(MS2[:])
	if rtn != 0 {
		fmt.Println("GetServerSecret Error:", rtn)
		return
	}

	// Combine server secret shares
	rtn, SS := mpin.RecombineG2(SS1[:], SS2[:])
	if rtn != 0 {
		fmt.Println("RecombineG2(SS1, SS2) Error:", rtn)
		return
	}

	// Generate client secret share 1
	rtn, CS1 := mpin.GetClientSecret(MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("GetClientSecret Error:", rtn)
		return
	}

	// Generate client secret share 2
	rtn, CS2 := mpin.GetClientSecret(MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("GetClientSecret Error:", rtn)
		return
	}

	// Combine client secret shares
	CS := make([]byte, mpin.G1S)
	rtn, CS = mpin.RecombineG1(CS1[:], CS2[:])
	if rtn != 0 {
		fmt.Println("RecombineG1 Error:", rtn, SS, CS)
		return
	}

	// Generate time permit share 1
	rtn, TP1 := mpin.GetClientPermit(HASH_TYPE_MPIN, date, MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("GetClientPermit Error:", rtn)
		return
	}

	// // Generate time permit share 2
	rtn, TP2 := mpin.GetClientPermit(HASH_TYPE_MPIN, date, MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("GetClientPermit Error:", rtn)
		return
	}

	// Combine time permit shares
	rtn, TP := mpin.RecombineG1(TP1[:], TP2[:])
	if rtn != 0 {
		fmt.Println("RecombineG1(TP1, TP2) Error:", rtn, TP)
		return
	}
	rtn, TOKEN := mpin.ExtractPIN(HASH_TYPE_MPIN, ID[:], PIN, CS[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: EXTRACT_PIN rtn: %d\n", rtn)
		return
	}

	// //////   Client   //////
	// // Send U, UT, V, timeValue and Message to server
	var X [mpin.PGS]byte
	rtn, _, _, SEC, U, UT := mpin.Client(HASH_TYPE_MPIN, date, ID[:], rng, X[:], PIN, TOKEN[:], TP[:], MESSAGE[:], timeValue)
	if rtn != 0 {
		fmt.Printf("FAILURE: CLIENT rtn: %d\n", rtn, SEC, U, UT)
		return
	}

	//////   Server   //////
	rtn, _, _, _, E, F := mpin.Server(HASH_TYPE_MPIN, date, timeValue, SS[:], U[:], UT[:], SEC[:], ID[:], MESSAGE[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: SERVER rtn: %d\n", rtn)
	}
	if rtn != 0 {
		fmt.Printf("Authentication failed Error Code %d\n", rtn)
		err := mpin.Kangaroo(E[:], F[:])
		if err != 0 {
			fmt.Printf("PIN Error %d\n", err)
		}
		return
	}

	wg.Done()
}

func main() {
	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := mpin.CreateCSPRNG(seed)

	wg = sync.WaitGroup{}

	fmt.Printf("Stating %v go routines...\n", numRoutines)
	wg.Add(1000)
	t := time.Now()
	for x := 0; x < 1000; x++ {
		go run(&rng)
	}
	wg.Wait()

	fmt.Printf("Done: %v \n", time.Now().Sub(t).Seconds())
}
