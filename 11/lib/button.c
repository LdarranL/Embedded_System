#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <unistd.h> // for open/close
#include <fcntl.h> // for O_RDWR
#include <sys/ioctl.h> // for ioctl
#include <sys/msg.h>
#include <pthread.h>
#include "button.h"

// first read input device
#define INPUT_DEVICE_LIST "/dev/input/event" // 실제 디바이스 드라이버 노드파일: 뒤에 숫자가 붙음. ex)/dev/input/event5
#define PROBE_FILE "/proc/bus/input/devices" // PPT에 제시된 "이 파일을 까보면 event? 의 숫자를 알수 있다"는 바로 그 파일
#define HAVE_TO_FIND_1 "N: Name=\"ecubebutton\"\n"
#define HAVE_TO_FIND_2 "H: Handlers=kbd event"

int fd;
int msgID;
pthread_t buttonTh_id;
char buttonPath[200];

// 디바이스 경로 찾기 함수 (원형만, 구현은 필요시 추가)
int probeButtonPath(char* buttonPath);

void* buttonThFunc(void* arg)
{
    struct input_event ev;
    BUTTON_MSG_T msg;
    while(1)
    {
        int readSize = read(fd, &ev, sizeof(ev));
        if (readSize == sizeof(ev))
        {
            if (ev.type == EV_KEY)
            {
                msg.messageNum = 1;
                msg.keyInput = ev.code;
                msg.pressed = ev.value;
                msgsnd(msgID, &msg, sizeof(BUTTON_MSG_T)-sizeof(long), 0);
            }
        }
        else
        {
            usleep(10000); // 10ms 대기
        }
    }
    return NULL;
}

int buttonInit(void)
{
    if (probeButtonPath(buttonPath) == 0)
        return 0;
    fd = open(buttonPath, O_RDONLY);
    msgID = msgget(MESSAGE_ID, IPC_CREAT|0666);
    pthread_create(&buttonTh_id, NULL, buttonThFunc, NULL);
    return 1;
}

