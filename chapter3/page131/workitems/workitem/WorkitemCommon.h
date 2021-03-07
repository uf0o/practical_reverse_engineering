#pragma once

struct JunkData{
	int junk ;
};


#define WORKITEM_DEVICE 0x8000

#define IOCTL_WORKITEM  CTL_CODE(WORKITEM_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
