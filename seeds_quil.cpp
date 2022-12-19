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

// g++ seeds_quil.cpp -Wall -Wextra -Wpedantic -O3 -static -o seeds_quil.exe

// some of this code is taken from https://github.com/aldelaro5/GC-pokemon-RNG-manipulation-assistant (follows MIT licence)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Alogorithm that returns (base^exp) mod 2^32 in the complexity O(log n)
static inline u32 modpow32(u32 base, u32 exp) {
	u32 result = 1;
	while (exp > 0) {
		if (exp & 1)
			result = result * base;
		base = base * base;
		exp >>= 1;
	}
	return result;
}

// The LCG used in both Pokemon games
static inline u32 LCG(u32& seed) {
	seed = seed * 0x343fd + 0x269EC3;
	return seed;
}

// Apply the LCG n times in O(log n) complexity
static inline u32 LCGn(u32 seed, const u32 n) {
	u32 ex = n - 1;
	u32 q = 0x343fd;
	u32 factor = 1;
	u32 sum = 0;
	while (ex > 0) {
		if (!(ex & 1)) {
			sum = sum + (factor * modpow32(q, ex));
			ex--;
		}
		factor *= (1 + q);
		q *= q;
		ex /= 2;
	}
	seed = (seed * modpow32(0x343fd, n)) + (sum + factor) * 0x269EC3;
	return seed;
}

struct PokemonProperties {
	u32 hpIV = 0;
	u32 atkIV = 0;
	u32 defIV = 0;
	u32 spAtkIV = 0;
	u32 spDefIV = 0;
	u32 speedIV = 0;
	u32 hiddenPowerTypeIndex = 0;
	u32 hiddenPowerPower = 0;
	u32 natureIndex = 0;
};

static inline u32 getPidGender(const u8 genderRatio, const u32 pid) {
	return genderRatio > (pid & 0xff) ? 1 : 0;
}

static inline bool isPidShiny(const u16 TID, const u16 SID, const u32 PID) {
	return ((TID ^ SID ^ (PID & 0xFFFF) ^ (PID >> 16)) < 8);
}

static inline void doStarter(PokemonProperties& starter, u32& seed, const u32 TID, const u32 SID) {
	LCG(seed);
	LCG(seed);

	{
		// HP, ATK, DEF IV
		LCG(seed);
		starter.hpIV = (seed >> 16) & 31;
		starter.atkIV = (seed >> 21) & 31;
		starter.defIV = (seed >> 26) & 31;
		// SPEED, SPATK, SPDEF IV
		LCG(seed);
		starter.speedIV = (seed >> 16) & 31;
		starter.spAtkIV = (seed >> 21) & 31;
		starter.spDefIV = (seed >> 26) & 31;
	}

	// Ability, doesn't matter
	LCG(seed);

	{
		int typeSum = (starter.hpIV & 1) + 2 * (starter.atkIV & 1) + 4 * (starter.defIV & 1) +
		8 * (starter.speedIV & 1) + 16 * (starter.spAtkIV & 1) +
		32 * (starter.spDefIV & 1);
		starter.hiddenPowerTypeIndex = typeSum * 15 / 63;
		int powerSum = ((starter.hpIV & 2) >> 1) + 2 * ((starter.atkIV & 2) >> 1) +
		4 * ((starter.defIV & 2) >> 1) + 8 * ((starter.speedIV & 2) >> 1) +
		16 * ((starter.spAtkIV & 2) >> 1) + 32 * ((starter.spDefIV & 2) >> 1);
		starter.hiddenPowerPower = (powerSum * 40 / 63) + 30;
	}

	u32 PID;
	while (true) {
		// A personality ID is generated as candidate, high then low 16 bits
		u32 hId = LCG(seed) >> 16;
		u32 lId = LCG(seed) >> 16;
		PID = (hId << 16) | (lId);

        u8 idGender = getPidGender(0x1f, PID);
        if (idGender != 0)
			continue;

		// This is apparently preventing a shiny personality ID...in the most convoluted way ever!
		if (!isPidShiny(TID, SID, PID))
			break;
	}

	starter.natureIndex = PID % 25;
}

//#define DEBUG

#ifdef DEBUG

#define DEBUG_PRINT_SEED() do { \
	printf("DEBUG - seed 0x%08X\n", seed); \
	fflush(stdout); \
} while (0) \

#else

#define DEBUG_PRINT_SEED() do {} while (0)

#endif

