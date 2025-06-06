/* buzzer.c : Buzzer control library --------------------------------------- */
#include "buzzer.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

/* ---- 내부 상수 ---------------------------------------------------------- */
#define MAX_SCALE_STEP        8
#define BUZZER_BASE_SYS_PATH "/sys/bus/platform/devices/"
#define BUZZER_FILENAME       "peribuzzer"
#define BUZZER_ENABLE_NAME    "enable"
#define BUZZER_FREQ_NAME      "frequency"

static const int musicScale[MAX_SCALE_STEP] =
{
    262, 294, 330, 349, 392, 440, 494, 523
};

/* ---- 내부 전역 ----------------------------------------------------------- */
static char gBuzzerBaseSysDir[128] = {0};     /* “…/peribuzzer.xx/” */

/* ---- 내부 함수 ----------------------------------------------------------- */
static int findBuzzerSysPath(void)
{
    DIR *dir_info = opendir(BUZZER_BASE_SYS_PATH);
    if (dir_info == NULL)
        return -1;

    struct dirent *dir_entry;
    while ((dir_entry = readdir(dir_info)) != NULL)
    {
        if (strncmp(BUZZER_FILENAME,
                    dir_entry->d_name,
                    strlen(BUZZER_FILENAME)) == 0)
        {
            snprintf(gBuzzerBaseSysDir,
                     sizeof(gBuzzerBaseSysDir),
                     "%s%s/",
                     BUZZER_BASE_SYS_PATH,
                     dir_entry->d_name);
            closedir(dir_info);
            return 0;                /* 경로 확인 성공 */
        }
    }
    closedir(dir_info);
    return -1;                       /* 장치 찾지 못함 */
}

static int buzzerEnable(int enable)
{
    if (gBuzzerBaseSysDir[0] == '\0')
        return -1;                   /* 초기화 안 됨 */

    char path[200];
    snprintf(path, sizeof(path), "%s%s",
             gBuzzerBaseSysDir, BUZZER_ENABLE_NAME);

    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;

    const char *val = enable ? "1" : "0";
    int ret = write(fd, val, 1);
    close(fd);
    return (ret == 1) ? 0 : -1;
}

static int setFrequency(int frequency)
{
    if (gBuzzerBaseSysDir[0] == '\0')
        return -1;

    char path[200];
    snprintf(path, sizeof(path), "%s%s",
             gBuzzerBaseSysDir, BUZZER_FREQ_NAME);

    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;

    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%d", frequency);
    int ret = write(fd, buf, len);
    close(fd);
    return (ret == len) ? 0 : -1;
}

/* ---- 외부 API 구현 ------------------------------------------------------- */
int buzzerInit(void)
{
    if (findBuzzerSysPath() < 0)
        return -1;
    return buzzerEnable(0);          /* 시작 시 OFF */
}

int buzzerPlaySong(int scale)
{
    if (scale < 1 || scale > MAX_SCALE_STEP)
        return -1;

    if (setFrequency(musicScale[scale - 1]) < 0)
        return -1;
    return buzzerEnable(1);
}

int buzzerStopSong(void)
{
    return buzzerEnable(0);
}

int buzzerExit(void)
{
    /* 정리 단계에서도 버저를 꺼 둠 */
    return buzzerEnable(0);
}

