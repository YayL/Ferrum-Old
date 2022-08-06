#include <stdlib.h>
#include <stdio.h>

int other_func(int a, int b) {
	
	int c = a == b ? 1 : 2;

	if (a == b) {
		return 3;
	} else if (a > 4) {
		return 4;
	} else {
		return a + b;
	}
}

int main() {

	int res = other_func(2, 4);

	printf("%d\n", res);

}
