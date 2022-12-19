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

int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	std::string seedStr;
	std::vector<u32> seeds;
	std::ifstream file("seeds_quil.txt");

	while (std::getline(file, seedStr)) {
		seedStr = std::string("0x") + seedStr;
		u32 seed = std::stoul(seedStr, nullptr, 16);
		seeds.push_back(seed);
	}

	std::sort(seeds.begin(), seeds.end());

	// 0xD169975D
	u32 targetSeed = std::stoul(std::string(argv[1]), nullptr, 0);
	u32 const advanceOffset = 484 + 64; // rng freeze at pokeball animation start
	for (u32 i = 0; i < 200000; i++) {
		if (std::binary_search(seeds.begin(), seeds.end(), targetSeed)) {
			printf("Found good seed in %d advances (seed = %08X)\n", i - advanceOffset, targetSeed);
		}

		targetSeed = targetSeed * 214013 + 2531011;
	}

	return 0;
}
