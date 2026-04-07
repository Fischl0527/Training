#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "AEStable.h"

#define MAXSIZE 100

/*-------------------------------------------------------------
                            核心工具函数
-------------------------------------------------------------*/
// 有限域乘法（GF(2^8)）
unsigned char gf_mul(unsigned char a, unsigned char b) {
    /*
    a 是被乘数（1字节）
    b 是乘数（1字节）
    返回值是 GF(2^8) 下的乘积（AES 使用的不可约多项式）
    */
    unsigned char res = 0;
    unsigned char temp = a;
    /*
    res 保存累加结果
    temp 保存当前被乘数（每轮左移一次）
    算法相当于 GF(2^8) 的“俄罗斯农夫乘法”：
    1) 如果 b 的最低位为 1，就把 temp 异或进 res
    2) temp 左移 1 位，如果溢出（最高位为1）则与 0x1B 异或约简
    3) b 右移 1 位
    */
    for (int i = 0; i < 8; i++) {
        if (b & 0x01) res ^= temp;
        unsigned char hi = temp & 0x80;
        temp <<= 1;
        if (hi) temp ^= 0x1B;
        b >>= 1;
    }
    return res;
}

// 二进制转十六进制（带字符串结尾）
void bin_to_hex(unsigned char* bin, int len, char* hex) {
    /*
    bin 是二进制数组
    len 是字节长度
    hex 是输出的十六进制字符串（每字节2个字符，末尾补 '\0'）
    */
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 0; i < len; i++) {
        hex[i * 2] = hex_chars[(bin[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[bin[i] & 0x0F];
    }
    hex[len * 2] = '\0';
}

// 十六进制转二进制
void hex_to_bin(const char* hex, int hex_len, unsigned char* bin) {
    /*
    hex 是十六进制字符串
    hex_len 是字符串长度（必须是偶数）
    bin 是输出的二进制数组
    */
    for (int i = 0; i < hex_len / 2; i++) {
        char c1 = hex[i * 2];
        char c2 = hex[i * 2 + 1];
        int high = (c1 >= '0' && c1 <= '9') ? c1 - '0' : (c1 >= 'A' ? c1 - 'A' + 10 : c1 - 'a' + 10);
        int low = (c2 >= '0' && c2 <= '9') ? c2 - '0' : (c2 >= 'A' ? c2 - 'A' + 10 : c2 - 'a' + 10);
        bin[i] = (unsigned char)((high << 4) | low);
    }
}

// PKCS#7 填充
int Fill_function(const unsigned char* plaintext, int len, unsigned char* fill_plaintext) {
    /*
    plaintext 是原始明文
    len 是明文字节长度
    fill_plaintext 是填充后的输出
    返回值是填充后的长度
    */
    int fill_len = 16 - (len % 16);
    memcpy(fill_plaintext, plaintext, len);
    for (int i = 0; i < fill_len; i++) {
        fill_plaintext[len + i] = (unsigned char)fill_len;
    }
    return len + fill_len;
}

// 去除 PKCS#7 填充
int Remove_padding(unsigned char* data, int len) {
    /*
    data 是填充后的数据
    len 是数据长度
    返回值是去填充后的长度
    */
    int pad = data[len - 1];
    if (pad <= 0 || pad > 16) return len;
    return len - pad;
}

/*-------------------------------------------------------------
                            AES核心变换
-------------------------------------------------------------*/
void add_round_key(unsigned char state[4][4], const unsigned char* round_key) {
    /*
    state 是 4x4 状态矩阵（按列存放）
    round_key 是当前轮密钥（16字节）
    将状态矩阵的每个字节与轮密钥对应字节异或
    */
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            state[r][c] ^= round_key[c * 4 + r];
        }
    }
}

void sub_bytes(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    对每个字节进行 S 盒代换
    */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = AES_SBOX[state[r][c]];
        }
    }
}

void inv_sub_bytes(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    对每个字节进行逆 S 盒代换
    */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = AES_INV_SBOX[state[r][c]];
        }
    }
}

void shift_rows(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    第0行：不移位
    第1行：左移1
    第2行：左移2
    第3行：左移3
    */
    unsigned char tmp;
    // row 1 左移 1
    tmp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = tmp;
    // row 2 左移 2
    tmp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = tmp;
    tmp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = tmp;
    // row 3 左移 3
    tmp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = tmp;
}

