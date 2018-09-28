#include <frameworks/ServiceManager.h>
#include <frameworks/InputManager.h>
#include <frameworks/EventDevice.h>
#include <frameworks/InputReader.h>


#include <unistd.h>
#include <stdio.h>




static void dispatchEvent(void *data) 
{
    CookEvent *cv = (CookEvent *) data;
	BnInputManager::get()->dispatchEvent(cv);

    delete cv;
}




static void inputReaderThread(void *ptr)
{
	struct CookEvent cookEvent[128];
	int iRet;
	int i;

	EventDevice::scanDevice("/dev/input");
	EventDevice::scanDevice("/dev");
	while (1) {
		iRet = InputReader::getInputReader()->readOnce(cookEvent, 128);
		if (iRet <= 0) {
			continue;
		}
		for (i = 0; i < iRet; i++) {
			CookEvent *cv = new CookEvent();
			*cv = cookEvent[i];
			Task *task = new Task(dispatchEvent, cv);		
			BnInputManager::get()->mDispatchthread->add_task(task);
		}
	}
}


int main(int argc, char **argv)
{
	Binder::getBinder()->binderSetMaxthreads(2);

	IServiceManager *sm = interface_cast<IServiceManager>(BINDER_SERVICE_MANAGER);
	sm->addService(IInputManager::serviceName, BnInputManager::get());

	
	BnInputManager::get()->mInputReaderthread->set_thread_start_cb(inputReaderThread, NULL);
	BnInputManager::get()->mInputReaderthread->start();
	BnInputManager::get()->mDispatchthread->start();

	Binder::getBinder()->binderLoop();
	return 0;
}
