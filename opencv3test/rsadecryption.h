#include <iostream>
#include <string>
#include <sstream>
using namespace std;
// RSA �����㷨��ʵ��
// ����=18λ������+2λ������4��һ�飬��5��
// ����Ϊ15λ��д��ĸ��3��һ�飬��5��
#define rsaD 1873
#define VECTOR_SIZE 5 // 18λ������+2λ�������ƣ�4������һ�飬��5��
#define REG_CODE_LEN 15

int candp1(int a,int b,int c);									// ���ݴ�������ʵ���ݵ�ȡ������,result = a^b%c
bool regstr2arr(string regcode,int arr[VECTOR_SIZE]);			// ע����ת5ά����
string arr2mcodestr(int arr[VECTOR_SIZE]);						// 5λ����ת������+����
int al32num(string s);											// 3λ�ַ�ת����
string DecryptMachineCode(string regstr, int n, int d=1873);