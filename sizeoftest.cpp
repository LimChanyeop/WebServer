#include <iostream>
#include <stdio.h>

#define BYTE_SIZE 13

struct sizeoftest
{
	int i;
	char c[BYTE_SIZE];
};

int main()
{
	sizeoftest data;
	std::cout << "int-" << sizeof(int) << " char-" << sizeof(char) << " float-" << sizeof(float) << " double-"
			  << sizeof(double) << std::endl;
	std::cout << "data-" << sizeof(data);
	return 0;
}

// 구조체에서 가장 큰 자료형의 크기의 배수로 정렬
//(int형이 제일 클 경우 4바이트씩 정렬, 남은 부분은 자료형 채우고 패딩)