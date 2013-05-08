#include "syscall.h"

int main(){
	Print("Simulation 0 started");
	Exec("../test/project2",16);
	Print("Simulation 1 started");
	Exec("../test/project2",16);
	Exit(0);
}