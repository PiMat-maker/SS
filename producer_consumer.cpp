#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv_consumer = PTHREAD_COND_INITIALIZER;

int value = 0;
int sleep_time = 0;
bool global_debug_flag = false;
bool ready = false;
std::atomic_bool finish = false;

int get_tid() {
  static pthread_mutex_t local_mutex = PTHREAD_MUTEX_INITIALIZER;
  thread_local std::unique_ptr<int> TLS_val = std::make_unique<int>(0);
  thread_local bool is_init = false;
  static int iter = 1;

  if (!is_init) {
    is_init = true;
    pthread_mutex_lock(&local_mutex);
    *TLS_val = iter++;
    pthread_mutex_unlock(&local_mutex);
  }
  return *TLS_val;
}

void* producer_routine(void* arg) {
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  (void)arg;
  while (std::cin >> value) {
    pthread_mutex_lock(&mutex);
    ready = true;
    pthread_cond_signal(&cv_consumer);

    while (ready) {
      pthread_cond_wait(&cv_producer, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }

  finish = true;
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&cv_consumer);
  pthread_mutex_unlock(&mutex);
  return nullptr;
}
void* consumer_routine(void* arg) {
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  (void)arg;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  thread_local long long sum = 0;

  while (!finish) {
    pthread_mutex_lock(&mutex);
    while (!ready && !finish) {
      pthread_cond_wait(&cv_consumer, &mutex);
    }

    if (ready) {
      int local = value;
      ready = false;
      if (global_debug_flag)
        std::cout << "Tid, sum: " << get_tid() << " " << sum;
      pthread_cond_signal(&cv_producer);
      pthread_mutex_unlock(&mutex);
      sum += local;
    } else {
      pthread_cond_signal(&cv_producer);
      pthread_mutex_unlock(&mutex);
    }
    usleep(sleep_time);
  }
  return &sum;
}

void* consumer_interruptor_routine(void* arg) {
  // interrupt random consumer while producer is running
  auto thread_buffer = *(std::vector<pthread_t>*)arg;
  while (!finish.load())
    pthread_cancel(thread_buffer[std::rand() % thread_buffer.size()]);
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads(int num_threads, int sleep_limit_time, bool debug_flag) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  int number_of_threads = num_threads > 0 ? num_threads : 3;
  sleep_time = sleep_limit_time > 0 ? sleep_limit_time : 500;
  global_debug_flag = debug_flag;

  pthread_t producer_tid, interaptor_tid;
  pthread_create(&producer_tid, NULL, producer_routine, NULL);

  std::vector<pthread_t> consumer_tids(number_of_threads);
  for (int i = 0; i < number_of_threads; ++i) {
    pthread_create(&consumer_tids[i], NULL, consumer_routine, NULL);
  }

  pthread_create(&interaptor_tid, nullptr, consumer_interruptor_routine,
                 (void*)&consumer_tids);

  pthread_join(producer_tid, NULL);
  pthread_join(interaptor_tid, NULL);

  int sum = 0;
  for (int i = 0; i < number_of_threads; ++i) {
    long long* thread_sum = NULL;
    pthread_join(consumer_tids[i], (void**)&thread_sum);

    if (thread_sum != nullptr) {
      sum += *(long long*)thread_sum;
    }
  }

  return sum;
}
