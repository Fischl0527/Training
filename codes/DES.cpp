#include"stdio.h"
#include"string.h"
#include"DEStable.h"
#include"stdlib.h"
#define MAXSIZE 100
#define _CRT_SECURE_NO_WARNINGS


/*-------------------------------------------------------------
                            核心位操作函数
-------------------------------------------------------------*/
// 表置换函数
void permute(char* input, char* output, const char* table, int table_len) {
    /*
    input 是输入字节数组(按位存储的比特流)
    output 是输出字节数组
    table 是置换表(1-based bit 索引)
    table_len 是表长度(输出的比特数)
    */

    int out_bytes = (table_len + 7) / 8;
    int src = 0;
    int src_byte = 0;
    int src_off = 0;
    int bit_value = 0;
    int output_byte = 0;
    int output_offset = 0;
    /*
    out_bytes 是输出的字节长度
    src 置换表中的 bit 位置(转成 0-based)
    src_byte 输入目标 bit 所在字节索引(0 开始)
    src_off 目标 bit 在该字节内的偏移(高位为 7，低位为 0)
    bit_value 目标 bit 值(0/1)
    output_byte 输出目标 bit 所在字节索引
    output_off 输出目标 bit 在字节内的偏移(高位为 7，低位为 0)
    */

    memset(output, 0, out_bytes);

    for (int i = 0; i < table_len; i++) {
        src = table[i] - 1;        // 表是 1-based，转成 0-based
        src_byte = src / 8;        // 输入字节索引
        src_off = 7 - (src % 8);   // 输入字节内 bit 偏移(高位在前)

        bit_value = (input[src_byte] >> src_off) & 0x01;    // 防止原来的其它位产生影响

        output_byte = i / 8;       // 输出字节索引
        output_offset = 7 - (i % 8);  // 输出字节内 bit 偏移(高位在前)

        // 用 |= 防止覆盖已写入的 bit
        output[output_byte] |= (bit_value << output_offset);
    }
}


// S盒替换：48位输入 -> 32位输出
void s_box_substitution(char* input, char* output) {
    /*
    input  是 48 位输入（6 字节），来自 E 扩展后再与子密钥异或
    output 是 32 位输出（4 字节），每个 S 盒输出 4 位
    */

    int i = 0, bit_index = 0, byte_index = 0, bit_offset = 0, six_bits = 0, row = 0, column = 0, value = 0, out_byte = 0;
    /*
    bit_index 是当前处理的 6-bit 在 48 位中的起始 bit 索引
    byte_index 是该 6-bit 所在的起始字节索引
    bit_offset 是在该字节内的 bit 偏移
    six_bits 是取出的 6 位值（0~63）
    row 是S盒行号（由第1位和第6位组成，0~3）
    column 是S盒列号（由中间4位组成，0~15）
    value 是S盒输出的 4 位值（0~15）
    out_byte 是输出 4 位写入的目标字节索引
    */

    memset(output, 0, 4);   // 清空输出

    for (int i = 0; i < 8; i++) {
        bit_index = i * 6;  // 记录每6位的开头的索引位置
        byte_index = bit_index / 8; // 记录输入的字节位置
        bit_offset = bit_index % 8; // 记录偏移量

        // 合并input[i]和inpiut[i+1]，防止之后的six_bits移位操作越界
        six_bits = (input[byte_index] << 8) & 0xFF00;
        if (byte_index + 1 < 6)
            six_bits |= input[byte_index + 1] & 0xFF;

        // 15 - bit_offset - 6 + 1 =  (10 - bit_offset)移位操作，提取第i个6字节块
        six_bits = (six_bits >> (10 - bit_offset)) & 0x3F;

        row = ((six_bits & 0x20) >> 4) | (six_bits & 0x01); // 找行号，即6字节块的首尾部相连组成的二进制数
        column = (six_bits >> 1) & 0x0F;    // 找列号，即6字节块去除首尾组成的二进制数

        value = S_Box[i][row][column] & 0x0F;   // 从S盒找出对应的4位二进制的值


        // 2个4位二进制的值组成一个字节存储在output数组里
        out_byte = i / 2;
        if (i % 2 == 0) {
            output[out_byte] = (value << 4);
        }
        else {
            output[out_byte] |= value;
        }

    }
}
// F函数：32位R + 48位子密钥 -> 32位输出
void f_function(char* right, char* subkeys, char* output) {
    /*
r 是右32位的数据
subkeys 是当前f函数密钥
output 是经过本轮f函数输出的结果
*/
    char right_expanded[6] = { 0 };
    char xored[6] = { 0 };
    char sbox_out[4] = { 0 };
    /*
    r_expanded 是存储E扩展后的数据
    xored 是存储扩展后的数据与密钥异或后的数据
    sbox_out 是存储经过S盒处理之后的数据
    */

    // 1. E 扩展：32 -> 48
    permute(right, right_expanded, E_Table, 48);

    // 2. 子密钥异或（逐字节）
    // C/C++ 中数组不能直接 ^，只能逐字节处理
    for (int i = 0; i < 6; i++) {
        xored[i] = right_expanded[i] ^ subkeys[i];
    }

    // 3. S 盒代换：48 -> 32
    s_box_substitution(xored, sbox_out);

    // 4. P 盒置换：32 -> 32 （只需要一次）
    permute(sbox_out, output, P_Table, 32);
}


