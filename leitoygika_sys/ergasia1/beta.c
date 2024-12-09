#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>


#define SHM_SIZE 1024

int messent = 0;
int mesrecieved = 0;
int chunkssent = 0;



typedef struct {
    char messageFromProducer[SHM_SIZE];
    char messageFromConsumer[SHM_SIZE];
    sem_t semaphore;
} SharedStruct;

void* terminalWrite(void* args);
void* terminalRead(void* args);

int main() {
    pthread_t writerThread, readerThread;
    const char *shm_name = "/my_shm";
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    SharedStruct*shared_memory = mmap(0, sizeof(SharedStruct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    pthread_create(&writerThread, NULL, terminalWrite, (void*) shared_memory);
    pthread_create(&readerThread, NULL, terminalRead, (void*) shared_memory);

    // Wait for threads to finish
    pthread_join(writerThread, NULL);
    pthread_join(readerThread, NULL);
    printf("Messages sent:%d\n",messent);
    printf("Messages recieved:%d\n",mesrecieved);
    printf("Chunks sent:%d\n",chunkssent-1); // I dont count #BYE#
    printf("Chunks per messages:%f\n",(float) (chunkssent - 1)/messent); 

    munmap(shared_memory, sizeof(SharedStruct));
    close(shm_fd);
    return 0;
}

void* terminalWrite(void* args) {
    SharedStruct *shared_memory = args;
    char inputBuffer[SHM_SIZE];
    
    // Write loop
    while (1) {
        if (fgets(inputBuffer, SHM_SIZE, stdin) == NULL) {
            continue; // Handle fgets error
        }

        int inputLen = strlen(inputBuffer);
        int offset = 0;
        

        while (offset < inputLen) {
            // Prepare chunk
            char chunk[16]; // 15 characters + null terminator
            int chunkLength = (inputLen - offset > 15) ? 15 : inputLen - offset;
            strncpy(chunk, &inputBuffer[offset], chunkLength);
            chunk[chunkLength] = '\0'; // Ensure null termination

            

            // Write chunk to shared memory
            sem_wait(&shared_memory->semaphore);
            strncpy(shared_memory->messageFromConsumer, chunk, 16);
            sem_post(&shared_memory->semaphore);

            offset += chunkLength;
            chunkssent++;
            sleep(1);
            

        
        }
        
        // Check for exit condition
        if (strcmp(inputBuffer, "#BYE#\n") == 0) {
            break;
        }
        messent++;
    }

    return NULL;
}

void* terminalRead(void* args) {
    SharedStruct *shared_memory = args;
    char fullMessage[SHM_SIZE] = {0} ; // Buffer to assemble the full message

    // Read loop
    while (1) {
        sem_wait(&shared_memory->semaphore);

        // Assemble message from chunks
        if(strlen(shared_memory->messageFromProducer) > 0){
            strcat(fullMessage, shared_memory->messageFromProducer);
            int isLastChunk = (strlen(shared_memory->messageFromProducer) < 15);
            memset(shared_memory->messageFromProducer, 0, SHM_SIZE); // Clear buffer after reading

            sem_post(&shared_memory->semaphore);

            if (isLastChunk) {
                // Process the full message
                if (strcmp(fullMessage, "#BYE#\n") == 0) {
                    break;
                }
                printf("Received full message: %s", fullMessage);
                mesrecieved++;


                // Reset buffer for next message
                memset(fullMessage, 0, SHM_SIZE);
            }
        }else{
            sem_post(&shared_memory->semaphore);
            
        }    
    }

    return NULL;
}