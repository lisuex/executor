#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "utils.h"
#include "err.h"


#define MAX_N_TASKS 4096
#define MAX_LINE_LENGTH 511

// mutex for writing to task struct
pthread_mutex_t task_mutex;

// mutex 
pthread_mutex_t executor_running_mutex;

// mutex blocking changes of executor_running flag
pthread_mutex_t print_mutex;

// bool informing if the executor is doing command
bool executor_running;

// Information about task_ended information
struct InfoToPrint {
    int task_number;
    int status;
    int is_set;
};

// TaskEnded struct saving information about the tasks to print "task ended"
struct TaskEnded {
    int number;
    pthread_mutex_t task_ended_mutex;
    struct InfoToPrint to_print[MAX_N_TASKS];
};
struct TaskEnded task_ended;

struct Task {
    pid_t pid;
    pthread_t reading_output_thread;
    pthread_t reading_err_thread;
    pthread_t waiting_for_signal_thread;
    int task_number;
    pthread_mutex_t mutex_read;
    char last_line_out[MAX_LINE_LENGTH];
    char last_line_err[MAX_LINE_LENGTH];
    int pipe_out;
    int pipe_err;
};

// global array of tasks
struct Task tasks[MAX_N_TASKS];

// function reading STDOUT
void* read_run_output(void* data) {
    int* task_number = data;

    ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
    int pipe_out = tasks[*task_number].pipe_out;
    FILE* run_output = fdopen(pipe_out, "r");
    ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));
    char line[MAX_LINE_LENGTH];

    while(fgets(line, MAX_LINE_LENGTH, run_output)) {
        if(strlen(line) != 0) {
            // saving last-printed line into task struct       
            if(line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = '\0';
            }
            ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
            ASSERT_ZERO(pthread_mutex_lock(&tasks[*task_number].mutex_read));
            strcpy(tasks[*task_number].last_line_out, line);
            ASSERT_ZERO(pthread_mutex_unlock(&tasks[*task_number].mutex_read));
            ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));
        }
    }
    ASSERT_SYS_OK(fclose(run_output));

    return 0;
}

// function reading STDERR
void* read_run_err(void* data) {
    int* task_number = data;

    ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
    int pipe_err = tasks[*task_number].pipe_err;
    FILE* run_err = fdopen(pipe_err, "r");
    ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));
    char line[MAX_LINE_LENGTH];

    while(fgets(line, MAX_LINE_LENGTH, run_err)) {
        if(strlen(line) != 0) {
            // saving last-printed line into task struct 
            if(line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = '\0';
            }
            ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
            ASSERT_ZERO(pthread_mutex_lock(&tasks[*task_number].mutex_read));
            strcpy(tasks[*task_number].last_line_err, line);
            ASSERT_ZERO(pthread_mutex_unlock(&tasks[*task_number].mutex_read));
            ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));
        }
    }
    ASSERT_SYS_OK(fclose(run_err));

    return 0;
}

// function waiting until the task is finished
void* wait_for_signal(void* data) {
    int* task_number = data;
    int status;

    ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
    pid_t task_pid = tasks[*task_number].pid;
    ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));
    waitpid(task_pid, &status, 0);
    ASSERT_ZERO(pthread_mutex_lock(&executor_running_mutex));

    ASSERT_ZERO(pthread_join(tasks[*task_number].reading_err_thread, NULL));
    ASSERT_ZERO(pthread_join(tasks[*task_number].reading_output_thread, NULL));

    // if executor is running command it should add task_ended info to the global struct
    if(executor_running) {
        if(WIFSIGNALED(status)) {
            ASSERT_ZERO(pthread_mutex_lock(&task_ended.task_ended_mutex));
            task_ended.to_print[task_ended.number].task_number = *task_number;
            task_ended.to_print[task_ended.number].is_set = 0;
            task_ended.number++;
            ASSERT_ZERO(pthread_mutex_unlock(&task_ended.task_ended_mutex));
        }
        else if(WIFEXITED(status)) {
            ASSERT_ZERO(pthread_mutex_lock(&task_ended.task_ended_mutex));
            task_ended.to_print[task_ended.number].task_number = *task_number;
            task_ended.to_print[task_ended.number].is_set = 1;
            task_ended.to_print[task_ended.number].status = WEXITSTATUS(status);
            task_ended.number++;
            ASSERT_ZERO(pthread_mutex_unlock(&task_ended.task_ended_mutex));
        }
        else {
            ASSERT_ZERO(pthread_mutex_unlock(&executor_running_mutex));
            exit(1);
        }
    }
    // if executor is not running command it should immediately print task_ended info
    else {
        ASSERT_ZERO(pthread_mutex_lock(&print_mutex));
        if(WIFSIGNALED(status)) {
            printf("Task %d ended: signalled.\n", *task_number);
        }
        else if(WIFEXITED(status)) {
            printf("Task %d ended: status %d.\n", *task_number, WEXITSTATUS(status));
        }
        else {
            ASSERT_ZERO(pthread_mutex_unlock(&print_mutex));
            ASSERT_ZERO(pthread_mutex_unlock(&executor_running_mutex));
            exit(1);
        }
        ASSERT_ZERO(pthread_mutex_unlock(&print_mutex));
    }
    ASSERT_ZERO(pthread_mutex_unlock(&executor_running_mutex));
    return 0;
}

