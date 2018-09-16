#include <frameworks/ServiceManager.h>


int main(int argc, char **argv)
{
	Binder::getBinder()->binderSetMaxthreads(4);
	Binder::getBinder()->binderBecomeContextManager();
	
	Binder::getBinder()->setBnBinder(BnServiceManager::get());
	Binder::getBinder()->binderLoop();
	return 0;
}