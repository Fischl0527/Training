#ifndef AES_TABLE_H
#define AES_TABLE_H

// AES S盒
extern const unsigned char AES_SBOX[256];

// AES 逆S盒
extern const unsigned char AES_INV_SBOX[256];

// AES Rcon 常量（轮常量）
extern const unsigned char AES_RCON[11];

#endif
