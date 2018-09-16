#include <frameworks/EventDevice.h>
#include <frameworks/InputReader.h>
#include <frameworks/InputManager.h>
#include <string.h>
#include <sys/types.h>
#include <linux/input.h>
#include <frameworks/Calibrate.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ABS_MT_SLOT		0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */


static void CookTouchEvent(struct input_event& iev, struct CookEvent* cookEvent)
{
	//Calibrate *calibrate = Calibrate::getCalibrate();
	//struct CalibrateInfo& calibrateInfo = calibrate->getCalibrateInfo();

	cookEvent->iType = TOUCHSCREEN;
	switch (iev.code) {
#if 0		
		case ABS_X:
		{		
			if (calibrate->isCalibrated()) {
				if (calibrateInfo.mInterchangeXY) {
					/* 应该互换X Y 所以这里把本来赋给X的值赋给Y*/
					cookEvent->iY = ((iev.value - calibrateInfo.iYLc) / calibrateInfo.iKy) + calibrateInfo.iYc;
				}
				else {
					cookEvent->iX = ((iev.value - calibrateInfo.iXLc) / calibrateInfo.iKx) + calibrateInfo.iXc;
				}				
			}
			else {
				/* 没有校准的话就返回原值 */
				cookEvent->iX = iev.value;
			}
			break;
		}
	
		case ABS_Y:
		{
			if (calibrate->isCalibrated()) {
				if (calibrateInfo.mInterchangeXY) {
					/* 应该互换X Y 所以这里把本来赋给Y的值赋给X*/
					cookEvent->iX = ((iev.value - calibrateInfo.iXLc) / calibrateInfo.iKx) + calibrateInfo.iXc;				
				}
				else {
					cookEvent->iY = ((iev.value - calibrateInfo.iYLc) / calibrateInfo.iKy) + calibrateInfo.iYc;
				}
			}
			else {
				/* 没有校准的话就返回原值 */
				cookEvent->iY = iev.value;
			}
			break;	
		}
	
		case ABS_PRESSURE:
		{
			cookEvent->iPressure = iev.value;
			break;	
		}
#endif
		case ABS_MT_POSITION_X:
		{
			cookEvent->iX = iev.value;
			cookEvent->iPressure = 1;	//tiny4412触摸屏没有传iPressure值
			break;	
		}
		
		case ABS_MT_POSITION_Y:
		{
			cookEvent->iY = iev.value;
			break;	
		}
	
		case ABS_MT_TOUCH_MAJOR:
		{
			cookEvent->iTouchMajor = iev.value;
			break;	
		}
		
		case ABS_MT_TRACKING_ID:
		{
			cookEvent->iTrackID = iev.value;
			break;	
		}
		default :
		{
			printf("Not About Knowing iev.code : %d\n", iev.code);
		}
	}


}


static void CookKeyEvent(struct input_event& iev, struct CookEvent* cookEvent)
{
	cookEvent->iKey  = iev.code;

	if (iev.code == BTN_TOUCH) {
		cookEvent->iType = TOUCHSCREEN;
	}
	else {
		cookEvent->iType = KEYEVENT;		
	}
}


static int isCharDevice_Static(char *strFilePath)
{
    struct stat tStat;


    if ((stat(strFilePath, &tStat) == 0) && S_ISCHR(tStat.st_mode)) {
        return 1;
    }
    else {
        return 0;
    }
}


int EventDevice::scanDevice(std::string devicePath)
{
	DIR *pDir;
	struct dirent *file;

	if(!(pDir = opendir(devicePath.c_str()))) {
		printf("error opendir %s!!!/n", devicePath.c_str());
		return -1;
	}

	while((file = readdir(pDir)) != NULL) {
		if(strncmp(file->d_name, ".", 1) == 0)
			continue;

		char strTmp[256];			
		snprintf(strTmp, 256, "%s/%s", devicePath.c_str(), file->d_name);
		strTmp[255] = '\0';
		if (isCharDevice_Static(strTmp)) {
			if (strncmp(file->d_name, "event", 5) == 0) {
				int iFd = open(strTmp, O_RDWR);
				if (iFd < 0) {
					fprintf(stderr, "open device %s error\n", strTmp);
					return -1;
				}
				
				EventDevice *device = new EventDevice();
				device->setFd(iFd);
				device->setDeviceName(strTmp);
				InputReader::getInputReader()->addDevice(device);	
				
			}
		}

	}
	closedir(pDir);

	return 0;

}


int EventDevice::process(int iFd, struct CookEvent* cookEvent, int iSize)
{
	
	struct input_event readBuffer[256];
	size_t capacity = 256;
	struct CookEvent* cev = cookEvent;
	int iRet = 0;
	static int iFlagPressure = 0;

	int32_t readSize = read(iFd, readBuffer,sizeof(struct input_event) * capacity);
	if (readSize == 0 || (readSize < 0 && errno == ENODEV)) {
		/* 这个设备已经被移除，但是还没有处理inotifiy事件 */
		//printf("Device was removed before INotify noticed.\n");
		return -1;
	} else if (readSize < 0) {
		if (errno != EAGAIN && errno != EINTR) {
			fprintf(stderr, "could not get event (errno=%d)", errno);
			return -1;
		}
	} else if ((readSize % sizeof(struct input_event)) != 0) {
		fprintf(stderr, "could not get event (wrong size: %d)", readSize);
		return -1;
	}
		
	size_t count = size_t(readSize) / sizeof(struct input_event);
	for (size_t i = 0; i < count; i++) {
		struct input_event& iev = readBuffer[i];

		switch (iev.type) {
			case EV_ABS :
			{
				/* 触摸屏事件 */
				CookTouchEvent(iev, cev);
				iFlagPressure = 0;
				break;
			}

			case EV_KEY :
			{
				/* 按键类事件 */
				CookKeyEvent(iev, cev);
				break;
			}

			case EV_SYN :
			{
				/* 同步事件 */
				if (iev.code == SYN_REPORT) {
					iFlagPressure++;
					if (iFlagPressure == 2) {
						cev->iPressure = 0;
					}
					/* 单点触摸屏的事件完整 */
					gettimeofday(&cev->tTime, NULL);
					cev++;					
					iRet++;
					if (iRet == iSize) {
						/* 已经得到了iSize个CookEvent */
						return iRet;
					}

				}
				break;
			}

			default :
			{
				printf("Not About Knowing : iev.type = %d \n", iev.type);
			}
		}
		
	}

	return iRet;
}


