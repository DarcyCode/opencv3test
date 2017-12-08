#include <iostream>
#include <string>
using namespace std;
// RSA 加密算法简单实现
// 明文=18位机器码+2位天数，4个一组，分5组
// 密文为15位大写字母，3个一组，分5组
#define rsaP 67
#define rsaQ 211
#define rsaE 37 // d=1873    13*6397 
#define VECTOR_SIZE 5 // 18位机器码+2位天数限制，4个数字一组，共5组
#define M_CODE_LEN 20
#define REG_CODE_LEN 15

int candp(int a,int b,int c);									// result = a^b%c
bool mcodestr2arr(string machinecode, int arr[VECTOR_SIZE]);	// 机器码+天数转为5维向量,正确返回true
string num2al3(int a);											// 数字转3位字符
string arr2regstr(int arr[VECTOR_SIZE]);						// 5维向量转注册码
string EncryptMachineCode(string machinecode, int n, int e);
