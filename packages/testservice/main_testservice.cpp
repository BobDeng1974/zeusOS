#include <frameworks/ServiceManager.h>
#include <frameworks/TestService.h>




int main(int argc, char **argv)
{
	BnTestService *testService = new BnTestService();
	

	testService->binderOpen();
	IServiceManager *sm = interface_cast<IServiceManager>(testService, BINDER_SERVICE_MANAGER);

	sm->addService((unsigned char *)ITestService::serviceName, testService);


	testService->binderSetMaxthreads(2);
	

	testService->binderLoop();
	testService->binderClose();

	return 0;
}

