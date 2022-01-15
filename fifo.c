#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "fifo.h"
#include "opensl_io.h"

static OPENSL_STREAM  *pOpenSL_stream;

int samplerate = 44100;
int inchannels = 1;
int outchannels = 2;
int chunk = 128;

int playback_fd = -1;
int capture_fd = -1;
static void* playbuf;
static void* recbuf;
char playstr[1024];


int loop = 1;

void closeaudio(int signum)
{
    android_CloseAudioDevice(pOpenSL_stream);
    printf("OpenSLES device closed");
    loop = 0;
}

ssize_t read_wholy(int fd, void* buf, size_t size)
{
    ssize_t readed = 0;
    while (readed < size)
    {
//	printf("read(%d, buf + readed, %d)\n", fd, size - readed);
        readed += read(fd, buf + readed, size - (size_t)readed);
//	printf("read %d\n", readed);
    }
//    snprintf(playstr, readed + 1, "%s", buf);
//    printf("%s\n", playstr);
    return readed;
}

ssize_t write_wholy(int fd, void* buf, size_t size)
{
    ssize_t writed = 0;
    while (writed < size)
        write(fd, buf + writed, size - (size_t)writed);
    return writed;
}

void* playback()
{
    int num = 0;
    if (playback_fd == -1) {
        playback_fd = open(PLAYBACK_DEVICE, O_RDONLY);
        if (playback_fd == -1)
            error (errno, errno, "Cannot open playback fifo");
    }
    printf("OpenSLES playback started\n");
    while(1) {

        int size = read_wholy(playback_fd, playbuf, chunk * sizeof(float) * outchannels);
        int ret = android_AudioOut(pOpenSL_stream, (float*) playbuf, chunk * outchannels);
        printf("Playing chunk %d samples %d size %d fd %d\n", num++, ret, size, playback_fd);
    }

    return NULL;
}

void* capture()
{
    int num = 0;
    if (capture_fd  == -1) {
        capture_fd = open(CAPTURE_DEVICE, O_WRONLY);
        if (capture_fd == -1)
            error (errno, errno, "Cannot open capture fifo");
    }
    printf("OpenSLES capture started\n");

    while(1) {
        int ret = android_AudioIn(pOpenSL_stream, recbuf, chunk * inchannels);
        int size =write_wholy(capture_fd, recbuf, chunk * sizeof(float) * inchannels);
        printf("Captiring chunk %d samples %d size %d\n", num++, ret, size);
    }

    return NULL;
}


int main(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "s:c:i:o:")) != -1) {
        switch (opt) {
           case 's':
               samplerate = atoi(optarg);
               break;
           case 'c':
               chunk = atoi(optarg);
               break;
           case 'i':
               inchannels = atoi(optarg);
               break;
           case 'o':
               outchannels = atoi(optarg);
               break;
           default: /* '?' */
               fprintf(stderr, "Usage: %s [-s samplerate] [-c chunk] [-i inchannels] [-o outchannels]\n",
                       argv[0]);
               exit(EXIT_FAILURE);
        }
    }

    if ((playbuf = malloc(chunk * sizeof(float) * outchannels)) == NULL)
        error(errno, errno, "Cannot malloc playbuf");
    if ((recbuf = malloc(chunk * sizeof(float) * inchannels)) == NULL)
        error(errno, errno, "Cannot malloc recbuf");

    pOpenSL_stream = android_OpenAudioDevice(samplerate, inchannels, outchannels, chunk);
    if (pOpenSL_stream == NULL)
       error(-1, errno, "Cannot open OpenSLES device");
    printf("OpenSLES hardware opened %d %d-in %d-out %d-chunk\n", samplerate,
           inchannels, outchannels, chunk);

    mkfifo(PLAYBACK_DEVICE, S_IRWXU|S_IRWXG|S_IRWXO);
    mkfifo(CAPTURE_DEVICE, S_IRWXU|S_IRWXG|S_IRWXO);

    pthread_t playback_id;
    pthread_t capture_id;
    int errnum;
    if ((errnum = pthread_create(&playback_id, NULL, playback, NULL)) != 0)
        error(0, errnum, "Cannot create playback thread");
    if ((errnum = pthread_create(&capture_id, NULL, capture, NULL)) != 0)
        error(0, errnum, "Cannot create capture thread");
    if ((errnum = pthread_join(playback_id, NULL)) != 0)
        error(0, errnum, "Cannot join playback thread");
    if ((errnum = pthread_join(capture_id, NULL)) != 0)
        error(0, errnum, "Cannot join capture thread");

/*    if (signal(SIGKILL, closeaudio) == SIG_ERR)
         error(0, errno, "Cannot catch SIGKILL");

    if (signal(SIGINT, closeaudio) == SIG_ERR)
         error(0, errno, "Cannot catch SIGINT");

    if (signal(SIGQUIT, closeaudio) == SIG_ERR)
         error(0, errno, "Cannot catch SIGQUIT");

    if (signal(SIGTERM, closeaudio) == SIG_ERR)
         error(0, errno, "Cannot catch SIGTERM");

    if (signal(SIGUSR1, closeaudio) == SIG_ERR)
         error(0, errno, "Cannot catch SIGUSR1"); */

    while(loop)
	sleep(1);

    return 0;
}

