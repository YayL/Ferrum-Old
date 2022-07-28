#include <stdlib.h>
#include <stdio.h>

int other_func(int a, int b) {

	int c = a + b;
	return c;
}

int main() {

	int res = other_func(2, 4);

	printf("%d\n", res);

}