/*-------------------------------------------------------------
                            密钥处理
-------------------------------------------------------------*/
// 循环左移（针对28位密钥部分）
void left_shift(char* key_half, int shifts) {
    /*
    key_half 是指左右半边原始密钥
    shifts 是指位移的大小
    */
    // key_half 是 28 位，放在 4 字节里，高 28 位有效
    for (int s = 0; s < shifts; s++) {
        int first_bit = (key_half[0] >> 7) & 0x01; // 最高位
        // 左移 1 位（只移 28 位）
        key_half[0] = ((key_half[0] << 1) & 0xFF) | ((key_half[1] >> 7) & 0x01);
        key_half[1] = ((key_half[1] << 1) & 0xFF) | ((key_half[2] >> 7) & 0x01);
        key_half[2] = ((key_half[2] << 1) & 0xFF) | ((key_half[3] >> 7) & 0x01);
        key_half[3] = ((key_half[3] << 1) & 0xFF);
        // 把移出去的最高位补到第28位
        key_half[3] |= (first_bit << 4); // 第28位在 byte3 的 bit3
        // 清掉低 4 位，只保留 28 位
        key_half[3] &= 0xF0;
    }
}

void Generate_subkeys(char* key, char subkeys[16][6]) {
    /*
    key 输入 64 位密钥（8 字节）
    subkeys 输出 16 个子密钥，每个 48 位（6 字节）
    */

    char key_pc1[7] = { 0 };
    char left[4] = { 0 };
    char right[4] = { 0 };
    char left_right[7] = { 0 };
    /*
    key_pc1 PC-1 置换后的 56 位密钥
    left 左半部分（28 位）
    right 右半部分（28 位）
    left_right 合并后的 56 位（left||right）
    */

    // 1. PC-1置换
    permute(key, key_pc1, PC1_Table, 56);

    // 2. 分割密钥，左28右28
    memcpy(left, key_pc1, 4);   // 分割key的左边0-31位到left里
    left[3] &= 0xF0;    // 去除末尾的28-31位，从而使得左密钥是28位

    memcpy(right, key_pc1 + 3, 4);  // 分割key的右边24-55位到right里
    /*因为我们需要的是28 - 55位的，所以数组的前4位需要左移舍弃，后4位移到左4位，下一个数组的前4位移到本数组的后四位。*/
    right[0] = (right[0] << 4) | ((right[1] >> 4) & 0x0F);
    right[1] = (right[1] << 4) | ((right[2] >> 4) & 0x0F);
    right[2] = (right[2] << 4) | ((right[3] >> 4) & 0x0F);
    right[3] = (right[3] << 4) & 0xF0;  // 最后一位舍弃前半部分就行了

    // 3. 16 轮生成子密钥
    for (int i = 0; i < 16; i++) {
        left_shift(left, Move_Table[i]);
        left_shift(right, Move_Table[i]);

        // 合并 left||right
        left_right[0] = left[0];
        left_right[1] = left[1];
        left_right[2] = left[2];
        left_right[3] = left[3] | ((right[0] >> 4) & 0x0F);
        left_right[4] = (right[0] << 4) | ((right[1] >> 4) & 0x0F);
        left_right[5] = (right[1] << 4) | ((right[2] >> 4) & 0x0F);
        left_right[6] = (right[2] << 4) | ((right[3] >> 4) & 0x0F);

        // PC-2置换
        permute(left_right, subkeys[i], PC2_Table, 48);
    }

}

