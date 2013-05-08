#include "syscall.h"

int main()
 {
	Print("\nIn main thread");
	Exec("../test/matmult",15);
	Exec("../test/matmult",15);
	Exit(0);   
}
