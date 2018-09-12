#include <frameworks/ServiceManager.h>


int main(int argc, char **argv)
{
	BnServiceManager *serviceManager = new BnServiceManager();

	serviceManager->binderSetMaxthreads(4);
	serviceManager->binderBecomeContextManager();

	serviceManager->binderLoop();
	return 0;
}