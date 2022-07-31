#include <stdio.h>

int main() {

	int val = sum(1,1,1,1,1,1,1,1);

	printf("%d", val);

}

int sum(int a, int b, int c, int d, int e, int f, int g) {
	return a+b+c+d+e+f+g;
}
