#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_LINE_LENGTH 100
int typeCount [3]={0};

int prodNum = 1;

typedef struct Node {
    char ** data ;
    struct Node* next;
} Node;


typedef struct BoundedQueue {
    int size;
    char ** data ;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
    int start;
    int end;

} BoundedQueue;

BoundedQueue* prodArr;
typedef struct Producer{
    int numOfProducts;
    int prodNum;
    int size;
    BoundedQueue bq;
}Producer;
typedef struct UnboundedQueue {
    Node* prev;
    Node* next;
} UnboundedQueue;

typedef struct CreationArg {
    int numOfProd;
    int numOfProducts;
    int size;
} CreationArg;


BoundedQueue createBoundedQueue(int size){
    BoundedQueue bq;
    bq.data= malloc(size * sizeof(char*));
    pthread_mutex_init(&bq.mutex,NULL);
    sem_init(&bq.empty, 0, size);
    sem_init(&bq.full, 0, 0);
    bq.start=size;
    bq.end=size;
    return bq;
}
void insertBoundedQueue(char * article, int prodNumber){
    BoundedQueue bq=prodArr[prodNumber];
    sem_wait(&bq.empty);
    pthread_mutex_lock(&bq.mutex);
    strcpy(bq.data[bq.start],article);
    pthread_mutex_unlock(&bq.mutex);
    sem_post(&bq.full);
    bq.start--;

}
void removeBoundedQueue(int prodNumber){
    BoundedQueue bq=prodArr[prodNumber];
    sem_wait(&bq.empty);
    pthread_mutex_lock(&bq.mutex);
    strcpy(bq.data[bq.end],NULL);
    pthread_mutex_unlock(&bq.mutex);
    sem_post(&bq.full);
    bq.end--;
}

void createArticles(void *arg){
    Producer* prod= (Producer*)arg;
    char * article;
    int i,type ;
    for (i=0;i<=prod->numOfProducts;i++){
        if(i==prod->numOfProducts){
            sprintf(article,"Done");
        }
        type= rand() % 3 + 1;
        if(type==1){
            sprintf(article,("producer %d %s %d\n",prod->prodNum,"SPORTS",typeCount[0]));
            typeCount[0]++;
        } else if (type==2){
            sprintf(article,("producer %d %s %d\n",prod->prodNum,"NEWS",typeCount[1]));
            typeCount[1]++;
        } else if (type==3){
            sprintf(article,("producer %d %s %d\n",prod->prodNum,"WEATHER",typeCount[2]));
            typeCount[2]++;
        }
    }
    char* articleToInsert= malloc(strlen(article)*sizeof (char));
    strcpy(articleToInsert,article);
    insertBoundedQueue(articleToInsert,prod->prodNum);
}


void createProducer(void *argPtr) {
    CreationArg* arg = (CreationArg*)argPtr;
    Producer prod;
    prod.prodNum=arg->numOfProd;
    prod.numOfProducts=arg->numOfProducts;
    prod.size=arg->size;
    prod.bq= createBoundedQueue(arg->size);
    prodArr[arg->numOfProd]=prod.bq;
    createArticles(&prod);

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        perror("configuration file is missing\n");
        return -1;
    }

    const char* file_name = argv[1];
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Failed to open the file\n");
        return -1;
    }

    char line[MAX_LINE_LENGTH], numOfProducts[MAX_LINE_LENGTH], qSize[MAX_LINE_LENGTH];
    int returnValue,  coEditorSize;
    prodArr= malloc(prodNum * sizeof(BoundedQueue));
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        if (fgets(numOfProducts, MAX_LINE_LENGTH, file) != NULL) {
            if (fgets(qSize, MAX_LINE_LENGTH, file) != NULL) {
                pthread_t threadId;
                CreationArg* arg = malloc(sizeof(CreationArg));
                if (arg == NULL) {
                    perror("Failed to allocate memory for CreationArg\n");
                    return 1;
                }
                if(prodNum!=1) {
                    prodArr = realloc(prodArr, 1 * sizeof(BoundedQueue));
                }
                arg->numOfProd = atoi(line);
                arg->numOfProducts = atoi(numOfProducts);
                arg->size = atoi(qSize);

                returnValue = pthread_create(&threadId, NULL, createProducer, (void*)arg);

                if (returnValue != 0) {
                    perror("Error creating thread\n");
                    return -1;
                }
                free(arg);
            }
        } else {
            coEditorSize = atoi(line);
            break;
        }

        prodNum++;

        if (fgets(line, MAX_LINE_LENGTH, file) == NULL) {
            perror("configuration file is not in the required format\n");
        }
    }
    int i;
    pthread_t threadId2;
    for(i=0;i<prodNum;i++){
   //     returnValue = pthread_create(&threadId2, NULL, createArticles,&prodArr[i]);

        if (returnValue != 0) {
            perror("Error creating thread\n");
            return -1;
        }
    }
    fclose(file);

    return 0;
}
