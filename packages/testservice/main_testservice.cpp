#include <frameworks/ServiceManager.h>
#include <frameworks/TestService.h>
#include <frameworks/InputManager.h>
#include <unistd.h>
#include <stdio.h>




int main(int argc, char **argv)
{
	Binder::getBinder()->binderSetMaxthreads(2);

	IServiceManager *sm = interface_cast<IServiceManager>(BINDER_SERVICE_MANAGER);
	sm->addService(ITestService::serviceName, BnTestService::get());


	Binder::getBinder()->binderLoop();
	return 0;
}

