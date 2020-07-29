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
#include <bitset>
#include <string.h>

#define M 50
#define IPSIZE 15
#define HALF_WORD   uint16_t
#define WORD        uint32_t

using namespace std;

struct ClientInfo{
    char headName[M];       
    char name[M];            
    int num;                
    char ip[IPSIZE];
    char data[M];
    char time[M];
};
int main()
{
    ClientInfo* aaa = (ClientInfo*)malloc(sizeof(ClientInfo));
    strcpy(aaa->headName,"aaaaaaaaaaaaa");
    strcpy(aaa->name,"bbbbbbbbb");
    aaa->num = 10;
    strcpy(aaa->ip,"ccccccc");
    strcpy(aaa->data,"ddddddddd");
    strcpy(aaa->time,"eeeeeeeee");

    int size = sizeof(ClientInfo);
    char buf[size];
    cout << "ClientInfo结构体大小：" << size * 8 << " bits" << endl; //理论上来说二期的CDatapackage结构体大小也固定，
                                                                     //一期包的大小为4448 bits，加上扰码有多大？最好大于5140 bits
                                                                     //这样我只需要切分了

    memcpy(buf,aaa,sizeof(*aaa)); //把结构体aaa的内存拷贝给buf

    //TODO
    //对buf进行编码解码，用RS短截码(204,188)，信息为长度188 * 8 = 1504 bits，结构体大了要切，结构体小了要添
        ReedSolomon<32, 1, GF::Types<8, 0b100011101, uint8_t>> rs;
        uint8_t code[255];
        std::cout << "DVD RS(204, 188) T=16 由(255,223)截取而来" << std::endl;

        for (int i=0; i < 255; i++) //信息比特只有188位长，189到223用0填充，224到255只是保留长度
        {
            if (i < 188)
                code[i] = buf[i];
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

    
    struct ClientInfo bbb;
    memcpy(&bbb,buf,sizeof(bbb)); //把buf的内存拷贝给bbb

    cout << bbb.headName << endl;
    cout << bbb.name << endl;
    cout << bbb.num << endl;
    cout << bbb.ip << endl;
    cout << bbb.data << endl;
    cout << bbb.time << endl;
    return 0;
}