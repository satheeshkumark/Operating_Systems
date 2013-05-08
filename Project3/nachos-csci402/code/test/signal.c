#include "syscall.h"
int main()
{
	int lock;
	int cv;
	
	cv= CreateCV("lock",4);
	lock= CreateLock("cv",2);
	
	AcquireLock(lock);
	SignalCV(lock,cv);
	ReleaseLock(lock);
	
	Exit(0);
    
}