/*-------------------------------------------------------------
                           单块DES加密解密
-------------------------------------------------------------*/
void des_encrypt_block(char* fill_plaintext_block, char subkeys[16][6], char* ciphertext) {
    /*
    fill_plaintext_block 是填充后的明文块（64位）
    subkeys[16][6] 是16个子密钥，每个48位
    ciphertext 是存储加密后的密文（64位）
    */
    char ip_out[8] = { 0 };
    char left[4], right[4], temp[4], f_out[4];
    char merged[8] = { 0 };
    /*
    ip_out[8] 是IP置换后的中间结果（64位）
    l[4] 是Feistel网络的左半部分（32位）
    r[4] 是Feistel网络的右半部分（32位）
    temp[4] 是临时存储，用于交换左右部分
    f_out[4] 是F函数的输出（32位）
    merged[8] 是最后一轮交换后的合并数据（64位）
    */

    // 1. 初始置换
    permute(fill_plaintext_block, ip_out, IP_Table, 64);
    memcpy(left, ip_out, 4);   // L0 左4个字节
    memcpy(right, ip_out + 4, 4);// R0 右4个字节

    // 2. 16轮Feistel网络
    for (int i = 0; i < 16; i++) {
        memcpy(temp, right, 4); // 保存右半边的数据
        f_function(right, subkeys[i], f_out);   //进行论函数F
        for (int j = 0; j < 4; j++)
            right[j] = left[j] ^ f_out[j];
        memcpy(left, temp, 4); // 把之前保存右半边的数据复制到左半边
    }

    // 3. 在最后一轮后交换左右
    memcpy(merged, right, 4);
    memcpy(merged + 4, left, 4);

    // 4. 逆初始置换
    permute(merged, ciphertext, IPR_Table, 64);
}

void des_decrypt_block(char* ciphertext_block, char subkeys[16][6], char* plaintext_block) {
    char ip_out[8] = { 0 };
    char left[4], right[4], temp[4], f_out[4];
    char merged[8] = { 0 };

    // 1. 初始置换
    permute(ciphertext_block, ip_out, IP_Table, 64);
    memcpy(left, ip_out, 4);
    memcpy(right, ip_out + 4, 4);

    // 2. 16 轮 Feistel（子密钥倒序）
    for (int i = 15; i >= 0; i--) {
        memcpy(temp, right, 4);
        f_function(right, subkeys[i], f_out);
        for (int j = 0; j < 4; j++)
            right[j] = left[j] ^ f_out[j];
        memcpy(left, temp, 4);
    }

    // 3. 交换左右
    memcpy(merged, right, 4);
    memcpy(merged + 4, left, 4);

    // 4. 逆初始置换
    permute(merged, plaintext_block, IPR_Table, 64);
}

//  填充明文
int Fill_function(char* plaintext, int len, char* fill_plaintext) {
    int fill_len = 8 - (len % 8);
    memcpy(fill_plaintext, plaintext, len);
    for (int i = 0; i < fill_len; i++)
        fill_plaintext[len + i] = fill_len;
    return len + fill_len;
}


