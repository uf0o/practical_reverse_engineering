#define SYSTHREADIOCTL_TYPE 0xa000
#define SYSTHREADIOCTL_PROCESS 0x900

#define IOCTL_SYSTHREAD_METHOD_BUFFERED CTL_CODE(SYSTHREADIOCTL_TYPE, SYSTHREADIOCTL_PROCESS, METHOD_BUFFERED,FILE_ANY_ACCESS)