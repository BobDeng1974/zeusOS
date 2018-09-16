#ifndef __INPUT_READER_H__
#define __INPUT_READER_H__

/* 头文件做前置声明防止互相包含产生未定义错误 */
struct CookEvent;
class InputReader;

#include <iostream>
#include <list>
#include <sys/time.h>
#include <sys/stat.h>
#include <frameworks/Device.h>



class InputReader {
public:
	~InputReader();
	void addDevice(Device* device);
	int delDevice(char *pcDevicePath);
	Device* findDevice(char *pcDevicePath);
	Device* findDevice(int iFd);
	void printAllDevice(void);
	
	int addEpollFd(int iFd);
	int delEpollFd(int iFd);

	int readOnce(struct CookEvent* cookEvent, int iSize);	
	static InputReader* getInputReader(void);
	
private:
	std::list<Device*> deviceList;
	int mEpollFd;
	static InputReader *inputReader;
	static pthread_mutex_t tMutex;
	InputReader();	 //把构造函数私有化，避免使用构造函数构造另一个实例化对象
};



#endif // __INPUT_READER_H__







