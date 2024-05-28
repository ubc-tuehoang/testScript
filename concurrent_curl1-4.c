
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT_SIZE 100
#define EVAL_COUNT 1000  

typedef struct {
    long thread_id;
    char model[MAX_INPUT_SIZE];
    char prompt[MAX_INPUT_SIZE];
    double eval_rate;
    struct timespec completion_time;
} thread_data_t;

void *send_curl_request(void *threadarg) {
    thread_data_t *my_data;
    my_data = (thread_data_t *)threadarg;
    long tid = my_data->thread_id;
    char command[400];
    struct timespec start_time, end_time;
    char start_time_str[26], end_time_str[26];
    double duration;


    clock_gettime(CLOCK_REALTIME, &start_time);
    strftime(start_time_str, 26, "%Y-%m-%d %H:%M:%S", localtime(&start_time.tv_sec));
    printf("Thread #%ld call started at: %s\n", tid, start_time_str);


    snprintf(command, sizeof(command), "curl http://localhost:11434/api/generate -d '{\"model\": \"%s\", \"prompt\": \"%s\", \"stream\": false}'", my_data->model, my_data->prompt);
    printf("Thread #%ld executing command: %s\n", tid, command);
    system(command);


    clock_gettime(CLOCK_REALTIME, &end_time);
    strftime(end_time_str, 26, "%Y-%m-%d %H:%M:%S", localtime(&end_time.tv_sec));
    printf("Thread #%ld call completed at: %s\n", tid, end_time_str);


    duration = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    my_data->eval_rate = EVAL_COUNT / duration;
    my_data->completion_time = end_time;
    printf("Thread #%ld Eval Rate: %f tokens/s\n", tid, my_data->eval_rate);

    pthread_exit(NULL);
}


int compare_completion_time(const void *a, const void *b) {
    thread_data_t *thread_a = (thread_data_t *)a;
    thread_data_t *thread_b = (thread_data_t *)b;

    if (thread_a->completion_time.tv_sec < thread_b->completion_time.tv_sec) {
        return -1;
    } else if (thread_a->completion_time.tv_sec > thread_b->completion_time.tv_sec) {
        return 1;
    } else {
        if (thread_a->completion_time.tv_nsec < thread_b->completion_time.tv_nsec) {
            return -1;
        } else if (thread_a->completion_time.tv_nsec > thread_b->completion_time.tv_nsec) {
            return 1;
        } else {
            return 0;
        }
    }
}

int main() {
    int num_threads;
    char model[MAX_INPUT_SIZE];
    char prompt[MAX_INPUT_SIZE];


    printf("Enter the model: ");
    fgets(model, MAX_INPUT_SIZE, stdin);
    model[strcspn(model, "\n")] = '\0';  

    printf("Enter the prompt: ");
    fgets(prompt, MAX_INPUT_SIZE, stdin);
    prompt[strcspn(prompt, "\n")] = '\0';  

    printf("Enter the number of threads: ");
    scanf("%d", &num_threads);

    pthread_t threads[num_threads];
    thread_data_t thread_data_array[num_threads];
    int rc;
    long t;

    for (t = 0; t < num_threads; t++) {
        thread_data_array[t].thread_id = t;
        strncpy(thread_data_array[t].model, model, MAX_INPUT_SIZE);
        strncpy(thread_data_array[t].prompt, prompt, MAX_INPUT_SIZE);

        rc = pthread_create(&threads[t], NULL, send_curl_request, (void *)&thread_data_array[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }


    qsort(thread_data_array, num_threads, sizeof(thread_data_t), compare_completion_time);


    printf("\nThreads sorted by completion time:\n");
    for (t = 0; t < num_threads; t++) {
        printf("Thread #%ld completed at %ld.%09ld with Eval Rate: %f tokens/s\n",
               thread_data_array[t].thread_id,
               thread_data_array[t].completion_time.tv_sec,
               thread_data_array[t].completion_time.tv_nsec,
               thread_data_array[t].eval_rate);
    }

    pthread_exit(NULL);
}