void inv_shift_rows(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    逆 ShiftRows：
    第0行：不移位
    第1行：右移1
    第2行：右移2
    第3行：右移3
    */
    unsigned char tmp;
    // row 1 右移 1
    tmp = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = tmp;
    // row 2 右移 2
    tmp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = tmp;
    tmp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = tmp;
    // row 3 右移 3
    tmp = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = tmp;
}

void mix_columns(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    MixColumns 将每一列与固定矩阵相乘：
    [02 03 01 01]
    [01 02 03 01]
    [01 01 02 03]
    [03 01 01 02]
    运算在 GF(2^8) 中进行
    */
    for (int c = 0; c < 4; c++) {
        unsigned char a0 = state[0][c];
        unsigned char a1 = state[1][c];
        unsigned char a2 = state[2][c];
        unsigned char a3 = state[3][c];
        state[0][c] = (unsigned char)(gf_mul(0x02, a0) ^ gf_mul(0x03, a1) ^ a2 ^ a3);
        state[1][c] = (unsigned char)(a0 ^ gf_mul(0x02, a1) ^ gf_mul(0x03, a2) ^ a3);
        state[2][c] = (unsigned char)(a0 ^ a1 ^ gf_mul(0x02, a2) ^ gf_mul(0x03, a3));
        state[3][c] = (unsigned char)(gf_mul(0x03, a0) ^ a1 ^ a2 ^ gf_mul(0x02, a3));
    }
}

void inv_mix_columns(unsigned char state[4][4]) {
    /*
    state 是 4x4 状态矩阵
    逆 MixColumns 使用矩阵：
    [0E 0B 0D 09]
    [09 0E 0B 0D]
    [0D 09 0E 0B]
    [0B 0D 09 0E]
    */
    for (int c = 0; c < 4; c++) {
        unsigned char a0 = state[0][c];
        unsigned char a1 = state[1][c];
        unsigned char a2 = state[2][c];
        unsigned char a3 = state[3][c];
        state[0][c] = (unsigned char)(gf_mul(0x0E, a0) ^ gf_mul(0x0B, a1) ^ gf_mul(0x0D, a2) ^ gf_mul(0x09, a3));
        state[1][c] = (unsigned char)(gf_mul(0x09, a0) ^ gf_mul(0x0E, a1) ^ gf_mul(0x0B, a2) ^ gf_mul(0x0D, a3));
        state[2][c] = (unsigned char)(gf_mul(0x0D, a0) ^ gf_mul(0x09, a1) ^ gf_mul(0x0E, a2) ^ gf_mul(0x0B, a3));
        state[3][c] = (unsigned char)(gf_mul(0x0B, a0) ^ gf_mul(0x0D, a1) ^ gf_mul(0x09, a2) ^ gf_mul(0x0E, a3));
    }
}

/*-------------------------------------------------------------
                            密钥扩展（AES-128）
-------------------------------------------------------------*/
void key_expansion(const unsigned char* key, unsigned char* round_keys) {
    /*
    key 是 16 字节 AES-128 密钥
    round_keys 是扩展后的轮密钥（11 * 16 = 176字节）
    通过 RotWord + SubWord + Rcon 生成每轮子密钥
    */
    // round_keys 大小 176 字节（11 个轮密钥 * 16）
    memcpy(round_keys, key, 16);
    int bytes_generated = 16;
    int rcon_index = 1;
    unsigned char temp[4];

    while (bytes_generated < 176) {
        temp[0] = round_keys[bytes_generated - 4];
        temp[1] = round_keys[bytes_generated - 3];
        temp[2] = round_keys[bytes_generated - 2];
        temp[3] = round_keys[bytes_generated - 1];

        if (bytes_generated % 16 == 0) {
            // RotWord
            unsigned char t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            temp[0] = AES_SBOX[temp[0]];
            temp[1] = AES_SBOX[temp[1]];
            temp[2] = AES_SBOX[temp[2]];
            temp[3] = AES_SBOX[temp[3]];
            // Rcon
            temp[0] ^= AES_RCON[rcon_index++];
        }

        for (int i = 0; i < 4; i++) {
            round_keys[bytes_generated] = round_keys[bytes_generated - 16] ^ temp[i];
            bytes_generated++;
        }
    }
}

