/**
 * @file simple_threads.c
 * @brief Basic example of using the lwthread library
 */

 #include <lwthread/lwthread.h>
 #include <stdio.h>
 #include <stdlib.h>
 
 /* Thread function that counts */
 void counter_thread(void* arg) {
     int id = *(int*)arg;
     
     printf("Thread %d starting\n", id);
     
     for (int i = 0; i < 5; i++) {
         printf("Thread %d: Count %d\n", id, i);
         
         /* Sleep a bit to simulate work */
         lwt_sleep(100 * id);
         
         /* Yield to let other threads run */
         lwt_yield();
     }
     
     printf("Thread %d finished\n", id);
 }
 
 int main() {
     printf("Lightweight Threads Example\n");
     
     /* Create a scheduler with 2 worker threads */
     lwt_scheduler_t* scheduler = lwt_scheduler_create(2);
     if (!scheduler) {
         perror("Failed to create scheduler");
         return 1;
     }
     
     /* Start the scheduler */
     lwt_scheduler_start(scheduler);
     printf("Scheduler started with 2 worker threads\n");
     
     /* Create 5 threads */
     lwt_thread_t* threads[5];
     int ids[5];
     
     for (int i = 0; i < 5; i++) {
         ids[i] = i + 1;
         threads[i] = lwt_create(scheduler, counter_thread, &ids[i]);
         if (!threads[i]) {
             perror("Failed to create thread");
             continue;
         }
         printf("Created thread %d\n", ids[i]);
     }
     
     /* Wait for all threads to complete */
     for (int i = 0; i < 5; i++) {
         if (threads[i]) {
             printf("Waiting for thread %d\n", ids[i]);
             lwt_join(threads[i]);
             printf("Thread %d joined\n", ids[i]);
             
             /* Free thread memory */
             free(threads[i]);
         }
     }
     
     /* Shutdown scheduler */
     printf("All threads completed, shutting down\n");
     lwt_scheduler_stop(scheduler);
     lwt_scheduler_destroy(scheduler);
     
     return 0;
 }