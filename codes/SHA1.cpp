#include <iostream>
#include"string.h"
#include"stdlib.h"
#define MAXSIZE 1000

int Full(const unsigned char* input, int input_len, unsigned char* output) {
    // (input_len + pad + 1) = 56 mod 64 ===> pad = (56 - (input_len + 1) % 64 + 64) % 64
    int pad = (119 - (input_len % 64)) % 64;

    memcpy(output, input, input_len);
    output[input_len] = 0x80;
    for (int i = 1; i <= pad; i++) {
        output[input_len + i] = 0x00;
    }
    unsigned long long bits = (unsigned long long)input_len * 8;
    int pos = input_len + 1 + pad;

    // 最后追加 8 字节，表示原始数据的比特长度（即 input_len * 8），按大端序存放
    for (int j = 0; j < 8; j++) {
        output[pos + j] = (bits >> (56 - 8 * j)) & 0xFF;
    }
    return input_len + 1 + pad + 8;
}
/*===================================
                加密逻辑
====================================*/
void Every_block(const unsigned char* block, unsigned int H[5]) {
    unsigned int w[80] = { 0 };
    unsigned int A = H[0];
    unsigned int B = H[1];
    unsigned int C = H[2];
    unsigned int D = H[3];
    unsigned int E = H[4];
    unsigned int K[4] = { 0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6 };
    unsigned int temp = 0;
    for (int i = 0; i < 16; i++) {
        //          高   八   位                                                           低   八   位
        w[i] = (block[4 * i] << 24) | (block[4 * i + 1] << 16) | (block[4 * i + 2] << 8) | block[4 * i + 3];
    }

    //  之后按照公式把w[16-79]的值补充完整
    for (int t = 16;t < 80;t++) {
        w[t] = ((w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]) << 1)
            | ((w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]) >> 31);
    }

    for (int j = 0;j < 80;j++) {
        switch (j / 20) {
        case 0:
            temp = ((A << 5) | (A >> 27)) + ((B & C) | (~B & D)) + E + w[j] + K[0];
            E = D; D = C; C = ((B << 30) | (B >> 2)); B = A; A = temp;
            break;
        case 1:
            temp = ((A << 5) | (A >> 27)) + (B ^ C ^ D) + E + w[j] + K[1];
            E = D; D = C; C = ((B << 30) | (B >> 2)); B = A; A = temp;
            break;
        case 2:
            temp = ((A << 5) | (A >> 27)) + ((B & C) | (B & D) | (C & D)) + E + w[j] + K[2];
            E = D; D = C; C = ((B << 30) | (B >> 2)); B = A; A = temp;
            break;
        case 3: 
            temp = ((A << 5) | (A >> 27)) + (B ^ C ^ D) + E + w[j] + K[3];
            E = D; D = C; C = ((B << 30) | (B >> 2)); B = A; A = temp;
            break;
        }
    }
    H[0] += A;
    H[1] += B;
    H[2] += C;
    H[3] += D;
    H[4] += E;
}

void Encrypt_SHA1(const unsigned char* plaintext, int len, unsigned char chipertext[20]) {
    unsigned int H[5] = { 0x67452301, 0xEFCDAB89,  0x98BADCFE,  0x10325476, 0xC3D2E1F0 };
    unsigned char filled_plaintext[MAXSIZE + 64];

    int filled_len = Full(plaintext, len, filled_plaintext);

    for (int i = 0; i < filled_len; i += 64) {
        Every_block(filled_plaintext + i, H);
    }


    // 大端序存放
    for (int i = 0; i < 5; i++) {
        chipertext[4 * i + 0] = (H[i] >> 24) & 0xFF;
        chipertext[4 * i + 1] = (H[i] >> 16) & 0xFF;
        chipertext[4 * i + 2] = (H[i] >> 8) & 0xFF;
        chipertext[4 * i + 3] = H[i] & 0xFF;
    }
}


/*===================================
                主函数
====================================*/
int main() {
    unsigned char plaintext[] = "I_LOVE_FISCHL";
    int len = sizeof(plaintext) - 1;  // 减去结尾的 '\0'
    unsigned char chipertext[20] = { 0 };

    printf("YOUR PLAINTEXT : %s\n", plaintext);
    Encrypt_SHA1(plaintext, len, chipertext);

    // 打印十六进制结果
    printf("YOUR CIPERTEXT : ");
    for (int i = 0; i < 20; i++) {
        printf("%02x", chipertext[i]);
    }
    printf("\n");
    return 0;
}