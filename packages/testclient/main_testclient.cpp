#include <frameworks/ServiceManager.h>
#include <frameworks/TestService.h>
#include <stdio.h>



int main(int argc, char **argv)
{
	int iHandle;
	Binder *binder = new Binder();

	binder->binderOpen();
	
	IServiceManager *sm = interface_cast<IServiceManager>(binder, BINDER_SERVICE_MANAGER);
	iHandle = sm->getService((unsigned char *)ITestService::serviceName);

	printf("iHandle = %d !\n", iHandle);
	ITestService *testservice = interface_cast<ITestService>(binder, iHandle);
	
	if (testservice->hello((const unsigned char *)"android binder ok!"))
		printf("testservice->hello error!\n");

	binder->binderRelease(iHandle);
	return 0;
}

