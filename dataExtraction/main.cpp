//
//  本科毕业设计
//  main.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/3.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include <iostream>
#include "bmpStruct.h"
#include "commonFun.h"
#include "JpegExtraction.h"

int main(int argc, const char * argv[]) {
    string image_decrypt;
    string image_recover;
    
    /*extraction jpeg*/
    image_decrypt = "/Users/huangluxian/Desktop/lena_decrypted.jpeg";
    image_recover = "/Users/huangluxian/Desktop/lena_recover.bmp";
    cout << "Begin to extraction image..." << endl;
    Jpeg_Extraction exjpeg;
    exjpeg.read_Jpeg(image_decrypt.c_str());
    exjpeg.alloc_Mem();
    exjpeg.start_Decode();
    exjpeg.generate_Reference();
    exjpeg.reference_Decode();
    exjpeg.start_Decompress();
    exjpeg.tran_ColorSpace();
    exjpeg.extractMess_and_recoverImage();
    cout << "The correct percent is: " << exjpeg.message_Correct_Percent() << '%' << endl;
    exjpeg.write_Bmp(image_recover.c_str());
    cout << "done!" << endl << endl;
    
    return 0;
}
