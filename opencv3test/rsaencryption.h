#include <iostream>
#include <string>
using namespace std;
// RSA �����㷨��ʵ��
// ����=18λ������+2λ������4��һ�飬��5��
// ����Ϊ15λ��д��ĸ��3��һ�飬��5��
#define rsaP 67
#define rsaQ 211
#define rsaE 37 // d=1873    13*6397 
#define VECTOR_SIZE 5 // 18λ������+2λ�������ƣ�4������һ�飬��5��
#define M_CODE_LEN 20
#define REG_CODE_LEN 15

int candp(int a,int b,int c);									// result = a^b%c
bool mcodestr2arr(string machinecode, int arr[VECTOR_SIZE]);	// ������+����תΪ5ά����,��ȷ����true
string num2al3(int a);											// ����ת3λ�ַ�
string arr2regstr(int arr[VECTOR_SIZE]);						// 5ά����תע����
string EncryptMachineCode(string machinecode, int n, int e);