static inline void doSearch(u32 seed) {
	u32 const originalSeed = seed;
	{
		seed = LCGn(seed, 1000);
		u32 TID = LCG(seed) >> 16;
		u32 SID = LCG(seed) >> 16;
		for (u32 i = 0; i < 2; i++) {
			seed = LCGn(seed, 5);
			while (true) {
				u32 hId = LCG(seed) >> 16;
				u32 lId = LCG(seed) >> 16;
				u32 PID = (hId << 16) | (lId);

				u8 idGender = getPidGender(0x1f, PID);
				if (idGender != 0) {
					continue;
				}

				if (!isPidShiny(TID, SID, PID)) {
					break;
				}
			}
		}

		LCG(seed);
		LCG(seed);
	}

	{
		seed = LCGn(seed, 1000);
		u32 TID = LCG(seed) >> 16;
		u32 SID = LCG(seed) >> 16;
		for (u32 i = 0; i < 2; i++) {
			seed = LCGn(seed, 5);
			while (true) {
				u32 hId = LCG(seed) >> 16;
				u32 lId = LCG(seed) >> 16;
				u32 PID = (hId << 16) | (lId);

				u8 idGender = getPidGender(0x1f, PID);
				if (idGender != 0) {
					continue;
				}

				if (!isPidShiny(TID, SID, PID)) {
					break;
				}
			}
		}
	}

	// 600 rng call loop at beginning
	for (u32 i = 0; i < 600; i++) {
		LCG(seed);
		u16 hiSeed = seed >> 16;
		// floatless version of static_cast<double>(hiSeed) / 65536.0 < 0.1
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}
	}

	// extra advancement here
	{
		LCG(seed);
		u16 hiSeed = seed >> 16;
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}
	}

	// 1531 - earliest frame entering naming screen
	// 1567 - naming keyboard starts render
	// 1632 - naming screen keyboard no longer rendered
	// 1663 - end rng for naming screen
	for (u32 i = 0; i < (1567 - 1531); i++) {
		LCG(seed);
		u16 hiSeed = seed >> 16;
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}
	}

	// note: framecount is taken at 60 fps, rng advances here at 30 fps
	// also +1 due to lag frame?
	for (u32 i = 0; i < ((1632 - 1567) / 2) + 1; i++) {
		LCG(seed);
		u16 hiSeed = seed >> 16;
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}

		// extra rng call for keyboard
		LCG(seed);
		hiSeed = seed >> 16;
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}
	}

	for (u32 i = 0; i < (1663 - 1632); i++) {
		LCG(seed);
		u16 hiSeed = seed >> 16;
		if ((hiSeed * 10) < 65536) {
			seed = LCGn(seed, 4);
		}
	}

	seed = LCGn(seed, 1000); // 1000 rng call batch lol
	u32 TID = LCG(seed) >> 16;
	u32 SID = LCG(seed) >> 16;
	PokemonProperties starter;

	// umbreon
	doStarter(starter, seed, TID, SID);
	// need 10+ attack if negative attack nature
	if (starter.atkIV < 10 && (starter.natureIndex == 5 || starter.natureIndex == 10 || starter.natureIndex == 15 || starter.natureIndex == 20)) {
		return;
	}

	// espeon
	doStarter(starter, seed, TID, SID);
	// rash
	if (starter.natureIndex != 19) {
		return;
	}
	// hidden power grass, 61+
	if (starter.hiddenPowerTypeIndex != 10 || starter.hiddenPowerPower < 61) {
		return;
	}
	// hp 21+, atk 30+, def 26+, spatk 30, speed 27+
	if (starter.hpIV < 21 || starter.atkIV < 30 || starter.defIV < 26 || starter.spAtkIV != 30 || starter.speedIV < 27) {
		return;
	}

	seed = originalSeed;
	seed = LCGn(seed, 1248257);
	PokemonProperties quilava;
	for (u32 i = 0; i < 5000; i++) {
		u32 oldSeed = seed;

		// HP, ATK, DEF IV
		LCG(seed);
		quilava.hpIV = (seed >> 16) & 31;
		quilava.atkIV = (seed >> 21) & 31;
		quilava.defIV = (seed >> 26) & 31;
		// SPEED, SPATK, SPDEF IV
		LCG(seed);
		quilava.speedIV = (seed >> 16) & 31;
		quilava.spAtkIV = (seed >> 21) & 31;
		quilava.spDefIV = (seed >> 26) & 31;

		// ability
		LCG(seed);

		// pid
		u32 hId = LCG(seed) >> 16;
		u32 lId = LCG(seed) >> 16;
		u32 pid = (hId << 16) | (lId);

		quilava.natureIndex = pid % 25;

		seed = oldSeed;
		LCG(seed);

		// naughty
		if (quilava.natureIndex != 4) {
			continue;
		}
		// 25+ hp, 29+ atk, 24+ def, 30+ sp atk, 24+ speed
		if (quilava.hpIV < 25 || quilava.atkIV < 29 || quilava.defIV < 24 || quilava.spAtkIV < 30 || quilava.speedIV < 24) {
			continue;
		}

		printf("found good espeon/quilava pair at initial seed %08X and quilava frame %d\n", originalSeed, i);
		fflush(stdout);
	}
}

static void doThread(u64 start, u64 end) {
	for (u64 i = start; i < end; i++) {
		doSearch(i);
	}
}

#define NUM_THREADS 4

int main(void) {
#ifdef DEBUG
	doSearch(0x82DD8E91);
#else
	std::thread* threads[NUM_THREADS];
	u64 const n = 0x100000000 / NUM_THREADS;
	for (u32 i = 0; i < NUM_THREADS; i++) {
		threads[i] = new std::thread(doThread, n * i, n * (i + 1));
	}

	for (u32 i = 0; i < NUM_THREADS; i++) {
		threads[i]->join();
		delete threads[i];
		threads[i] = nullptr;
	}
#endif

	return 0;
}
