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
    */
   /*测试RS短截码（204，188），由RS（255，223）截取*/
   if (1) {
		ReedSolomon<32, 1, GF::Types<8, 0b100011101, uint8_t>> rs;
        uint8_t code[255];
        std::cout << "DVD RS(204, 188) T=16 由(255,223)截取而来" << std::endl;
        for (int i=0; i < 255; i++) //信息比特只有188位长，189到223用0填充，224到255只是保留长度
        {
            if (i < 188)
                code[i] = i;
            else
                code[i] = 0;
        }
        std::cout << "信息码=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
        rs.encode(code);
        std::cout << "编码完成后=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
        for (int i = 188; i < 223; i++) //编码完成后，抹除前189到223的码字
        {
            code[i] = 0;
        }
        rs.decode(code); //把编好的码重新再解码，默认是没有擦除的
        std::cout << "译码完成后=";
        for (int i = 0; i < rs.N; i++) {
			printf("%d ", code[i]);
		}
        std::cout << std::endl;
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