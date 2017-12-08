#include <iostream>
#include <string>
#include <sstream>
using namespace std;
// RSA 加密算法简单实现
// 明文=18位机器码+2位天数，4个一组，分5组
// 密文为15位大写字母，3个一组，分5组
#define rsaD 1873
#define VECTOR_SIZE 5 // 18位机器码+2位天数限制，4个数字一组，共5组
#define REG_CODE_LEN 15

int candp1(int a,int b,int c);									// 数据处理函数，实现幂的取余运算,result = a^b%c
bool regstr2arr(string regcode,int arr[VECTOR_SIZE]);			// 注册码转5维向量
string arr2mcodestr(int arr[VECTOR_SIZE]);						// 5位向量转机器码+天数
int al32num(string s);											// 3位字符转数字
string DecryptMachineCode(string regstr, int n, int d=1873);