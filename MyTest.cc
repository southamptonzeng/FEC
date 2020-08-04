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
    /*
	if (1) {
		BoseChaudhuriHocquenghem<2, 1, 11, GF::Types<4, 0b10011, uint8_t>> bch({0b10011});
		uint8_t code[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		std::cout << "NASA INTRO BCH(15, 11) T=1" << std::endl;
        bch.encode(code);
        code[0] = 1;
        bch.decode(code);
       std::cout << "纠正错误之后=";
       for (int i = 0; i < bch.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
	}
    if (1) {
		ReedSolomon<4, 0, GF::Types<4, 0b10011, uint8_t>> rs;
		uint8_t code[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		std::cout << "BBC WHP031 RS(14, 10) T=2" << std::endl;
        rs.encode(code);
        rs.decode(code); //把编好的码重新再解码，默认是没有擦除的
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
   //测试RS短截码（544，514），由RS（1023，993）截取
   if (1) {
		ReedSolomon<30, 1, GF::Types<10, 0b10000001001, uint16_t>> rs;
        uint16_t code[1024] = {0};
        for (int i = 0; i < 993; i++) {
            code[i] = i;
        }
        rs.encode(code);
        std::cout << "纠正错误之前=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        code[0] = 1023;
        rs.decode(code);
        std::cout << "纠正错误之后=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
   }*/
   //测试RS短截码（96，86），由RS（127，117）截取
   if (1) {
        ReedSolomon<10, 1, GF::Types<7, 0b10001001, uint8_t>> rs;
        uint8_t code[128] = {0};
        for (int i = 0; i < 117; i++) {
            code[i] = i;
        }
        rs.encode(code);
        std::cout << "纠正错误之前=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        code[0] = 97;
        rs.decode(code);
        std::cout << "纠正错误之后=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
   }
    return 0;
}