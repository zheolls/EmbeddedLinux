// CMakeProject1.cpp: 定义应用程序的入口点。
//

#include "CMakeProject1.h"

using namespace std;

int main()
{
	int a;
	int sum;
	for (int a = 0; a < 5; a++) {
		cout << a << endl;

		sum += a;
	}
	cout << "Hello CMake。" << endl;
	return 0;
}
