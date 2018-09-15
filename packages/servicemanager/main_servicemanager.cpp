#include <frameworks/ServiceManager.h>


int main(int argc, char **argv)
{
	BnServiceManager *serviceManager = new BnServiceManager();
	serviceManager->binderOpen();
	serviceManager->binderSetMaxthreads(4);
	serviceManager->binderBecomeContextManager();
	

	serviceManager->binderLoop();
	serviceManager->binderClose();
	return 0;
}