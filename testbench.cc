/*
FEC - Forward error correction
Written in 2017 by <Ahmet Inan> <xdsopl@gmail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>
#include "galois_field.hh"
#include "reed_solomon.hh"
#include "bose_chaudhuri_hocquenghem.hh"

template <typename TYPE>
void print_table(TYPE *table, const char *name, int N)
{
	std::cout << name << "[" << N << "] = {";
	for (int i = 0; i < N; ++i) {
		std::cout << " " << (int)table[i];
		if (i < N - 1)
			std::cout << ",";
	}
	std::cout << " };" << std::endl;
}

template <int NR, int FCR, int M, int P, typename TYPE>
void test_rs(std::string name, ReedSolomon<NR, FCR, GF::Types<M, P, TYPE>> &rs, TYPE *code, TYPE *target, std::vector<uint8_t> &data)
{
	std::cout << "testing: " << name << std::endl;

	{
		rs.encode(code);
		bool error = false;
		for (int i = 0; i < rs.N; ++i)
			error |= code[i] != target[i];
		if (error)
			std::cout << "encoder error!" << std::endl;
		assert(!error);
		//print_table(code + rs.K, "parity", NR);
		error = rs.decode(code);
		if (error)
			std::cout << "decoder error!" << std::endl;
		assert(!error);
		// first symbol is without corruption
		int pos = 0, par = 0, corrupt = -1, erasures_count = 0;
		TYPE erasures[NR];
		// need one parity symbol per erasure
		for (int i = 0; pos < rs.N && par+1 <= NR && i < NR/2; ++i, ++corrupt, ++pos, ++par)
			code[erasures[erasures_count++] = pos] ^= pos;
		// need two parity symbols per error
		for (int i = 0; pos < rs.N && par+2 <= NR && i < NR/2; ++i, ++corrupt, ++pos, par+=2)
			code[pos] ^= pos;
		int corrected = rs.decode(code, erasures, erasures_count);
		if (corrupt != corrected)
			std::cout << "decoder error: expected " << corrupt << " but got " << corrected << std::endl;
		assert(corrupt == corrected);
		TYPE syndromes[NR];
		if (corrected >= 0 && rs.compute_syndromes(code, syndromes)) {
			std::cout << "decoder error: result of correction is not a codeword!" << std::endl;
			assert(false);
		}
		error = false;
		for (int i = 0; i < rs.N; ++i)
			error |= code[i] != target[i];
		if (error)
			std::cout << "decoder error: code doesnt match target" << std::endl;
		assert(!error);
	}

	int blocks = (8 * data.size() + M * rs.K - 1) / (M * rs.K);
	TYPE *coded = new TYPE[rs.N * blocks];
	{
		unsigned acc = 0, bit = 0, pos = 0;
		for (unsigned byte : data) {
			acc |= byte << bit;
			bit += 8;
			while (bit >= M) {
				bit -= M;
				coded[pos++] = rs.N & acc;
				acc >>= M;
				if (pos % rs.N >= rs.K)
					pos += NR;
			}
		}
	}
	{
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < blocks; ++i)
			rs.encode(coded + i * rs.N);
		auto end = std::chrono::system_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		int mbs = (data.size() + msec.count() / 2) / msec.count();
		int bytes = (rs.N * blocks * M) / 8;
		float redundancy = (100.0f*(bytes-data.size())) / data.size();
		std::cout << "encoding of " << data.size() << " random bytes into " << bytes << " codeword bytes (" << std::setprecision(1) << std::fixed << redundancy << "% redundancy) in " << blocks << " blocks took " << msec.count() << " milliseconds (" << mbs << "KB/s)." << std::endl;
	}
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<int> bit_dist(0, M-1), pos_dist(0, rs.N-1);
	auto rnd_bit = std::bind(bit_dist, generator);
	auto rnd_pos = std::bind(pos_dist, generator);
	std::vector<uint8_t> recovered(data.size());
	TYPE *tmp = new TYPE[rs.N * blocks];
	TYPE *erasures = new TYPE[NR * blocks];
	for (int places = 0; places <= NR; ++places) {
		for (int erasures_count = 0; erasures_count <= places; ++erasures_count) {
			for (int i = 0; i < rs.N * blocks; ++i)
				tmp[i] = coded[i];
			int corrupt = 0;
			for (int i = 0; i < blocks; ++i) {
				for (int j = 0; j < places; ++j) {
					erasures[i*NR + j] = rnd_pos();
					for (int k = 0; k < j;) {
						if (erasures[i*NR + k++] == erasures[i*NR + j]) {
							erasures[i*NR + j] = rnd_pos();
							k = 0;
						}
					}
					tmp[i * rs.N + erasures[i*NR + j]] ^= 1 << rnd_bit();
					++corrupt;
				}
			}
			int corrected = 0, wrong = 0;
			auto start = std::chrono::system_clock::now();
			for (int i = 0; i < blocks; ++i) {
				int result = rs.decode(tmp + i * rs.N, erasures + i * NR, erasures_count);
				if (places > NR/2 && places > erasures_count && result >= 0)
					for (int j = i * rs.N; j < (i + 1) * rs.N; ++j)
						wrong += coded[j] != tmp[j];
				corrected += result;
			}
			auto end = std::chrono::system_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			int bytes = (rs.N * blocks * M) / 8;
			int mbs = (bytes + msec.count() / 2) / msec.count();
			std::cout << "decoding with " << places << " errors and " << erasures_count << " known erasures per block took " << msec.count() << " milliseconds (" << mbs << "KB/s).";
			if (corrupt != corrected || wrong)
				std::cout << " expected " << corrupt << " corrected errors but got " << corrected << " and " << wrong << " wrong corrections.";
			std::cout << std::endl;
			assert((places > NR/2 && places > erasures_count) || (corrupt == corrected && !wrong));
			if (corrupt == corrected && !wrong) {
				unsigned acc = 0, bit = 0, pos = 0;
				for (uint8_t &byte: recovered) {
					while (bit < 8) {
						acc |= (unsigned)tmp[pos++] << bit;
						bit += M;
						if (pos % rs.N >= rs.K)
							pos += NR;
					}
					bit -= 8;
					byte = 255 & acc;
					acc >>= 8;
				}
				for (int i = 0; i < blocks; ++i) {
					TYPE syndromes[NR];
					if (rs.compute_syndromes(tmp + i * rs.N, syndromes)) {
						std::cout << "decoder error: result of correction is not a codeword!" << std::endl;
						assert(false);
					}
				}
				if (data != recovered) {
					std::cout << "decoder error: data could not be recovered from corruption!" << std::endl;
					assert(places > NR/2 && places > erasures_count);
				}
			}
		}
	}
	delete[] erasures;
	delete[] tmp;
	delete[] coded;
}

template <int NR, int FCR, int K, int M, int P, typename TYPE>
void test_bch(std::string name, BoseChaudhuriHocquenghem<NR, FCR, K, GF::Types<M, P, TYPE>> &bch, TYPE *code, TYPE *target, std::vector<uint8_t> &data)
{
	std::cout << "testing: " << name << std::endl;

	{
		bch.encode(code);
		bool error = false;
		for (int i = 0; i < bch.N; ++i)
			error |= code[i] != target[i];
		if (error)
			std::cout << "encoder error!" << std::endl;
		assert(!error);
		//print_table(code + K, "parity", bch.NP);
		error = bch.decode(code);
		if (error)
			std::cout << "decoder error!" << std::endl;
		assert(!error);
		int pos = 0, cap = 0, corrupt = 0, erasures_count = 0;
		TYPE erasures[NR];
		// need one root per erasure
		for (int i = 0; pos < bch.N && cap+1 <= NR && i < NR/2; ++i, ++corrupt, ++pos, ++cap)
			code[erasures[erasures_count++] = pos] ^= 1;
		// need two roots per error
		for (int i = 0; pos < bch.N && cap+2 <= NR && i < NR/2; ++i, ++corrupt, ++pos, cap+=2)
			code[pos] ^= 1;
		int corrected = bch.decode(code, erasures, erasures_count);
		if (corrupt != corrected)
			std::cout << "decoder error: expected " << corrupt << " but got " << corrected << std::endl;
		assert(corrupt == corrected);
		TYPE syndromes[NR];
		if (corrected >= 0 && bch.compute_syndromes(code, syndromes)) {
			std::cout << "decoder error: result of correction is not a codeword!" << std::endl;
			assert(false);
		}
		error = false;
		for (int i = 0; i < bch.N; ++i)
			error |= code[i] != target[i];
		if (error)
			std::cout << "decoder error: code doesnt match target" << std::endl;
		assert(!error);
	}
	int blocks = (8 * data.size() + K - 1) / K;
	TYPE *coded = new TYPE[bch.N * blocks];
	{
		unsigned pos = 0;
		for (unsigned byte : data) {
			for (int bit = 0; bit < 8; ++bit) {
				coded[pos++] = (byte >> bit) & 1;
				if (pos % bch.N >= K)
					pos += bch.NP;
			}
		}
	}
	{
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < blocks; ++i)
			bch.encode(coded + i * bch.N);
		auto end = std::chrono::system_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		int mbs = (data.size() + msec.count() / 2) / msec.count();
		int bytes = (bch.N * blocks) / 8;
		float redundancy = (100.0f*(bytes-data.size())) / data.size();
		std::cout << "encoding of " << data.size() << " random bytes into " << bytes << " codeword bytes (" << std::setprecision(1) << std::fixed << redundancy << "% redundancy) in " << blocks << " blocks took " << msec.count() << " milliseconds (" << mbs << "KB/s)." << std::endl;
	}
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<int> pos_dist(0, bch.N-1);
	auto rnd_pos = std::bind(pos_dist, generator);
	std::vector<uint8_t> recovered(data.size());
	TYPE *tmp = new TYPE[bch.N * blocks];
	TYPE *erasures = new TYPE[NR * blocks];
	for (int places = 0; places <= NR; ++places) {
		for (int erasures_count = 0; erasures_count <= places; ++erasures_count) {
			for (int i = 0; i < bch.N * blocks; ++i)
				tmp[i] = coded[i];
			int corrupt = 0;
			for (int i = 0; i < blocks; ++i) {
				for (int j = 0; j < places; ++j) {
					erasures[i*NR + j] = rnd_pos();
					for (int k = 0; k < j;) {
						if (erasures[i*NR + k++] == erasures[i*NR + j]) {
							erasures[i*NR + j] = rnd_pos();
							k = 0;
						}
					}
					tmp[i * bch.N + erasures[i*NR + j]] ^= 1;
					++corrupt;
				}
			}
			int corrected = 0, wrong = 0;
			auto start = std::chrono::system_clock::now();
			for (int i = 0; i < blocks; ++i) {
				int result = bch.decode(tmp + i * bch.N, erasures + i * NR, erasures_count);
				if (places > NR/2 && places > erasures_count && result >= 0)
					for (int j = i * bch.N; j < (i + 1) * bch.N; ++j)
						wrong += coded[j] != tmp[j];
				corrected += result;
			}
			auto end = std::chrono::system_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			int bytes = (bch.N * blocks) / 8;
			int mbs = (bytes + msec.count() / 2) / msec.count();
			std::cout << "decoding with " << places << " errors and " << erasures_count << " known erasures per block took " << msec.count() << " milliseconds (" << mbs << "KB/s).";
			if (corrupt != corrected || wrong)
				std::cout << " expected " << corrupt << " corrected errors but got " << corrected << " and " << wrong << " wrong corrections.";
			std::cout << std::endl;
			assert((places > NR/2 && places > erasures_count) || (corrupt == corrected && !wrong));
			int left = 0;
			for (int i = 0; i < bch.N * blocks; ++i)
				left += tmp[i] > 1;
			if (left)
				std::cout << "correction left GF(2) " << left << " times!" << std::endl;
			if (corrupt == corrected && !wrong) {
				unsigned acc = 0, bit = 0, pos = 0;
				for (uint8_t &byte: recovered) {
					while (bit < 8) {
						acc |= (unsigned)tmp[pos++] << bit++;
						if (pos % bch.N >= K)
							pos += bch.NP;
					}
					bit -= 8;
					byte = 255 & acc;
					acc >>= 8;
				}
				for (int i = 0; i < blocks; ++i) {
					TYPE syndromes[NR];
					if (bch.compute_syndromes(tmp + i * bch.N, syndromes)) {
						std::cout << "decoder error: result of correction is not a codeword!" << std::endl;
						assert(false);
					}
				}
				if (data != recovered) {
					std::cout << "decoder error: data could not be recovered from corruption!" << std::endl;
					assert(places > NR/2 && places > erasures_count);
				}
			}
		}
	}
	delete[] erasures;
	delete[] tmp;
	delete[] coded;
}

int main()
{
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<uint8_t> distribution(0, 255);
	std::vector<uint8_t> data(65471*16);
	std::generate(data.begin(), data.end(), std::bind(distribution, generator));
	/*
	if (1) {
		BoseChaudhuriHocquenghem<6, 1, 5, GF::Types<4, 0b10011, uint8_t>> bch({0b10011, 0b11111, 0b00111});
		uint8_t code[15] = { 1, 1, 0, 0, 1 };
		uint8_t target[15] = { 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0 };
		test_bch("NASA INTRO BCH(15, 5) T=3", bch, code, target, data);
	}
	*/
	if (1) {
		BoseChaudhuriHocquenghem<2, 1, 11, GF::Types<4, 0b10011, uint8_t>> bch({0b10011});
		uint8_t code[15] = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1};
		uint8_t target[15] = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1};
		test_bch("NASA INTRO BCH(15, 11) T=1", bch, code, target, data);
	}
	if (1) {
		ReedSolomon<4, 0, GF::Types<4, 0b10011, uint8_t>> rs;
		uint8_t code[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		uint8_t target[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 3, 3, 12, 12 };
		test_rs("BBC WHP031 RS(15, 11) T=2", rs, code, target, data);
	}
	if (1) {
		ReedSolomon<16, 0, GF::Types<8, 0b100011101, uint8_t>> rs;
		uint8_t code[255], target[255];
		for (int i = 0; i < 239; ++i)
			target[i] = code[i] = i + 1;
		uint8_t parity[16] = { 1, 126, 147, 48, 155, 224, 3, 157, 29, 226, 40, 114, 61, 30, 244, 75 };
		for (int i = 0; i < 16; ++i)
			target[239+i] = parity[i];
		test_rs("DVB-T RS(255, 239) T=8", rs, code, target, data);
	}
	if (1) {
		BoseChaudhuriHocquenghem<24, 1, 65343, GF::Types<16, 0b10000000000101101, uint16_t>> bch({0b10000000000101101, 0b10000000101110011, 0b10000111110111101, 0b10101101001010101, 0b10001111100101111, 0b11111011110110101, 0b11010111101100101, 0b10111001101100111, 0b10000111010100001, 0b10111010110100111, 0b10011101000101101, 0b10001101011100011});
		uint16_t code[65535], target[65535];
		for (int i = 0, s = 0; i < 65343; ++i, s=(s*(s*s*51767+71287)+35149)&0xffffff)
			target[i] = code[i] = (s^=s>>7)&1;
		uint16_t parity[192] = { 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0 };
		for (int i = 0; i < 192; ++i)
			target[65343+i] = parity[i];
		test_bch("DVB-S2 FULL BCH(65535, 65343) T=12", bch, code, target, data);
	}
	if (1) {
		ReedSolomon<64, 1, GF::Types<16, 0b10001000000001011, uint16_t>> rs;
		uint16_t code[65535], target[65535];
		for (int i = 0; i < 65471; ++i)
			target[i] = code[i] = i + 1;
		uint16_t parity[64] = { 25271, 26303, 22052, 31318, 31233, 6076, 40148, 29468, 47507, 32655, 12404, 13265, 23901, 38403, 50967, 50433, 40818, 226, 62296, 23636, 56393, 12952, 11476, 44416, 518, 50014, 10037, 57582, 33421, 42654, 54025, 7157, 4826, 52148, 17167, 23294, 6427, 40953, 11168, 35305, 18209, 1868, 39971, 54928, 27566, 1424, 4846, 25347, 34710, 42190, 56452, 21859, 49805, 28028, 41657, 25756, 22014, 24479, 28758, 17438, 12976, 61743, 46735, 1557 };
		for (int i = 0; i < 64; ++i)
			target[65471+i] = parity[i];
		test_rs("FUN RS(65535, 65471) T=32", rs, code, target, data);
	}
}

