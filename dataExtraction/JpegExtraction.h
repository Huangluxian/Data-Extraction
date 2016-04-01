//
//  JpegExtraction.h
//  dataExtraction
//
//  Created by 黄路衔 on 16/3/10.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#ifndef JpegExtraction_h
#define JpegExtraction_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <string>
using namespace std;

struct Code_Info {
    string Huff_Bit;
    string Appended_Bit;
    int block_index;
    bool usable;
};

class Jpeg_Extraction {
public:
    Jpeg_Extraction();
    ~Jpeg_Extraction();
    
    void read_Jpeg(const char *jpegName);
    bool alloc_Mem();
    void start_Decode();
    void generate_Reference();
    void reference_Decode();
    void start_Decompress();
    void tran_ColorSpace();
    void extractMess_and_recoverImage();
    double message_Correct_Percent();
    bool write_Bmp(const char *bmpName);
    
private:
    string message;             //所要隐藏的信息
    int capacity;               //信息容量
    
    /*jpeg 文件相关信息*/
    int width;                  //图像的宽
    int height;                 //图像的高
    int biBitCount;             //图像类型，每像素位数
    unsigned char *Ycode, *Cbcode, *Crcode;   //三通道的编码
    unsigned char *Ycode_;                    //做对比的编码
    int YcodeSize, CbcodeSize, CrcodeSize;
    string Y_Code_Str, Cb_Code_Str, Cr_Code_Str;
    string Y_Code_Str_;
    
    /*解码相关信息*/
    char *Z_Y, *Z_Cb, *Z_Cr;    //以 Zigzag 扫描并量化后的 YCbCr 通道一维数组
    char *Z_Y_;
    double **Y, **Cb, **Cr;     //YCbCr 矩阵
    double **Y_;
    unsigned char **R, **G, **B;//RGB 矩阵
    unsigned char **R_;
    
    /*分段的解码字节流，每个段由哈夫曼码和其指定长度的扩展码组成*/
    int Y_Segment_Size, Cb_Segment_Size, Cr_Segment_Size;
    Code_Info *Y_Segment, *Cb_Segment, *Cr_Segment;
    Code_Info *Y_Segment_;
    bool *Usable_Block;    //记录哪个块可嵌入信息
    
    /*亮度和色度的量化表*/
    unsigned char Y_Quan_Table[8][8];
    unsigned char C_Quan_Table[8][8];
    
    /* jpeg 储存标准的哈弗曼编码表*/
    unsigned int YDC_Huff_Table[16];
    unsigned int YDC_Huff_Value[12];
    unsigned int CDC_Huff_Table[16];
    unsigned int CDC_Huff_Value[12];
    unsigned int YAC_Huff_Table[16];
    unsigned int YAC_Huff_Value[130];
    unsigned int CAC_Huff_Table[16];
    unsigned int CAC_Huff_Value[130];
    string *YDC_Huff_Code, *CDC_Huff_Code, *YAC_Huff_Code, *CAC_Huff_Code; //便于查表
    int YDC_table_size, YAC_table_size, CDC_table_size, CAC_table_size;
    
    void huff_Code_2_String();
    void release_Code();
    void release_Huff_Code();
    void release_Temp_Bit();
    void release_YCbCr();
    void release_RGB();
};

#endif /* JpegExtraction_h */