/*-------------------------------------------------------------
                        单块AES加密/解密
-------------------------------------------------------------*/
void aes_encrypt_block(const unsigned char* input, const unsigned char* round_keys, unsigned char* output) {
    /*
    input 是 16 字节明文块
    round_keys 是扩展后的轮密钥（176字节）
    output 是 16 字节密文块
    加密流程：
    1) AddRoundKey（初始轮）
    2) 9 轮：SubBytes -> ShiftRows -> MixColumns -> AddRoundKey
    3) 最后一轮：SubBytes -> ShiftRows -> AddRoundKey（不做 MixColumns）
    */
    unsigned char state[4][4];

    // 加载状态矩阵（按列）
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            state[r][c] = input[c * 4 + r];
        }
    }

    add_round_key(state, round_keys);

    for (int round = 1; round <= 9; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, round_keys + round * 16);
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, round_keys + 160);

    // 输出
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            output[c * 4 + r] = state[r][c];
        }
    }
}

void aes_decrypt_block(const unsigned char* input, const unsigned char* round_keys, unsigned char* output) {
    /*
    input 是 16 字节密文块
    round_keys 是扩展后的轮密钥（176字节）
    output 是 16 字节明文块
    解密流程：
    1) AddRoundKey（最后一轮密钥）
    2) 9 轮：InvShiftRows -> InvSubBytes -> AddRoundKey -> InvMixColumns
    3) 最后一轮：InvShiftRows -> InvSubBytes -> AddRoundKey
    */
    unsigned char state[4][4];

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            state[r][c] = input[c * 4 + r];
        }
    }

    add_round_key(state, round_keys + 160);

    for (int round = 9; round >= 1; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, round_keys + round * 16);
        inv_mix_columns(state);
    }

    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, round_keys);

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            output[c * 4 + r] = state[r][c];
        }
    }
}

/*-------------------------------------------------------------
                        主加密/解密函数
-------------------------------------------------------------*/
void AES_Encrypt(const unsigned char* plaintext, const unsigned char* key, char* ciphertext_hex) {
    /*
    plaintext 是原始明文
    key 是 16 字节 AES 密钥
    ciphertext_hex 是输出十六进制密文字符串
    本函数使用 PKCS#7 填充 + ECB 模式
    */
    unsigned char round_keys[176] = {0};
    key_expansion(key, round_keys);

    unsigned char fill_plaintext[MAXSIZE + 16] = {0};
    int fill_len = Fill_function(plaintext, (int)strlen((const char*)plaintext), fill_plaintext);

    unsigned char cipher_bin[MAXSIZE + 16] = {0};
    for (int i = 0; i < fill_len; i += 16) {
        aes_encrypt_block(fill_plaintext + i, round_keys, cipher_bin + i);
    }

    bin_to_hex(cipher_bin, fill_len, ciphertext_hex);
}

void AES_Decrypt(const char* ciphertext_hex, const unsigned char* key, char* plaintext) {
    /*
    ciphertext_hex 是十六进制密文字符串
    key 是 16 字节 AES 密钥
    plaintext 是解密后的明文输出
    本函数使用 PKCS#7 去填充 + ECB 模式
    */
    unsigned char round_keys[176] = {0};
    key_expansion(key, round_keys);

    int hex_len = (int)strlen(ciphertext_hex);
    int bin_len = hex_len / 2;
    unsigned char cipher_bin[MAXSIZE + 16] = {0};
    hex_to_bin(ciphertext_hex, hex_len, cipher_bin);

    unsigned char plain_bin[MAXSIZE + 16] = {0};
    for (int i = 0; i < bin_len; i += 16) {
        aes_decrypt_block(cipher_bin + i, round_keys, plain_bin + i);
    }

    int plain_len = Remove_padding(plain_bin, bin_len);
    memcpy(plaintext, plain_bin, plain_len);
    plaintext[plain_len] = '\0';
}

/*-------------------------------------------------------------
                               主函数
-------------------------------------------------------------*/
int main() {
    unsigned char plaintext[MAXSIZE] = "I_LOVE_FISCHL";
    unsigned char key[17] = "KLEEKLEE12345678"; // 16字节密钥

    char ciphertext_hex[(MAXSIZE + 16) * 2 + 1] = { 0 };
    char decrypted_text[MAXSIZE] = {0};

    printf("\nPLAINTEXT is : %s\n", plaintext);
    printf("\nKEY is : %s\n", key);

    AES_Encrypt(plaintext, key, ciphertext_hex);
    printf("\nCIPHERTEXT is : %s\n", ciphertext_hex);

    AES_Decrypt(ciphertext_hex, key, decrypted_text);
    printf("\nDECRYPTED TEXT is : %s\n", decrypted_text);

    system("pause");
    return 0;
}
