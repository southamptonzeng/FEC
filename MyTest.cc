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
int main()
{



	if (1) {
		BoseChaudhuriHocquenghem<2, 1, 11, GF::Types<4, 0b10011, uint8_t>> bch({0b10011});
		uint8_t code[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t target[15] = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1};
		std::cout << "NASA INTRO BCH(15, 11) T=1" << std::endl;
        bch.encode(code);
        //bch.decode(code); //把编好的码重新再解码，默认是没有擦除的
        code[0] = 1;
        bch.decode(code);
        /*
        uint8_t erasures[2] = {0, 0};
        int erasures_count = 1;
        bch.decode(code, erasures, erasures_count);
        */
       std::cout << "纠正错误之后=";
       for (int i = 0; i < bch.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
	}
    if (1) {
		ReedSolomon<4, 0, GF::Types<4, 0b10011, uint8_t>> rs;
		uint8_t code[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		uint8_t target[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 3, 3, 12, 12 };
		std::cout << "BBC WHP031 RS(15, 11) T=2" << std::endl;
        rs.encode(code);
        //rs.decode(code); //把编好的码重新再解码，默认是没有擦除的
        code[0] = 9;
        code[1] = 10;
        std::cout << "纠正错误之前=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
        rs.decode(code);
        std::cout << "纠正错误之后=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
    }
    return 0;
}