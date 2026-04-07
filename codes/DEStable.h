#ifndef DES_TABLE_H
#define DES_TABLE_H

// IP表，初始置换表
extern const char IP_Table[64];

// IPR表，逆初始置换表
extern const char IPR_Table[64];

// E表
extern const char E_Table[48];

// PC1表
extern const char PC1_Table[56];

// PC2表
extern const char PC2_Table[48];

// 移位表
extern const char Move_Table[16];

// S盒
extern const char S_Box[8][4][16];

// P盒
extern const char P_Table[32];

#endif
