#include "harness/unity.h"
#include "../src/lab.h"

// NOTE: Due to the multi-threaded nature of this project. Unit testing for this
// project is limited. I have provided you with a command line tester in
// the file app/main.cp. Be aware that the examples below do not test the
// multi-threaded nature of the queue. You will need to use the command line
// tester to test the multi-threaded nature of your queue. Passing these tests
// does not mean your queue is correct. It just means that it can add and remove
// elements from the queue below the blocking threshold.


void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}




void test_create_destroy(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    queue_destroy(q);
}

void test_queue_dequeue(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    enqueue(q, &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    queue_destroy(q);
}

void test_queue_dequeue_multiple(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    int data2 = 2;
    int data3 = 3;
    enqueue(q, &data);
    enqueue(q, &data2);
    enqueue(q, &data3);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data2);
    TEST_ASSERT_TRUE(dequeue(q) == &data3);
    queue_destroy(q);
}

void test_queue_dequeue_shutdown(void)
{
    queue_t q = queue_init(10);
    TEST_ASSERT_TRUE(q != NULL);
    int data = 1;
    int data2 = 2;
    int data3 = 3;
    enqueue(q, &data);
    enqueue(q, &data2);
    enqueue(q, &data3);
    TEST_ASSERT_TRUE(dequeue(q) == &data);
    TEST_ASSERT_TRUE(dequeue(q) == &data2);
    queue_shutdown(q);
    TEST_ASSERT_TRUE(dequeue(q) == &data3);
    TEST_ASSERT_TRUE(is_shutdown(q));
    TEST_ASSERT_TRUE(is_empty(q));
    queue_destroy(q);
}

/**
 * @brief Ensure dequeue returns NULL immediately if shutdown is called and queue is empty.
 */
void test_dequeue_empty_after_shutdown(void) {
  queue_t q = queue_init(5);
  queue_shutdown(q);
  TEST_ASSERT_NULL(dequeue(q));
  queue_destroy(q);
}

/**
* @brief Ensure enqueue after shutdown does not add items to the queue.
*/
void test_enqueue_after_shutdown(void) {
  queue_t q = queue_init(5);
  int data = 42;
  queue_shutdown(q);
  enqueue(q, &data);
  TEST_ASSERT_TRUE(is_empty(q));
  queue_destroy(q);
}

/**
* @brief Test the queue properly handles being filled to capacity.
*/
void test_fill_queue_to_capacity(void) {
  queue_t q = queue_init(3);
  int a = 1, b = 2, c = 3;
  enqueue(q, &a);
  enqueue(q, &b);
  enqueue(q, &c);
  TEST_ASSERT_FALSE(is_empty(q));
  TEST_ASSERT_EQUAL_PTR(&a, dequeue(q));
  TEST_ASSERT_EQUAL_PTR(&b, dequeue(q));
  TEST_ASSERT_EQUAL_PTR(&c, dequeue(q));
  TEST_ASSERT_TRUE(is_empty(q));
  queue_destroy(q);
}

/**
 * @brief Interleaves enqueue and dequeue operations to test wraparound behavior
 *        and ensure the circular buffer maintains correct FIFO order under mixed use.
 */
void test_interleaved_enqueue_dequeue(void) {
  queue_t q = queue_init(3);
  int a = 1, b = 2, c = 3, d = 4;
  // Enqueue and immediately dequeue
  enqueue(q, &a);
  TEST_ASSERT_EQUAL_PTR(&a, dequeue(q));
  // Enqueue 2 items
  enqueue(q, &b);
  enqueue(q, &c);
  // Dequeue 1 item
  TEST_ASSERT_EQUAL_PTR(&b, dequeue(q));
  // Enqueue 1 more (should wrap around)
  enqueue(q, &d);
  // Dequeue remaining items in order
  TEST_ASSERT_EQUAL_PTR(&c, dequeue(q));
  TEST_ASSERT_EQUAL_PTR(&d, dequeue(q));

  TEST_ASSERT_TRUE(is_empty(q));
  queue_destroy(q);
}

/**
 * @brief Tests that initializing a queue with zero or negative capacity
 *        returns NULL and does not allocate memory.
 */
void test_init_invalid_capacity(void) {
  TEST_ASSERT_NULL(queue_init(0));
  TEST_ASSERT_NULL(queue_init(-5));
}

/**
 * @brief Verifies that enqueueing a NULL pointer is safely ignored
 *        and does not modify the queue or cause a crash.
 */
void test_enqueue_null_data(void) {
  queue_t q = queue_init(5);
  enqueue(q, NULL);
  TEST_ASSERT_TRUE(is_empty(q));
  queue_destroy(q);
}

/**
 * @brief Verifies that a dequeue on an empty queue after shutdown
 *        returns NULL immediately and does not block or crash.
 */
void test_dequeue_from_empty_after_shutdown(void) {
  queue_t q = queue_init(5);
  queue_shutdown(q);
  void *result = dequeue(q);
  TEST_ASSERT_NULL(result);
  queue_destroy(q);
}

/**
 * @brief Enqueues and dequeues a large number of items in chunks to
 *        test wraparound, memory handling, and queue stability under load.
 */
void test_large_volume(void) {
  queue_t q = queue_init(100);
  int items[1000];
  // Enqueue
  for (int i = 0; i < 1000; i++) {
      items[i] = i;
      enqueue(q, &items[i]);
      if ((i + 1) % 100 == 0) {
          // Dequeue in bursts to avoid hitting capacity
          for (int j = i - 99; j <= i; j++) {
              TEST_ASSERT_EQUAL_PTR(&items[j], dequeue(q));
          }
      }
  }
  TEST_ASSERT_TRUE(is_empty(q));
  queue_destroy(q);
}

/**
 * @brief Tests proper wraparound behavior of the circular queue.
 *        Enqueues and dequeues in an interleaved way to confirm head and tail
 *        correctly wrap and maintain FIFO order.
 */
void test_enqueue_dequeue_after_wraparound(void) {
    queue_t q = queue_init(2);
    int a = 1, b = 2, c = 3;
    enqueue(q, &a);
    enqueue(q, &b);
    TEST_ASSERT_EQUAL_PTR(&a, dequeue(q));
    enqueue(q, &c); // wraps around to slot 0
    TEST_ASSERT_EQUAL_PTR(&b, dequeue(q));
    TEST_ASSERT_EQUAL_PTR(&c, dequeue(q));
    queue_destroy(q);
}

#include <pthread.h>

/**
 * @brief Stress test with multiple producers and consumers.
 *        Validates no deadlocks under heavy load and shutdown.
 */
void test_stress_multithreaded(void) {
    // TODO
}


int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_create_destroy);
  RUN_TEST(test_queue_dequeue);
  RUN_TEST(test_queue_dequeue_multiple);
  RUN_TEST(test_queue_dequeue_shutdown);
  RUN_TEST(test_dequeue_empty_after_shutdown);
  RUN_TEST(test_enqueue_after_shutdown);
  RUN_TEST(test_fill_queue_to_capacity);
  RUN_TEST(test_interleaved_enqueue_dequeue);
  RUN_TEST(test_init_invalid_capacity);
  RUN_TEST(test_enqueue_null_data);
  RUN_TEST(test_dequeue_from_empty_after_shutdown);
  RUN_TEST(test_large_volume);
  RUN_TEST(test_enqueue_dequeue_after_wraparound);
  // RUN_TEST(test_stress_multithreaded);
  return UNITY_END();
}