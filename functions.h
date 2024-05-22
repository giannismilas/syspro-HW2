#define BUFFER_SIZE 1024



typedef struct {
    int jobID;
    char job[BUFFER_SIZE];
    int clientSocket;
} Job;

void error(const char *msg);