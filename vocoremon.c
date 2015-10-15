/* note: this little tool is used to detours the issue of openwrt in RT5350
 * that ap+sta can not show ap during sta can not connect to its host.
 * once we find such issue happens, use backup wireless config replace
 * current one from user.
 * if everything is normal, just quit itself quietly.
 * better to make it run automaticly every start up.
 *
 * author: Qin Wei(support@vocore.io)
 * date:   20150915
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFSIZE     0x100

// there are two modes, one is normal mode, ap+sta works well.
// one is recover mode, if last time failed to go into ap+sta mode, I will
// set VoCore into recover mode, back into only ap mode. Once sta is recovered,
// I will put VoCre to normal mode, ap+sta could work same time.

// check if we are in recover mode.
int vocoremon_is_recover_mode()
{
    FILE *fp;

    // in recover mode, backup wireless config file must exist.
    fp = fopen("/etc/config/wireless.user", "rb");
    if(fp == NULL)
        return 0;

    fclose(fp);
    return 1;
}

int vocoremon_is_normal_mode()
{
    return !vocoremon_is_recover_mode();
}

// check if user setup sta mode in config.
int vocoremon_check_sta_mode()
{
    FILE *pp;

    pp = popen("grep -i sta /etc/config/wireless", "r");
    if(pp == NULL)
        return -1;

    while(!feof(pp)) {
        char buf[BUFSIZE] = {0};
        if(fgets(buf, BUFSIZE, pp) == NULL)
            break;
        if(strchr(buf, '#'))
            continue;       // skip the commented line.

        pclose(pp);
        return 1;
    }

    pclose(pp);
    return 0;
}

char *vocoremon_get_default_gateway()
{
    static char gw[0x20];
    FILE *pp;

    pp = popen("route", "r");
    if(pp == NULL)
        return NULL;

    while(!feof(pp)) {
        char buf[BUFSIZE] = {0};
        int pos = sizeof("default") - 1, end;
        if(fgets(buf, BUFSIZE, pp) == NULL)
            break;
        if(memcmp(buf, "default", pos))
            continue;       // find default line.

        while(*(buf + pos) == ' ')
            pos++;
        end = pos;
        while(*(buf + end) != ' ')
            end++;
        memcpy(gw, buf + pos, end - pos);
        gw[end - pos] = 0;

        pclose(pp);
        return gw;
    }

    pclose(pp);

    return NULL;
}

// check if current sta mode is accessable.
int vocoremon_check_sta_accessable()
{
    FILE *pp;
    char cmd[BUFSIZE], *def;

    def = vocoremon_get_default_gateway();
    if(def == NULL)
        return 0;
    printf("default gateway: %s\n", def);

    sprintf(cmd, "ping -c1 -w1 %s", def);
    pp = popen(cmd, "r");
    if(pp == NULL)
        return -1;

    while(!feof(pp)) {
        char buf[BUFSIZE] = {0};
        if(fgets(buf, BUFSIZE, pp) == NULL)
            break;
        if(NULL == strstr(buf, ", 0%"))
            continue;       // 0% packet loss means it is accessable.

        pclose(pp);
        return 1;
    }

    // if ping failed, we will get this result around 1 seconds(-w1).
    pclose(pp);
    return 0;
}

void vocoremon_create_default_config()
{
    FILE *fp;
    char buf[] = "config wifi-device  radio0\n"
                 "\toption type     mac80211\n"
                 "\toption channel  11\n"
                 "\toption hwmode	11g\n"
                 "\toption path	\'10180000.wmac\'\n"
                 "\toption htmode	HT20\n"
                 "\n"
                 "config wifi-iface\n"
                 "\toption device   radio0\n"
                 "\toption network  lan\n"
                 "\toption mode     ap\n"
                 "\toption ssid     VoCore\n"
                 "\toption encryption none\n\n";

    fp = fopen("/etc/config/wireless", "wb");
    if(fp == NULL)
        return;

    fwrite(buf, sizeof(buf) - 1, 1, fp);
    fclose(fp);
}

void vocoremon_restart_wireless()
{
    execl("/sbin/wifi", "wifi");
}

int main(int argc, char *argv[])
{
    int wait = 0;

    if(argc > 2) {
        printf("vocoremon [wait]: put it in rc.local to make it run automaticly.\n");
        return -1;
    }

    if(argc == 2) {
        wait = atoi(argv[1]);
        printf("vocoremon will check your wifi status in %d seconds...\n", wait);
        sleep(wait);
    } else {
        printf("vocoremon is checking your wifi status...\n");
    }


    if(vocoremon_is_recover_mode()) {
        printf("recovering backup wireless config...\n");
        rename("/etc/config/wireless.user", "/etc/config/wireless");
        vocoremon_restart_wireless();
    }

    switch(vocoremon_check_sta_mode()) {
    case -1:
        printf("ERROR: failed to check sta mode.\n");
        // IMPORTANT: depends on wifi/ping/route/grep.
        break;

    case 0:     // not setting sta, just ignore.
        return 1;

    case 1:     // setting sta, we should check the network.
        switch(vocoremon_check_sta_accessable()) {
        case -1:
            printf("ERROR: failed to check sta accessable.\n");
            break;

        case 0: // network is not accessable, we should use default config.
            printf("network is not accessable, using default...\n");
            if(rename("/etc/config/wireless", "/etc/config/wireless.user") < 0) {
                printf("ERROR: failed to recover wireless setting.\n");
                return 0;
            }
            vocoremon_create_default_config();
            vocoremon_restart_wireless();

            printf("your wireless config changed to default now.\n");
            break;

        case 1:
            printf("wireless works normal, quit.\n");
            return 2;

        default:
            return 0;
        }

    default:
        return 0;
    }

    return 0;
}

