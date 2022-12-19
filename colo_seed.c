/*
MIT License
Copyright (c) 2022 CasualPokePlayer
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// gcc colo_seed.c -Wall -Wextra -Wpedantic -O3 -static -o colo_seed.exe

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t u32;

// user configurable...
#define TARGET_SEED 0xDD9B8CA5

// I forget how I came up with this
#define RNG_START 0x24113DB6

// note this is more an average
#define TB_TICKS_IN_SEC 40500000

int main(void) {
	u32 seed = RNG_START;
	for (u32 i = 0; i < 134217728; i++) {
		u32 seedUpper = seed + 64;
		u32 seedLower = seed - 64;
		if (TARGET_SEED >= seedLower && TARGET_SEED <= seedUpper) {
			printf("Potential seed hit at %d seconds (base seed 0x%08X, offset %d)\n", i, seed, seed - TARGET_SEED);
		}
		seed += TB_TICKS_IN_SEC;
	}

	return 0;
}
