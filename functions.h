#define BUFFER_SIZE 1024



void error(const char *msg);
void *controller_thread(void *arg);
void *worker_thread(void *arg);