// ******************************************************************************
// Header: secureenginesdk_386.go
// Description: Go macros definitions
//
// Author/s: Oreans Technologies 
// (c) 2015 Oreans Technologies
//
// --- File generated automatically from Oreans VM Generator (16/6/2015) ---
// ******************************************************************************

package secureenginesdk;

import (
	"fmt"
)

// ******************************************************************************
//                               Constants definition
// ******************************************************************************

const TIGER_WHITE_START                              = "CustomVM00000100_Start";
const TIGER_WHITE_END                                = "CustomVM00000100_End";
const TIGER_RED_START                                = "CustomVM00000101_Start";
const TIGER_RED_END                                  = "CustomVM00000101_End";
const TIGER_BLACK_START                              = "CustomVM00000102_Start";
const TIGER_BLACK_END                                = "CustomVM00000102_End";
const FISH_WHITE_START                               = "CustomVM00000106_Start";
const FISH_WHITE_END                                 = "CustomVM00000106_End";
const FISH_RED_START                                 = "CustomVM00000108_Start";
const FISH_RED_END                                   = "CustomVM00000108_End";
const FISH_BLACK_START                               = "CustomVM00000110_Start";
const FISH_BLACK_END                                 = "CustomVM00000110_End";
const PUMA_WHITE_START                               = "CustomVM00000112_Start";
const PUMA_WHITE_END                                 = "CustomVM00000112_End";
const PUMA_RED_START                                 = "CustomVM00000114_Start";
const PUMA_RED_END                                   = "CustomVM00000114_End";
const PUMA_BLACK_START                               = "CustomVM00000116_Start";
const PUMA_BLACK_END                                 = "CustomVM00000116_End";
const SHARK_WHITE_START                              = "CustomVM00000118_Start";
const SHARK_WHITE_END                                = "CustomVM00000118_End";
const SHARK_RED_START                                = "CustomVM00000120_Start";
const SHARK_RED_END                                  = "CustomVM00000120_End";
const SHARK_BLACK_START                              = "CustomVM00000122_Start";
const SHARK_BLACK_END                                = "CustomVM00000122_End";
const DOLPHIN_WHITE_START                            = "CustomVM00000134_Start";
const DOLPHIN_WHITE_END                              = "CustomVM00000134_End";
const DOLPHIN_RED_START                              = "CustomVM00000136_Start";
const DOLPHIN_RED_END                                = "CustomVM00000136_End";
const DOLPHIN_BLACK_START                            = "CustomVM00000138_Start";
const DOLPHIN_BLACK_END                              = "CustomVM00000138_End";
const EAGLE_WHITE_START                              = "CustomVM00000146_Start";
const EAGLE_WHITE_END                                = "CustomVM00000146_End";
const EAGLE_RED_START                                = "CustomVM00000148_Start";
const EAGLE_RED_END                                  = "CustomVM00000148_End";
const EAGLE_BLACK_START                              = "CustomVM00000150_Start";
const EAGLE_BLACK_END                                = "CustomVM00000150_End";
const MUTATE_ONLY_START                              = "Mutate_Start";
const MUTATE_ONLY_END                                = "Mutate_End";


// ******************************************************************************
//                                 Bridge Function definition
// ******************************************************************************

func Macro(str string) int {
	if (str == "Hi Bridge") {
		for i := 0; i < 5; i++ {
			  fmt.Printf("text", str[i]);
		}
		for i := 0; i < 5; i++ {
			fmt.Printf("text1", str[i + 1]);
		}
	}
	return  int(str[0]);
}