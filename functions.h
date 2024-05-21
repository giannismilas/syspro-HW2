#define BUFFER_SIZE 1024



typedef struct {
    int jobID;
    char job[BUFFER_SIZE];
    int clientSocket;
} Job;

void error(const char *msg);
void *controller_thread(void *arg);
void *worker_thread(void *arg);