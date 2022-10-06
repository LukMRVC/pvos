#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <sys/time.h>

enum {
    TimePeriod = 100,
};

void alarm_interupt( int t_sig );
// initialize and start SIGALRM
void gt_sig_start( void )
{
    struct sigaction l_sig_act;
    memset( &l_sig_act, 0, sizeof( l_sig_act ) );
    l_sig_act.sa_flags = 0; // SA_RESTART
    l_sig_act.sa_handler = alarm_interupt;

    sigaction( SIGALRM, &l_sig_act, NULL );

    struct itimerval l_tv_alarm = { { 0, TimePeriod * 1000 }, { 0, TimePeriod * 1000 } };
    setitimer( ITIMER_REAL, &l_tv_alarm, NULL );
}

// deinitialize and stop SIGALRM
void gt_sig_stop( void )
{
    struct itimerval l_tv_alarm = { { 0, 0 }, { 0, 0 } };
    setitimer( ITIMER_REAL, & l_tv_alarm, NULL );

    struct sigaction l_sig_act;
    memset( &l_sig_act, 0, sizeof( l_sig_act ) );
    l_sig_act.sa_handler = SIG_DFL;

    sigaction( SIGALRM, &l_sig_act, NULL );
}



void alarm_interupt(int t_sig) {
    static int n = 1;
    char parts[] = {'-', '/', '|', '\\'};
    printf("%c \r", parts[n++ % 4]);
}

int main() {
    const int MAX_ROTATE_TIME = 10 * 5; // 5 seconds
    char parts[] = {'-', '/', '|', '\\'};
    int n = 1;
    while (n <= MAX_ROTATE_TIME) {
        printf("%c \r", parts[n++ % 4]);
        fflush(stdout);
        usleep(100000);
    }
    n = 0;
    printf("Now rotating without FFLUSH!\n");
    setvbuf(stdout, nullptr, _IONBF, 0);
    while (n <= MAX_ROTATE_TIME) {
        printf("%c \r", parts[n++ % 4]);
        usleep(100000);
    }
//    struct sigaction l_sa;
//    l_sa.sa_handler = alarm_iterrupt;
//    l_sa.sa_flags = 0; // SA_RESTART
//    sigemptyset(&l_sa.sa_mask);

//    sigaction(SIGALRM, &l_sa, nullptr);
    gt_sig_start();
    n = 0;
    printf("Now rotating using SIGALRM, press q to quit!\n");
    while (getchar() != 'q');
    gt_sig_stop();
    return 0;
}