#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

#define MILK_BUFFER_SIZE 9
#define CHEESE_BUFFER_SIZE 4

// Buffers
int milkBuffer[MILK_BUFFER_SIZE];
int cheeseBuffer[CHEESE_BUFFER_SIZE];

// Pointers and counters
int milkHead = 0, milkTail = 0, cheeseHead = 0, cheeseTail = 0;

// Semaphores and Mutexes
HANDLE milkFull, milkEmpty, cheeseFull, cheeseEmpty, milkMutex, cheeseMutex;

// Number of cheeseburgers to produce
int numCheeseburgers;

DWORD WINAPI milkProducer(LPVOID arg) {
    int producerID = *((int*)arg);
    int itemsToProduce = (2 * numCheeseburgers * 3) / 3;

    for (int i = 0; i < itemsToProduce; i++) {
        WaitForSingleObject(milkEmpty, INFINITE);  
        WaitForSingleObject(milkMutex, INFINITE);  

        milkBuffer[milkTail] = producerID;  
        milkTail = (milkTail + 1) % MILK_BUFFER_SIZE; 
        printf("Milk Producer %d produced a bottle of milk.\n", producerID);

        ReleaseSemaphore(milkMutex, 1, NULL);  
        ReleaseSemaphore(milkFull, 1, NULL);   

        Sleep(rand() % 500); 
    }

    return 0;
}

DWORD WINAPI cheeseProducer(LPVOID arg) {
    int producerID = *((int*)arg);
    int itemsToProduce = numCheeseburgers;

    for (int i = 0; i < itemsToProduce; i++) {
        // Wait for 3 milk bottles
        WaitForSingleObject(milkFull, INFINITE);
        WaitForSingleObject(milkFull, INFINITE);
        WaitForSingleObject(milkFull, INFINITE);
        WaitForSingleObject(milkMutex, INFINITE);  // Lock milk buffer

        // Consume milk
        int milk1 = milkBuffer[milkHead];
        milkHead = (milkHead + 1) % MILK_BUFFER_SIZE;
        int milk2 = milkBuffer[milkHead];
        milkHead = (milkHead + 1) % MILK_BUFFER_SIZE;
        int milk3 = milkBuffer[milkHead];
        milkHead = (milkHead + 1) % MILK_BUFFER_SIZE;

        ReleaseSemaphore(milkMutex, 1, NULL);  
        ReleaseSemaphore(milkEmpty, 3, NULL);  

        // Produce cheese
        WaitForSingleObject(cheeseEmpty, INFINITE);  
        WaitForSingleObject(cheeseMutex, INFINITE);  

        int cheeseID = milk1 * 1000 + milk2 * 100 + milk3 * 10 + producerID;
        cheeseBuffer[cheeseTail] = cheeseID;
        cheeseTail = (cheeseTail + 1) % CHEESE_BUFFER_SIZE;
        printf("Cheese Producer %d produced a slice of cheese: %d.\n", producerID, cheeseID);

        ReleaseSemaphore(cheeseMutex, 1, NULL); 
        ReleaseSemaphore(cheeseFull, 1, NULL);  

        Sleep(rand() % 500); 
    }

    return 0;
}

DWORD WINAPI cheeseburgerProducer(LPVOID arg) {
    for (int i = 0; i < numCheeseburgers; i++) {
        // Wait for 2 cheese slices
        WaitForSingleObject(cheeseFull, INFINITE);
        WaitForSingleObject(cheeseFull, INFINITE);
        WaitForSingleObject(cheeseMutex, INFINITE);  

        // Consume cheese
        int cheese1 = cheeseBuffer[cheeseHead];
        cheeseHead = (cheeseHead + 1) % CHEESE_BUFFER_SIZE;
        int cheese2 = cheeseBuffer[cheeseHead];
        cheeseHead = (cheeseHead + 1) % CHEESE_BUFFER_SIZE;

        ReleaseSemaphore(cheeseMutex, 1, NULL);  // Unlock cheese buffer
        ReleaseSemaphore(cheeseEmpty, 2, NULL);  // Signal empty slots

        // Produce cheeseburger
        printf("Cheeseburger %d produced: %d%d.\n", i + 1, cheese1, cheese2);

        Sleep(rand() % 500);  
    }

    return 0;
}

int main() {
    printf("How many cheeseburgers do you want? ");
    scanf("%d", &numCheeseburgers);

    // Initialize semaphores and mutexes
    milkFull = CreateSemaphore(NULL, 0, MILK_BUFFER_SIZE, NULL);
    milkEmpty = CreateSemaphore(NULL, MILK_BUFFER_SIZE, MILK_BUFFER_SIZE, NULL);
    cheeseFull = CreateSemaphore(NULL, 0, CHEESE_BUFFER_SIZE, NULL);
    cheeseEmpty = CreateSemaphore(NULL, CHEESE_BUFFER_SIZE, CHEESE_BUFFER_SIZE, NULL);
    milkMutex = CreateSemaphore(NULL, 1, 1, NULL);
    cheeseMutex = CreateSemaphore(NULL, 1, 1, NULL);

    HANDLE threads[6];
    int milkIDs[3] = {1, 2, 3};
    int cheeseIDs[2] = {4, 5};

    // Create threads
    for (int i = 0; i < 3; i++) {
        threads[i] = CreateThread(NULL, 0, milkProducer, &milkIDs[i], 0, NULL);
    }

    for (int i = 0; i < 2; i++) {
        threads[3 + i] = CreateThread(NULL, 0, cheeseProducer, &cheeseIDs[i], 0, NULL);
    }

    threads[5] = CreateThread(NULL, 0, cheeseburgerProducer, NULL, 0, NULL);

    // Wait for threads to complete
    WaitForMultipleObjects(6, threads, TRUE, INFINITE);

    // Clean up semaphores and mutexes
    for (int i = 0; i < 6; i++) {
        CloseHandle(threads[i]);
    }

    CloseHandle(milkFull);
    CloseHandle(milkEmpty);
    CloseHandle(cheeseFull);
    CloseHandle(cheeseEmpty);
    CloseHandle(milkMutex);
    CloseHandle(cheeseMutex);

    return 0;
}
