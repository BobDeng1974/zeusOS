#include <frameworks/ServiceManager.h>
#include <frameworks/TestService.h>




int main(int argc, char **argv)
{
	IServiceManager *sm = interface_cast<IServiceManager>(BINDER_SERVICE_MANAGER);
	BnTestService *testService = new BnTestService();
	
	sm->addService((unsigned char *)ITestService::serviceName, testService);


	testService->binderSetMaxthreads(2);
	

	testService->binderLoop();
	return 0;
}

