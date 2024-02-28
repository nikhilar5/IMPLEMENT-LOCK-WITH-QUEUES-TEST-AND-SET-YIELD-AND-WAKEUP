#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct node {
    struct node* next;
    pthread_t thread;
} node_t;

typedef struct queue {
    node_t* head;
    node_t* tail;
} queue_t;

typedef struct lock {
    atomic_bool lock_flag;
    queue_t* wait_queue;
} lock_t;

void queue_init(queue_t* q) {
    q->head = NULL;
    q->tail = NULL;
}

void enqueue(queue_t* q, pthread_t thread) {
    node_t* new_node = malloc(sizeof(node_t));
    new_node->next = NULL;
    new_node->thread = thread;

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
}

pthread_t dequeue(queue_t* q) {
    if (q->head == NULL) {
        return (pthread_t)NULL;
    } else {
        pthread_t thread = q->head->thread;
        node_t* tmp = q->head;

        if (q->head == q->tail) {
            q->head = NULL;
            q->tail = NULL;
        } else {
            q->head = q->head->next;
        }

        free(tmp);
        return thread;
    }
}
void lock(lock_t* lock) {
    while (atomic_exchange(&lock->lock_flag, 1)) {
       
        sched_yield();
    }
}

void unlock(lock_t* lock) {
    pthread_t thread = dequeue(lock->wait_queue);

    if (thread != (pthread_t)NULL) {
               pthread_self();
    } else {
       
        atomic_store(&lock->lock_flag, 0);
    }
}

void* thread_func(void* arg) {
    lock_t* lk = (lock_t*)arg;

   
    lock(lk);
    printf("Thread %ld ACQUIRE lock\n", pthread_self());
    sleep(1);
   
    printf("Thread %ld is in critical section\n", pthread_self());
    sleep(1);
   
    unlock(lk);
    printf("Thread %ld RELEASE lock\n", pthread_self());
    sleep(2);

    return NULL;
}

int main() {
    pthread_t threads[5];
    lock_t lock;
    queue_t wait_queue;
    atomic_store(&lock.lock_flag, 0);
    lock.wait_queue = &wait_queue;
    queue_init(lock.wait_queue);

    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, thread_func, &lock);
    }
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
