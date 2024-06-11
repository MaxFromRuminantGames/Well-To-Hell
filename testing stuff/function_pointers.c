#include <stdlib.h>
#include <stdio.h>

//void testFuncA(int a)
//{
//	printf("a = %d\n", a);
//}

void testFuncB(int b)
{
	printf("b = %d\n", b + 1);
}

int main()
{
	int a = 2;
	void (*pTestFunc[])(int) = { (void(*)(int))printf("a = %d\n", (int)), testFuncB };

	//for(int i = 0; i < 2; i++) pTestFunc[i](2);
	pTestFunc[0](2);
	pTestFunc[1](2);

	return 0;
}
