//
//  JpegExtraction.cpp
//  dataExtraction
//
//  Created by 黄路衔 on 16/3/10.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include "JpegExtraction.h"
#include "commonFun.h"
#include "bmpStruct.h"

Jpeg_Extraction::Jpeg_Extraction()
{
}

Jpeg_Extraction::~Jpeg_Extraction()
{
}

void Jpeg_Extraction::read_Jpeg(const char *jpegName)
{
    ifstream inFile;
    inFile.open(jpegName, ios::binary);
    char ch;
    
    /*读取图像的基本信息*/
    inFile >> width >> height >> biBitCount;
    
    /*读取量化表*/
    ch = inFile.get();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Y_Quan_Table[i][j] = inFile.get();
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            C_Quan_Table[i][j] = inFile.get();
        }
    }
    
    /*读取哈夫曼表*/
    for (int i = 0; i < 16; i++) {
        inFile >> YDC_Huff_Table[i];
        YDC_table_size = YDC_table_size + YDC_Huff_Table[i];
    }
    for (int i = 0; i < YDC_table_size; i++) {
        inFile >> YDC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> YAC_Huff_Table[i];
        YAC_table_size = YAC_table_size + YAC_Huff_Table[i];
    }
    for (int i = 0; i < YAC_table_size; i++) {
        inFile >> YAC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> CDC_Huff_Table[i];
        CDC_table_size = CDC_table_size + CDC_Huff_Table[i];
    }
    for (int i = 0; i < CDC_table_size; i++) {
        inFile >> CDC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> CAC_Huff_Table[i];
        CAC_table_size = CAC_table_size + CAC_Huff_Table[i];
        
    }
    for (int i = 0; i < CAC_table_size; i++) {
        inFile >> CAC_Huff_Value[i];
    }
    
    /*
     读取图像数据
     遇到连续两个 0xff 即表示该通道数据终止
     */
    ch = inFile.get();
    int i = 0, count = 0;
    unsigned char *Ych;
    Ych = new unsigned char[10000000];
    Ych[i] = inFile.get();
    while (count < 2) {
        i++;
        Ych[i] = inFile.get();
        if (0xff == Ych[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    YcodeSize = i - 1;
    Ycode = new unsigned char [YcodeSize];
    for (int z = 0; z < YcodeSize; z++) {
        Ycode[z] = Ych[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Cbch;
    Cbch = new unsigned char[10000000];
    Cbch[i] = inFile.get();
    while (count < 2) {
        i++;
        Cbch[i] = inFile.get();
        if (0xff == Cbch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CbcodeSize = i - 1;
    Cbcode = new unsigned char [CbcodeSize];
    for (int z = 0; z < CbcodeSize; z++) {
        Cbcode[z] = Cbch[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Crch;
    Crch = new unsigned char[10000000];
    Crch[i] = inFile.get();
    while (count < 2) {
        i++;
        Crch[i] = inFile.get();
        if (0xff == Crch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CrcodeSize = i - 1;
    Crcode = new unsigned char [CrcodeSize];
    for (int z = 0; z < CrcodeSize; z++) {
        Crcode[z] = Crch[z];
    }
    
    delete [] Ych;
    delete [] Cbch;
    delete [] Crch;
    inFile.clear();
    inFile.close();
}

void Jpeg_Extraction::start_Decode()
{
    huff_Code_2_String();
    int i, j, index, num, block_index;
    int zeronum;
    int DClength, AClength;
    string allcode, subcode;
    unsigned char temp;
    bool ac_all_zero = false;
    
    /**********处理 Y 通道**********/
    Y_Segment = new Code_Info[height * width];
    Y_Segment_Size = 0;
    for (i = 0; i < YcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Ycode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < YDC_table_size; i++) {
                if (subcode == YDC_Huff_Code[i]) {
                    Y_Segment[Y_Segment_Size].Huff_Bit = subcode;
                    DClength = YDC_Huff_Value[i];
                    if (0 == DClength) {
                        if(0 == num) {
                            Z_Y[num] = 0;
                        } else {
                            Z_Y[num] = Z_Y[num - 64];
                        }
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    Y_Segment[Y_Segment_Size].Appended_Bit = DCcode;
                    if(0 == num) {
                        Z_Y[num] = Decode_VLC(DCcode);
                    } else {
                        Z_Y[num] = Decode_VLC(DCcode) + Z_Y[num - 64];
                    }
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == YDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Y_Segment[Y_Segment_Size].block_index = block_index;
        /*DC 码不进行数据隐藏*/
        Y_Segment[Y_Segment_Size].usable = false;
        Y_Segment_Size++;
        
        /*读取 AC 值*/
        i = 0;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < YAC_table_size; j++) {
                if (subcode == YAC_Huff_Code[j]) {
                    Y_Segment[Y_Segment_Size].Huff_Bit = subcode;
                    if (0 == YAC_Huff_Value[j]) {
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        for (int k = i; k < 63; k++) {
                            Z_Y[num++] = 0;
                        }
                        i = 63;
                        index++;
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        break;
                    } else if(0xf0 == YAC_Huff_Value[j]) {
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        for (int k = 0; k < 16; k++) {
                            Z_Y[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = YAC_Huff_Value[j] / 16;
                        AClength = YAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Y[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Y[num++] = Decode_VLC(ACcode);
                        Y_Segment[Y_Segment_Size].Appended_Bit = ACcode;
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == YAC_table_size) {
                index++;
            } else {
                Y_Segment[Y_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    /*若(i + j)为偶数，且 AC 不全为0，则定义为可用块*/
                    Y_Segment[Y_Segment_Size].usable = true;
                    Usable_Block[block_index] = true;
                } else {
                    Y_Segment[Y_Segment_Size].usable = false;
                    Usable_Block[block_index] = false;
                }
                Y_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        ac_all_zero = false;
        block_index++;
    }
    
    /**********处理 Cb 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    Cb_Segment = new Code_Info [height * width];
    Cb_Segment_Size = 0;
    for (i = 0; i < CbcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Cbcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    Cb_Segment[Cb_Segment_Size].Huff_Bit = subcode;
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        if(0 == num) {
                            Z_Cb[num] = 0;
                        } else {
                            Z_Cb[num] = Z_Cb[num - 64];
                        }
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    Cb_Segment[Cb_Segment_Size].Appended_Bit = DCcode;
                    if(0 == num) {
                        Z_Cb[num] = Decode_VLC(DCcode);
                    } else {
                        Z_Cb[num] = Decode_VLC(DCcode) + Z_Cb[num - 64];
                    }
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Cb_Segment[Cb_Segment_Size].block_index = block_index;
        Cb_Segment[Cb_Segment_Size].usable = false;
        Cb_Segment_Size++;
        
        /*读取 AC 值*/
        i = 0;
        ac_all_zero = false;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    Cb_Segment[Cb_Segment_Size].Huff_Bit = subcode;
                    if (0 == CAC_Huff_Value[j]) {
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        for (int k = i; k < 63; k++) {
                            Z_Cb[num++] = 0;
                        }
                        i = 63;
                        index++;
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        break;
                    } else if(0xf0 == CAC_Huff_Value[j]) {
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        for (int k = 0; k < 16; k++) {
                            Z_Cb[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength = CAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Cb[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Cb[num++] = Decode_VLC(ACcode);
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = ACcode;
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else {
                Cb_Segment[Cb_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    Cb_Segment[Cb_Segment_Size].usable = true;
                } else {
                    Cb_Segment[Cb_Segment_Size].usable = false;
                }
                Cb_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        ac_all_zero = false;
        block_index++;
    }
    
    /**********处理 Cr 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    Cr_Segment = new Code_Info [height * width];
    Cr_Segment_Size = 0;
    for (i = 0; i < CrcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Crcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    Cr_Segment[Cr_Segment_Size].Huff_Bit = subcode;
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        if(0 == num) {
                            Z_Cr[num] = 0;
                        } else {
                            Z_Cr[num] = Z_Cr[num - 64];
                        }
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    Cr_Segment[Cr_Segment_Size].Appended_Bit = DCcode;
                    if(0 == num) {
                        Z_Cr[num] = Decode_VLC(DCcode);
                    } else {
                        Z_Cr[num] = Decode_VLC(DCcode) + Z_Cr[num - 64];
                    }
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Cr_Segment[Cr_Segment_Size].block_index = block_index;
        Cr_Segment[Cr_Segment_Size].usable = false;
        Cr_Segment_Size++;
        
        /*读取 AC 值*/
        i = 0;
        ac_all_zero = false;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    Cr_Segment[Cr_Segment_Size].Huff_Bit = subcode;
                    if (0 == CAC_Huff_Value[j]) {
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        for (int k = i; k < 63; k++) {
                            Z_Cr[num++] = 0;
                        }
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == CAC_Huff_Value[j]) {
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        for (int k = 0; k < 16; k++) {
                            Z_Cr[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength = CAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Cr[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Cr[num++] = Decode_VLC(ACcode);
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = ACcode;
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else {
                Cr_Segment[Cr_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    Cr_Segment[Cb_Segment_Size].usable = true;
                } else {
                    Cr_Segment[Cb_Segment_Size].usable = false;
                }
                Cr_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        block_index++;
        ac_all_zero = false;
    }
    release_Code();
}

void Jpeg_Extraction::generate_Reference()
{
    /*
     将所有可嵌入信息的块都与1异或，作为对比以取出嵌入的数据
     本程序只处理灰度图，故只需生成 Y 通道的对比
     */
    Y_Segment_ = new Code_Info[height * width];
    for (int i = 0; i < Y_Segment_Size; i++) {
        Y_Segment_[i].Huff_Bit = Y_Segment[i].Huff_Bit;
        Y_Segment_[i].Appended_Bit = Y_Segment[i].Appended_Bit;
        
        if (true == Y_Segment[i].usable) {
            string temp = get_String_One((int)Y_Segment[i].Appended_Bit.length());
            Y_Segment_[i].Appended_Bit = String_Xor(Y_Segment[i].Appended_Bit, temp);
        }
    }
}

bool Jpeg_Extraction::alloc_Mem()
{
    Z_Y = new char[width * height];
    Z_Y_ = new char[width * height];
    Z_Cb = new char[width * height];
    Z_Cr = new char[width * height];
    
    Y = new double *[height];
    Y_ = new double *[height];
    Cb = new double *[height];
    Cr = new double *[height];
    R = new unsigned char *[height];
    G = new unsigned char *[height];
    B = new unsigned char *[height];
    R_ = new unsigned char *[height];
    for (int i = 0; i < height; i++) {
        Y[i] = new double[width];
        Y_[i] = new double[width];
        Cb[i] = new double[width];
        Cr[i] = new double[width];
        R[i] = new unsigned char[width];
        G[i] = new unsigned char[width];
        B[i] = new unsigned char[width];
        R_[i] = new unsigned char[width];
    }
    
    Usable_Block = new bool[width * height / 64];
    
    if (NULL == Z_Y || NULL == Z_Cb || NULL == Z_Cr || NULL == Y || NULL == Cb || NULL == Cr || NULL == R || NULL == G || NULL == B) {
        delete []Ycode;
        delete []Ycode_;
        delete []Cbcode;
        delete []Crcode;
        return false;
    }
    return true;
}

void Jpeg_Extraction::reference_Decode()
{
    int i, j, index, num;
    int zeronum;
    string allcode, subcode;
    
    index = 0;
    num = 0;
    while (index < Y_Segment_Size) {
        /*读取 DC 值*/
        if (0 == Y_Segment_[index].Appended_Bit.length()) {
            if(0 == num) {
                Z_Y_[num] = 0;
            } else {
                Z_Y_[num] = Z_Y_[num - 64];
            }
        } else {
            if(0 == num) {
                Z_Y_[num] = Decode_VLC(Y_Segment_[index].Appended_Bit);
            } else {
                Z_Y_[num] = Decode_VLC(Y_Segment_[index].Appended_Bit) + Z_Y_[num - 64];
            }
        }
        num++;
        index++;
        /*读取 AC 值*/
        i = 0;
        while (i < 63 && index < Y_Segment_Size) {
            for (j = 0; j < YAC_table_size; j++) {
                if (Y_Segment_[index].Huff_Bit == YAC_Huff_Code[j]) {
                    if (0 == YAC_Huff_Value[j]) {
                        for (int k = i; k < 63; k++) {
                            Z_Y_[num++] = 0;
                        }
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == YAC_Huff_Value[j]) {
                        for (int k = 0; k < 16; k++) {
                            Z_Y_[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = YAC_Huff_Value[j] / 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Y_[num++] = 0;
                        }
                        Z_Y_[num++] = Decode_VLC(Y_Segment_[index].Appended_Bit);
                        i = i + 1 + zeronum;
                        index++;
                        break;
                    }
                }
            }
            if(num == width * height) {
            break;
            }
        }
    }
    release_Huff_Code();
    release_Temp_Bit();
}

void Jpeg_Extraction::start_Decompress()
{
    int i, j, k = 0, l = 0;
    
    /*分块*/
    double **block_Y, **block_Y_, **block_Cb, **block_Cr;
    block_Y = new double *[8];
    block_Y_ = new double *[8];
    block_Cb = new double *[8];
    block_Cr = new double *[8];
    for (i = 0; i < 8; i++) {
        block_Y[i] = new double[8];
        block_Y_[i] = new double[8];
        block_Cb[i] = new double[8];
        block_Cr[i] = new double[8];
    }
    int *block_Z_Y, *block_Z_Y_, *block_Z_Cb, *block_Z_Cr;
    block_Z_Y = new int [64];
    block_Z_Y_ = new int [64];
    block_Z_Cb = new int [64];
    block_Z_Cr = new int [64];
    
    /*依次取出64个数，转化为8*8矩阵进行逆变换*/
    for (i = 0; i < width * height / 64; i++) {
        for (j = 0; j < 64; j++) {
            block_Z_Y[j] = Z_Y[i * 64 + j];
            block_Z_Y_[j] = Z_Y_[i * 64 + j];
            block_Z_Cb[j] = Z_Cb[i * 64 + j];
            block_Z_Cr[j] = Z_Cr[i * 64 + j];
        }
        
        IZigzag(block_Z_Y, block_Y);
        IZigzag(block_Z_Y_, block_Y_);
        IZigzag(block_Z_Cb, block_Cb);
        IZigzag(block_Z_Cr, block_Cr);
        
        IQuan(block_Y, Y_Quan_Table);
        IQuan(block_Y_, Y_Quan_Table);
        IQuan(block_Cb, C_Quan_Table);
        IQuan(block_Cr, C_Quan_Table);
        
        IDCT(block_Y);
        IDCT(block_Y_);
        IDCT(block_Cb);
        IDCT(block_Cr);
        
        /*将数据写进图像矩阵*/
        for (int m = 0; m < 8; m++) {
            for (int n = 0; n < 8; n++) {
                Y[k * 8 + m][l * 8 + n] = block_Y[m][n];
                Y_[k * 8 + m][l * 8 + n] = block_Y_[m][n];
                Cb[k * 8 + m][l * 8 + n] = block_Cb[m][n];
                Cr[k * 8 + m][l * 8 + n] = block_Cr[m][n];
            }
        }
        l++;
        if (l * 8 == width) {
            l = 0;
            k++;
        }
    }
    
    for (i = 0; i < 8; i++) {
        delete[] block_Y[i];
        delete[] block_Y_[i];
        delete[] block_Cb[i];
        delete[] block_Cr[i];
    }
    delete []block_Y;
    delete []block_Y_;
    delete []block_Cb;
    delete []block_Cr;
    delete []block_Z_Y;
    delete []block_Z_Y_;
    delete []block_Z_Cb;
    delete []block_Z_Cr;
}

void Jpeg_Extraction::tran_ColorSpace()
{
    double r, g, b, r_;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            r = Y[i][j] + 0 * (Cb[i][j] - 128) + 1.402 * (Cr[i][j] - 128);
            g = Y[i][j] - 0.34414 * (Cb[i][j] - 128) - 0.71414 * (Cr[i][j] - 128);
            b = Y[i][j] + 1.772 * (Cb[i][j] - 128) + 0 * (Cr[i][j] - 128);
            r_ = Y_[i][j] + 0 * (Cb[i][j] - 128) + 1.402 * (Cr[i][j] - 128);
            
            /*将 double 类型的数据转化为 unsigned char 类型的数据，便于写进 bmp 文件*/
            if (r < 0) {
                R[i][j] = 0;
            } else if (r > 255) {
                R[i][j] = 255;
            } else {
                R[i][j] = (unsigned char)r;
            }
            if (g < 0) {
                G[i][j] = 0;
            } else if (g > 255) {
                G[i][j] = 255;
            } else {
                G[i][j] = (unsigned char)g;
            }
            if (b < 0) {
                B[i][j] = 0;
            } else if (b > 255) {
                B[i][j] = 255;
            } else {
                B[i][j] = (unsigned char)b;
            }
            
            if (r_ < 0) {
                R_[i][j] = 0;
            } else if (r_ > 255) {
                R_[i][j] = 255;
            } else {
                R_[i][j] = (unsigned char)r_;
            }
        }
    }
    release_YCbCr();
}

void Jpeg_Extraction::extractMess_and_recoverImage()
{
    unsigned char CB[8][8], UP[8][8], RT[8][8], DW[8][8], LF[8][8];
    unsigned char CB_[8][8], UP_[8][8], RT_[8][8], DW_[8][8], LF_[8][8];

    for (int i = 0; i < (height / 8); i++) {
        for (int j = 0; j < (width / 8); j++) {
            if (true == Usable_Block[i * (width / 8) + j]) {
                for (int m = 0; m < 8; m++) {
                    for (int n = 0; n < 8; n++) {
                        CB[m][n] = R[i * 8 + m][j * 8 + n];
                        CB_[m][n] = R_[i * 8 + m][j * 8 + n];
                        
                        /*提取上方矩阵，若 CB 在第一行，则将上方矩阵定义为与 CB 第一行一致*/
                        if (0 == i) {
                            UP[m][n] = CB[0][n];
                            UP_[m][n] = CB_[0][n];
                        } else {
                            UP[m][n] = R[(i - 1) * 8 + m][j * 8 + n];
                            UP_[m][n] = R_[(i - 1) * 8 + m][j * 8 + n];
                        }
                        /*提取右方矩阵，若 CB 在最后列，则将右方矩阵定义为与 CB 最后列一致*/
                        if ((width / 8 - 1) == j) {
                            RT[m][n] = CB[m][7];
                            RT_[m][n] = CB_[m][7];
                        } else {
                            RT[m][n] = R[i * 8 + m][(j + 1) * 8 + n];
                            RT_[m][n] = R_[i * 8 + m][(j + 1) * 8 + n];
                        }
                        /*提取下方矩阵，若 CB 在最后行，则将下方矩阵定义为与 CB 最后行一致*/
                        if ((height / 8 - 1) == i) {
                            DW[m][n] = CB[7][n];
                            DW_[m][n] = CB_[7][n];
                        } else {
                            DW[m][n] = R[(i + 1) * 8 + m][j * 8 + n];
                            DW_[m][n] = R_[(i + 1) * 8 + m][j * 8 + n];
                        }
                        /*提取左方矩阵，若 CB 在第一列，则将左方矩阵定义为与 CB 第一列一致*/
                        if (0 == j) {
                            LF[m][n] = CB[m][0];
                            LF_[m][n] = CB_[m][0];
                        } else {
                            LF[m][n] = R[i * 8 + m][(j - 1) * 8 + n];
                            LF_[m][n] = R_[i * 8 + m][(j - 1) * 8 + n];
                        }
                    }
                }
                if (BAF(CB, UP, RT, DW, LF) > BAF(CB_, UP_, RT_, DW_, LF_)) {
                    message += '1';
                    for (int m = 0; m < 8; m++) {
                        for (int n = 0; n < 8; n++) {
                            R[i * 8 + m][j * 8 + n] = R_[i * 8 + m][j * 8 + n];
                        }
                    }
                } else {
                    message += '0';
                }
            }
        }
    }
    delete [] Usable_Block;
}

double Jpeg_Extraction::message_Correct_Percent()
{
    string original = "1111111111111111111111111111111111111111111111111111";
    for (int i = 0; i < message.length() - original.length(); i++) {
        original += '0';
    }
    double same = 0;
    for (int i = 0; i < message.length(); i++) {
        if (message[i] == original[i]) {
            same++;
        }
    }
    return  same / message.length() * 100;
}

bool Jpeg_Extraction::write_Bmp(const char *bmpName)
{
    int i, j, k;
    /*每行字节数*/
    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    /*图像数据*/
    unsigned char* imgBuf;
    imgBuf = new unsigned char[lineByte * height];
    j = 0;
    for (i = (height - 1); i >= 0; i--) {
        for (k = 0; k < lineByte / 3; k++) {
            imgBuf[j++] = B[i][k];
            imgBuf[j++] = G[i][k];
            imgBuf[j++] = R[i][k];
        }
    }
    /*如果位图数据指针为0，则没有数据传入，函数返回 false*/
    if (!imgBuf) {
        cout << "写入数据为空" << endl;
        release_RGB();
        return false;
    }
    /*颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0*/
    int colorTablesize = 0;
    if (8 == biBitCount) {
        colorTablesize = 1024;
    }
    
    /*开始写 bmp 文件*/
    FILE *fp = fopen(bmpName, "wb");
    if (0 == fp) {
        cout << "创建 bmp 文件失败" << endl;
        delete []imgBuf;
        release_RGB();
        return false;
    }
    
    /*申请位图文件头结构变量，填写文件头信息*/
    BITMAPFILEHEADER fileHead;
    fileHead.bfType = 0x4D42; //bmp 类型
    /*bfSize 是图像文件4个组成部分之和*/
    fileHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte * height;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    /*bfOffBits是图像文件前3个部分所需空间之和*/
    fileHead.bfOffBits = 54 + colorTablesize;
    /*写文件头进文件*/
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
    
    /*申请位图信息头结构变量，填写信息头信息*/
    BITMAPINFOHEADER head;
    head.biBitCount = biBitCount;
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0;
    head.biHeight = height;
    head.biPlanes = 1;
    head.biSize = 40;
    head.biSizeImage = lineByte * height;
    head.biWidth = width;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;
    /*写位图信息头进内存*/
    fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);
    
    /*写位图数据进文件*/
    fwrite(imgBuf, height * lineByte, 1, fp);
    
    fclose(fp);
    delete []imgBuf;
    release_RGB();
    return true;
}

void Jpeg_Extraction::release_Code()
{
    delete [] Ycode;
    delete [] Cbcode;
    delete [] Crcode;
}

void Jpeg_Extraction::release_Huff_Code()
{
    delete [] YDC_Huff_Code;
    delete [] YAC_Huff_Code;
    delete [] CDC_Huff_Code;
    delete [] CAC_Huff_Code;
}

void Jpeg_Extraction::release_Temp_Bit()
{
    delete [] Y_Segment;
    delete [] Y_Segment_;
    delete [] Cb_Segment;
    delete [] Cr_Segment;
}

void Jpeg_Extraction::release_YCbCr()
{
    for (int i = 0; i < height; i++) {
        delete[] Y[i];
        delete[] Y_[i];
        delete[] Cb[i];
        delete[] Cr[i];
    }
    delete []Y;
    delete []Y_;
    delete []Cb;
    delete []Cr;
    delete []Z_Y;
    delete []Z_Y_;
    delete []Z_Cb;
    delete []Z_Cr;
}

void Jpeg_Extraction::release_RGB()
{
    for (int i = 0; i < height; i++) {
        delete[] R[i];
        delete[] G[i];
        delete[] B[i];
        delete[] R_[i];
    }
    delete []R;
    delete []G;
    delete []B;
    delete []R_;
}

void Jpeg_Extraction::huff_Code_2_String()
{
    int table_Size, index;
    string code;
    
    /*转换亮度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YDC_Huff_Table[i];
    }
    YDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YDC_Huff_Table[i]) {
            YDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CDC_Huff_Table[i];
    }
    CDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CDC_Huff_Table[i]) {
            CDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换亮度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YAC_Huff_Table[i];
    }
    YAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YAC_Huff_Table[i]) {
            YAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CAC_Huff_Table[i];
    }
    CAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CAC_Huff_Table[i]) {
            CAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
}