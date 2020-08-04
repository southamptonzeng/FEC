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
#include <stdio.h>
#include <string.h>
struct CDataPacket
{
    unsigned int TLPLength1:4;//TLP长度[3:0]，4bit
    unsigned int STP:4;//STP固定字段
    unsigned int FP:1;//帧奇偶校验，可检测一位TLPLength的错误
    unsigned int TLPLength2:7;//TLP长度[10:4]，7bit
    unsigned int FCRC:4;//帧CRC，可检出两位TLPLength的错误
    unsigned int TLPSeqNum:12;//数据包传至数据链路层添加的包序列号,12bit
    //以上为STP Token字段，共32bit

    unsigned int Fmt:3;//TLP包格式,010表示3DW header且携带数据
    unsigned int Type:5;//TLP包类型，00001表示内存读请求，00000表示内存写请求，01010表示携带数据的回复
    unsigned int Attr:2;//TLP包属性，包括排序等性质，00表示强排序
    unsigned int TC:3;//流量类型,000表示最高服务等级
    unsigned int TD:1;//表明此TLP是否含有Digest
    unsigned int Length:10;//表示DataPayload的长度,十位0表示1024DW
    unsigned int Reserved:8;//保留字段，8bit

    unsigned int Addressing1=0;
    unsigned int Addressing2=0;
    unsigned int Addressing3;
    //以上为数据包Header字段，共计32*3 bit

    int Digest;//TLP后缀，用于进行端到端的CRC校验,32bit

	char32_t LCRC;//TLP的链路CRC校验,32bit

    struct timeval Delay;//记录运算延时与硬件实际延时的差值,含tv_sec和tv_usec

    char DataPayload[556-44];//数据负载，大小在0~1024DW，按需求此处只模拟0~512Byte大小
};
int EnCode(CDataPacket* dp) {
    //开辟一个数组buf
    unsigned int size = sizeof(CDataPacket);
    std::cout << "包的大小为：" << size << std::endl; //目前包的大小为556字节，buf长度 > code长度
    uint8_t buf[size];
    memcpy(buf,dp,sizeof(CDataPacket));
    if (1) {//是10bit编码（544，514），一个包两个编码搞定。（1023，993）
        unsigned int times = size / 514; //编码几次
        //unsigned int rest = size % 514;
        unsigned int buf_position = 0;
        ReedSolomon<30, 0, GF::Types<10, 0b10000001001, uint16_t>> rs; //TODO参数有问题
        for(unsigned int temp = 0; temp < times + 1; temp++)
        {
            if (temp < times) { //填充完code
                uint16_t code[1023] = {0};
                for (unsigned int i = 0; i < 514; i++, buf_position++) { //把一个buf的8bit扩展成10bit就行了，直接赋值
                    code[i] = buf[buf_position];               
                }
                std::cout << "信息码=";
                for (unsigned int i = 0; i < rs.N; i++) {
			        printf("%d ", code[i]);
		        }
                rs.encode(code); //编码函数
                std::cout << "编码完成后=";
                for (unsigned int i = 0; i < rs.N; i++) {
			        printf("%d ", code[i]);
		        }
                uint16_t cut[544]; //截取之后的数组
                for (unsigned int i = 0, j = 994; i < 544; i++) {
                    if (i < 514)
                        cut[i] = code[i]; //0-513
                    else {
                        cut[i] = code[j]; //514-543
                        j++;
                    }
                }
                char trans[544 * 2] = {0};
                memcpy(trans, cut, 544 * 2);
                //DPSeg(trans);
            }
            else { //不能填充完code，粗暴补0
                uint16_t code[1023] = {0};
                for(unsigned int i =0; buf_position < size; i++, buf_position++) {
                    code[i] = buf[buf_position];
                }
                std::cout << "信息码=";
                for (unsigned int i = 0; i < rs.N; i++) {
			        printf("%d ", code[i]);
		        }
                rs.encode(code); //编码函数
                std::cout << "编码完成后=";
                for (unsigned int i = 0; i < rs.N; i++) {
			        printf("%d ", code[i]);
		        }
                uint16_t cut[544]; //截取之后的数组
                for (unsigned int i = 0, j = 994; i < 544; i++) {
                    if (i < 514)
                        cut[i] = code[i]; //0-513
                    else {
                        cut[i] = code[j]; //514-543
                        j++;
                    }
                }
                char trans[544 * 2] = {0};
                memcpy(trans, cut, 544 * 2);
                //DPSeg(trans);
            }    

        }
        
    }
}
int main()
{
    struct CDataPacket packet;
    //packet.DataPayload = {1, 2, 3};
   
    return 0;
}