#include <frameworks/ServiceManager.h>
#include <frameworks/TestService.h>
#include <frameworks/InputManager.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>


#if 0
int main(int argc, char **argv)
{
	int iHandle;
	
	IServiceManager *sm = interface_cast<IServiceManager>(BINDER_SERVICE_MANAGER);
	iHandle = sm->getService(ITestService::serviceName);

	printf("iHandle = %d !\n", iHandle);
	ITestService *testservice = interface_cast<ITestService>(iHandle);
	
	if (testservice->hello("android binder ok!"))
		printf("testservice->hello error!\n");

	Binder::getBinder()->binderRelease(iHandle);
	return 0;
}
#else

int main(int argc, char **argv)
{
	struct CookEvent tCookEvent;
	int iHandle;
	int iPid;
	int fd[2];
	struct ListenerDesc *ptListenerDesc;
	
	IServiceManager *sm = interface_cast<IServiceManager>(BINDER_SERVICE_MANAGER);
	iHandle = sm->getService(IInputManager::serviceName);

	printf("iHandle = %d !\n", iHandle);
	IInputManager *inputManager = interface_cast<IInputManager>(iHandle);

	
	iPid = getpid();
	socketpair(PF_LOCAL, SOCK_STREAM, 0, fd);

	ptListenerDesc = (struct ListenerDesc *)malloc(sizeof(struct ListenerDesc));

	ptListenerDesc->iFd = fd[1];
	ptListenerDesc->iType = TOUCHSCREEN;
	ptListenerDesc->iUid = (iPid << 8) | (ptListenerDesc->iFd);
	ptListenerDesc->pcName = "test";
	
	inputManager->addListener(ptListenerDesc);


	while (1) {
		char buf[512];
		int iSize;
		iSize = read(fd[0], &tCookEvent, sizeof(struct CookEvent));
		if (iSize < 0) {
			printf("read error!\n");
			continue;
		}

		printf("tCookEvent.iX = %d\n", tCookEvent.iX);
		printf("tCookEvent.iY = %d\n", tCookEvent.iY);
		printf("tCookEvent.iTrackID = %d\n", tCookEvent.iTrackID);
		printf("tCookEvent.iPressure = %d\n", tCookEvent.iPressure);
	}


	Binder::getBinder()->binderRelease(iHandle);
	return 0;
}
#endif