// function that prints task_ended info after finishing command in executor
int print_task_ended() {
    ASSERT_ZERO(pthread_mutex_lock(&task_ended.task_ended_mutex));
    for(int i = 0; i < task_ended.number; i++) {
        if(task_ended.to_print[i].is_set == 0) {
            printf("Task %d ended: signalled.\n", task_ended.to_print[i].task_number);
        }
        else {
            printf("Task %d ended: status %d.\n", task_ended.to_print[i].task_number, task_ended.to_print[i].status);
        }
        task_ended.to_print[i].is_set = 0;
        task_ended.to_print[i].task_number = 0;
        task_ended.to_print[i].status = 0;
    }
    task_ended.number = 0;
    ASSERT_ZERO(pthread_mutex_unlock(&task_ended.task_ended_mutex));
    return 0;
}

int main(int argc, char *argv[]) {

    assert(argv[argc] == NULL);
    int act_task_number = 0;
	pthread_mutex_init(&task_mutex, NULL);
	pthread_mutex_init(&task_ended.task_ended_mutex, NULL);
    pthread_mutex_init(&executor_running_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);
    executor_running = false;

    while(!feof(stdin)) {

        char line[MAX_LINE_LENGTH];
        read_line(line, MAX_LINE_LENGTH, stdin);

        char ** split_line = split_string(line);
        char * first_word = split_line[0];
        
        ASSERT_ZERO(pthread_mutex_lock(&executor_running_mutex));
        executor_running = true;
        ASSERT_ZERO(pthread_mutex_unlock(&executor_running_mutex));

        // run
        if(first_word[0] == 'r') {
            tasks[act_task_number].task_number = act_task_number;
            tasks[act_task_number].last_line_out[0] = '\0';
            tasks[act_task_number].last_line_err[0] = '\0';
            pthread_mutex_init(&tasks[act_task_number].mutex_read, NULL);

            int pipe_dsc_out[2];
            int pipe_dsc_err[2];
            ASSERT_SYS_OK(pipe(pipe_dsc_out));
            ASSERT_SYS_OK(pipe(pipe_dsc_err));

            set_close_on_exec(pipe_dsc_out[0], true);
            set_close_on_exec(pipe_dsc_err[0], true);
            
            pid_t pid = fork();
            ASSERT_SYS_OK(pid);

            if (!pid) { // child
                tasks[act_task_number].pid = getpid();

                // replacing STDERR
                ASSERT_SYS_OK(close(pipe_dsc_err[0]));                
                ASSERT_SYS_OK(dup2(pipe_dsc_err[1], STDERR_FILENO));
                ASSERT_SYS_OK(close(pipe_dsc_err[1]));
                
                // replacing STDOUT
                ASSERT_SYS_OK(close(pipe_dsc_out[0]));                
                ASSERT_SYS_OK(dup2(pipe_dsc_out[1], STDOUT_FILENO));
                ASSERT_SYS_OK(close(pipe_dsc_out[1]));
                
                // run the task
                ASSERT_SYS_OK(execvp(split_line[1], &split_line[1]));              
            } else { // parent
                ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
                tasks[act_task_number].pid = pid;

                ASSERT_SYS_OK(close(pipe_dsc_err[1]));
                tasks[act_task_number].pipe_err = pipe_dsc_err[0];
                ASSERT_ZERO(pthread_create(&tasks[act_task_number].reading_err_thread, NULL, read_run_err, &tasks[act_task_number].task_number));

                ASSERT_SYS_OK(close(pipe_dsc_out[1]));
                tasks[act_task_number].pipe_out = pipe_dsc_out[0];
                ASSERT_ZERO(pthread_create(&tasks[act_task_number].reading_output_thread, NULL, read_run_output, &tasks[act_task_number].task_number));

                ASSERT_ZERO(pthread_create(&tasks[act_task_number].waiting_for_signal_thread, NULL, wait_for_signal, &tasks[act_task_number].task_number));
                ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

                printf("Task %d started: pid %i.\n",act_task_number, pid);
            }
            act_task_number++;
        }
        // out
        else if(first_word[0] == 'o') {
            int task_number = atoi(split_line[1]);

            ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
            ASSERT_ZERO(pthread_mutex_lock(&tasks[task_number].mutex_read));
            char* line_to_print = tasks[task_number].last_line_out;
            ASSERT_ZERO(pthread_mutex_unlock(&tasks[task_number].mutex_read));
            ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

            printf("Task %d stdout: '%s'.\n",task_number, line_to_print);
        }
        // err
        else if(first_word[0] == 'e') {
            int task_number = atoi(split_line[1]);

            ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
            ASSERT_ZERO(pthread_mutex_lock(&tasks[task_number].mutex_read));
            char* line_to_print = tasks[task_number].last_line_err;
            ASSERT_ZERO(pthread_mutex_unlock(&tasks[task_number].mutex_read));
            ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

            printf("Task %d stderr: '%s'.\n",task_number, line_to_print);
        }
        // kill
        else if(first_word[0] == 'k') {
            ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
            int task_number = atoi(split_line[1]);
            pid_t pid = tasks[task_number].pid;
            pthread_mutex_t mutex_read = tasks[task_number].mutex_read;
            ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

            kill(pid, SIGINT);
            ASSERT_ZERO(pthread_mutex_destroy(&mutex_read));
        }
        // sleep
        else if(first_word[0] == 's') {
            int time = atoi(split_line[1]) * 1000;
            usleep(time);
        }
        // quit
        else if(first_word[0] == 'q' || first_word[0] == EOF) {
            for(int i = 0; i < act_task_number; i++) {
                ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
                pid_t pid = tasks[i].pid;
                pthread_t signal = tasks[i].waiting_for_signal_thread;
                pthread_mutex_t mutex_read = tasks[i].mutex_read;
                ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

                kill(pid, SIGKILL);
            
                ASSERT_ZERO(pthread_join(signal, NULL));

                ASSERT_ZERO(pthread_mutex_destroy(&mutex_read));
            }
            print_task_ended();
            free_split_string(split_line);
            ASSERT_ZERO(pthread_mutex_destroy(&task_ended.task_ended_mutex));
            ASSERT_ZERO(pthread_mutex_destroy(&print_mutex));
            ASSERT_ZERO(pthread_mutex_destroy(&executor_running_mutex));
            return 0;
        }
        free_split_string(split_line);
        // print task_ended info about tasks that ended while executor was processing last command
        print_task_ended();

        ASSERT_ZERO(pthread_mutex_lock(&executor_running_mutex));
        executor_running = false;
        ASSERT_ZERO(pthread_mutex_unlock(&executor_running_mutex));
    }
    // EOF
    for(int i = 0; i < act_task_number; i++) {
        ASSERT_ZERO(pthread_mutex_lock(&task_mutex));
        pid_t pid = tasks[i].pid;
        pthread_t signal = tasks[i].waiting_for_signal_thread;
        pthread_mutex_t mutex_read = tasks[i].mutex_read;
        ASSERT_ZERO(pthread_mutex_unlock(&task_mutex));

        kill(pid, SIGKILL);
    
        ASSERT_ZERO(pthread_join(signal, NULL));
        
        ASSERT_ZERO(pthread_mutex_destroy(&mutex_read));
    }
    print_task_ended();
    ASSERT_ZERO(pthread_mutex_destroy(&task_ended.task_ended_mutex));
    ASSERT_ZERO(pthread_mutex_destroy(&print_mutex));
    ASSERT_ZERO(pthread_mutex_destroy(&executor_running_mutex));
    return 0;
}