// 二进制转十六进制（带字符串结尾）
void bin_to_hex(char* bin, int len, char* hex) {
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 0; i < len; i++) {
        hex[i * 2] = hex_chars[(bin[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[bin[i] & 0x0F];
    }
    hex[len * 2] = '\0'; // 补字符串结束符,需要 hex 至少有 2*len + 1 空间
}

// 十六进制转二进制（带字符串结尾）
void hex_to_bin(const char* hex, int hex_len, char* bin) {
    for (int i = 0; i < hex_len / 2; i++) {
        char c1 = hex[i * 2];
        char c2 = hex[i * 2 + 1];
        int high = (c1 >= '0' && c1 <= '9') ? c1 - '0' : (c1 >= 'A' ? c1 - 'A' + 10 : c1 - 'a' + 10);
        int low = (c2 >= '0' && c2 <= '9') ? c2 - '0' : (c2 >= 'A' ? c2 - 'A' + 10 : c2 - 'a' + 10);
        bin[i] = (high << 4) | low;
    }
}

//  去除 PKCS#7 填充
int Remove_padding(char* data, int len) {
    int pad = data[len - 1];
    if (pad <= 0 || pad > 8) return len; // 简单保护
    return len - pad;
}
/*-------------------------------------------------------------
                           主加密解密函数
-------------------------------------------------------------*/

void Encrypt(char* plaintext, char* private_key, char* ciphertext) {
    // 1. 处理密钥，取前八字节，不足补 0
    char key[8] = { 0 };
    memcpy(key, private_key, 8);

    // 2. 生成16个子密钥
    char subkeys[16][6];
    Generate_subkeys(key, subkeys);

    // 3. 填充明文
    char fill_plaintext[MAXSIZE + 8] = { 0 }; // 最多多 8 字节 padding
    int fill_len = Fill_function(plaintext, (int)strlen(plaintext), fill_plaintext);

    // 4. 分组加密得到二进制密文
    char cipher_bin[MAXSIZE + 8] = { 0 };
    for (int i = 0; i < fill_len; i += 8) {
        des_encrypt_block(fill_plaintext + i, subkeys, cipher_bin + i);
    }

    // 5. 二进制密文 -> 十六进制字符串输出到 ciphertext
    bin_to_hex(cipher_bin, fill_len, ciphertext);
}
void Encrypt_NoPadding(char* plaintext, char* private_key, char* ciphertext) {
    // 1. 处理密钥，取前八字节，不足补 0
    char key[8] = { 0 };
    memcpy(key, private_key, 8);

    // 2. 生成16个子密钥
    char subkeys[16][6];
    Generate_subkeys(key, subkeys);

    // 3. 明文长度（无填充，必须是8的倍数）
    int len = (int)strlen(plaintext);
    if (len % 8 != 0) {
        printf("Error: plaintext length must be multiple of 8 (no padding).\n");
        ciphertext[0] = '\0';
        return;
    }

    // 4. 分组加密得到二进制密文
    char cipher_bin[MAXSIZE + 8] = { 0 };
    for (int i = 0; i < len; i += 8) {
        des_encrypt_block(plaintext + i, subkeys, cipher_bin + i);
    }

    // 5. 二进制密文 -> 十六进制字符串输出到 ciphertext
    bin_to_hex(cipher_bin, len, ciphertext);
}

void Decrypt(char* ciphertext_hex, char* private_key, char* plaintext) {
    // 1. 密钥
    char key[8] = { 0 };
    memcpy(key, private_key, 8);

    // 2. 子密钥
    char subkeys[16][6];
    Generate_subkeys(key, subkeys);

    // 3. 密文 hex -> bin
    int hex_len = (int)strlen(ciphertext_hex);
    int bin_len = hex_len / 2;
    char cipher_bin[MAXSIZE + 8] = { 0 };
    hex_to_bin(ciphertext_hex, hex_len, cipher_bin);

    // 4. 分块解密
    char plain_bin[MAXSIZE + 8] = { 0 };
    for (int i = 0; i < bin_len; i += 8) {
        des_decrypt_block(cipher_bin + i, subkeys, plain_bin + i);
    }

    // 5. 去填充
    int plain_len = Remove_padding(plain_bin, bin_len);
    memcpy(plaintext, plain_bin, plain_len);
    plaintext[plain_len] = '\0';
}

/*-------------------------------------------------------------
                           主函数
-------------------------------------------------------------*/
/*
int main() {
    char plaintext[MAXSIZE] = "I_LOVE_FISCHL";
    char private_key[9] = "KLEEKLEE";

    // 加密输出（十六进制字符串）
    char ciphertext_hex[(MAXSIZE + 8) * 2 + 1] = { 0 };

    // 解密输出（原文字符串）
    char decrypted_text[MAXSIZE] = { 0 };

    printf("\nPLAINTEXT is : %s\n", plaintext);
    printf("\nPRIVATE KEY is : %s\n", private_key);

    // 加密
    Encrypt(plaintext, private_key, ciphertext_hex);
    printf("\nCIPHERTEXT is : %s\n", ciphertext_hex);

    // 解密（从刚刚生成的密文还原）
    Decrypt(ciphertext_hex, private_key, decrypted_text);
    printf("\nDECRYPTED PLAINTEXT is : %s\n", decrypted_text);

    system("pause");
    return 0;
}
*/
/*
    char plaintext[MAXSIZE] = "I_LOVE_FISCHL";
    char private_key[9] = "KLEEKLEE";
    C5C0D76A51D4CB63150945220A1A093A
*/