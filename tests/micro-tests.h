//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// micro-tests.h
// -------------
//
// Lightweight, header-only testing framework written in C99, with
// multithread support and run-time settings. Perfect for small to
// medium projects where you want quick, painless testing without
// bringing in a heavy framework.
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// License: MIT
//
//
// Features
// --------
//
// - Single-header + tiny linker script
// - Automatic test registration — no boilerplate needed.
// - Organize tests into suites for cleaner structure.
// - Optional multithreaded execution to speed up large test sets.
// - Command-line controls:
//   - Run a specific suite or test
//   - List available tests
//   - Enable Multithreading
//   - Number of threads
//   - Output settings
//
// Usage
// -----
//
// Do this:
//
//   #define MICRO_TESTS_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define MICRO_TESTS_IMPLEMENTATION
//   #include "micro-tests.h"
//
// You can tune the library by #defining certain values. See the
// "Config" comments under "Configuration" below.
//
// !!IMPORTANT: READ THIS FOR MULTITHREADING !!
// Multithreading execution is optional, if you want to support
// multiple threads, then define MICRO_TESTS_MULTITHREADED before
// including this file.
//
// ```
// #define MICRO_TESTS_MULTITHREADED
// ```
//
// You define a test like this:
//
// ```
// TEST(suite_name, test_name)
// {
//   ASSERT(1);
//   TEST_SUCCESS;
// }
// ```
//
// A test should terminate with either TEST_SUCCESS or TEST_FAILED.
//
// To run the tests, you need to either call micro_tests_run(argc,
// arv), or use the MICRO_TESTS_MAIN macro. The return value will be
// the number of failed tests (0 on success).
//
// ```
// #define MICRO_TESTS_MAIN
//     int main(int argc, char **argv) { return micro_tests_run(argc, argv); }
// ```
//
// !!IMPORTANT: THE TESTS WILL NOT BE REGISTERED IF YOU DO NOT DO THIS!!
// To automatically register the tests, you need to add the linker
// script `mini-tests.ld` to your linker, for example by adding
// "-Wl,-T,micro-tests.ld" when building.
//
// You can finally run the executable, and specify (optional) flags:
//
// ```
// $ ./test --help
// micro-tests usage:
//
//  --help,-h             show help message
//  --list                list tests
//  --suite <suite-name>  run a specific suite
//  --test  <test-name>   run a specific test
//  --multithreaded       run tests on multiple threads
//  --threads <n>         specify the number n of threads (use with --multithreaded)
//  --no-banner           do not print the banner
//  --debug               additional debug prints
//  --quiet               do not print OK results
// ```
//
// Check out more examples at the end of the header.
//
//
// Code
// ----
//
// The official git repository of micro-tests.h is hosted at:
//
//     https://github.com/San7o/micro-tests.h
//
// This is part of a bigger collection of header-only C99 libraries
// called "micro-headers", contributions are welcome:
//
//     https://github.com/San7o/micro-headers
//

#ifndef _MICRO_TESTS_H_
#define _MICRO_TESTS_H_

#define MICRO_TESTS_MAJOR 0
#define MICRO_TESTS_MINOR 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//
// Config
//

// Config: Enable --multithreaded runner by defining
//         MICRO_TESTS_MULTITHREDED
//
// Note: Disabled by default
#if 0
  #define MICRO_TESTS_MULTITHREADED
#endif

// Config: Allocator, used only for multithreaded execution
//
// Note: should behave like calloc(3)
#ifdef MICRO_TESTS_MULTITHREADED
#ifndef MICRO_TESTS_CALLOC
#define MICRO_TESTS_CALLOC calloc
#endif
#endif

// Config: Free allocated memory, used only for multithreaded execution
//
// Note: should behave like free(3)
#ifdef MICRO_TESTS_MULTITHREADED
#ifndef MICRO_TESTS_FREE
#define MICRO_TESTS_FREE free
#endif
#endif

//
// Macros
//

#define ASSERT(condition)     \
  do {                        \
    if (!(condition))  	      \
    {                         \
      fprintf(stderr, "error: %s:%d: failed assertion: %s\n", \
              __FILE__, __LINE__, #condition);     \
      return -1;              \
    }                         \
  } while(0)


#define ASSERT_EQ(a, b)       \
  do {                        \
    if (a != b)       	      \
    {                         \
      fprintf(stderr, "error: %s:%d: failed expect equal: %s and %s\n", \
              __FILE__, __LINE__, #a, #b);                        \
      return -1;              \
    }                         \
  } while(0)

#define ASSERT_NOT_EQ(a, b)     \
  do {                          \
    if (a == b)                 \
    {                           \
      fprintf(stderr, "error: %s:%d: failed expect not equal: %s and %s\n", \
              __FILE__, __LINE__, #a, #b);                              \
      return -1;                \
    }                           \
  } while(0)

#if __STDC_VERSION__ >= 201112L
  #define ALIGNOF(T) _Alignof(T)
#else
  #define ALIGNOF(T) __alignof__(T)
#endif
  
// Register a test case
//
// Args:
//  - arg1: suite name
//  - arg2: test name
//
// Note: the test should terminate with either TEST_SUCCESS or
// TEST_FAILED.
//
// Credits: Thanks to Sam P. (stackoverflow)
#define TEST(__suite_name, __test_name)                        \
  static int __suite_name##_##__test_name(void);               \
  static MicroTest __micro_test_record_##__suite_name##_##__test_name   \
  __attribute__((used, section(".micro_tests"), aligned(sizeof(ALIGNOF(MicroTest))))) = { \
    .marker = 0xDeadBeaf,                                      \
    .test_suite = #__suite_name,                               \
    .test_name = #__test_name,                                 \
    .file_name = __FILE__,                                     \
    .line_number = __LINE__,                                   \
    .function_name = #__suite_name "_" #__test_name,           \
    .function_pointer = __suite_name##_##__test_name           \
  };                                                           \
  static int __suite_name##_##__test_name(void)

// Exit a test successfully
#define TEST_SUCCESS \
  do { return 0; } while(0)

// Exit a test unsuccessfully
#define TEST_FAILED \
  do { return -1; } while(0)

// A main() function for running the tests
#define MICRO_TESTS_MAIN \
  int main(int argc, char **argv) { return micro_tests_run(argc, argv); }

//
// Types
//

#ifdef MICRO_TESTS_MULTITHREADED
  #include <pthread.h>
#endif
  
// A MicroTest
//
// Note: Each test will create this struct automatically in the
// section .micro_tests
typedef struct __attribute__((packed)) {
  // A known set of bytes to validate that the struct is correctly
  // aligned
  uint32_t marker;                   // To validate the Test
  // Name of the test suite
  const char* test_suite;
  // Name of the test
  const char* test_name;
  // test_suite + "_" + test_name
  const char* function_name;
  // File where the test is located
  const char* file_name;
  // Line where the test is located
  uint32_t line_number;
  // Test function
  int (*function_pointer)(void);

} MicroTest;

// Settings for the MicroTests framework
typedef struct {
  // If specified, run a specific test suite
  const char *run_suite;
  // If specified, run a specific test
  const char *run_test;
#ifdef MICRO_TESTS_MULTITHREADED
  // Whether to run the tests with multiple threads
  _Bool run_multithreaded;
  // Number of threads to use of multithreaded is enabled
  int thread_number;
#endif
  // Whether to show a list of the tests
  _Bool show_list;
  // Whether to print the banner at the start of the tests
  _Bool print_banner;
  // Whether to print the help message
  _Bool print_help;
  // Additional debug prints
  _Bool debug;
  // Whether to not print OK results
  _Bool quiet;
} MicroTests;

//
// Functions
//

// Parse arguments settings
//
// Args:
//  - micro_tests: pointer to the MicroTests structure to initialize
//  - argc: number of arguments
//  - argv: arguments
//
// Returns: 0 on success, or a negative value on failure
int micro_tests_parse_args(MicroTests *micro_tests, int argc, char **argv);

// Print a list of the tests and return
//
// Args:
//  - micro_tests: the settings of the testing framework
void micro_tests_show_list(MicroTests *micro_tests);

// Run tests
//
// Args:
//  - argc: number of arguments
//  - argv: arguments
//
// Returns: 0 on success, or the number of failed tests
int micro_tests_run(int argc, char **argv);

void micro_tests_print_banner(void);
void micro_tests_print_help(void);

int _micro_tests_run(MicroTests *micro_tests);
int _micro_tests_strcmp(const char* s1, const char *s2);

#ifdef MICRO_TESTS_MULTITHREADED

// Get the next MicroTest to run
//
// Args:
//  - micro_tests: settings for the testing framework
//
// Returns: a pointer to a MicroTest
//
// Notes: Can be called by multiple threads
MicroTest *_micro_tests_get_next_test(MicroTests *micro_tests);

// A single test runner
//
// Args:
//  - micro_tests: settings for the testing framework
//
// Returns: The number of failed tests, casted to a (void*)
void *_micro_tests_thread(void *micro_tests);

// Run the tests with multiple threads
//
// Args:
//  - micro_tests: settings for the testing framework
//
// Returns: The number of failed tests
int _micro_tests_run_multithreaded(MicroTests *micro_tests);

#endif // MICRO_TESTS_MULTITHREADED

//
// Variables
//

// Start of the micro tests section (exported by the linker)
extern char __micro_tests_start[];
// End of the micro tests section (exported by the linker)
extern char __micro_tests_stop[];

//
// Implementation
//

#ifdef MICRO_TESTS_IMPLEMENTATION

int _micro_tests_strcmp(const char* s1, const char *s2)
{
  while (*s2 != '\0' && *s1 != '\0')
  {
    if (*s1 != *s2)
      return (*s1 > *s2) ? 1 : -1;
    s1++; s2++;
  }
  return !(*s2 == '\0' && *s1 == '\0');
}

int micro_tests_parse_args(MicroTests *micro_tests, int argc, char **argv)
{
  // Default values
  *micro_tests = (MicroTests){
    .run_suite         = NULL,
    .run_test          = NULL,
#ifdef MICRO_TESTS_MULTITHREADED
    .run_multithreaded = 0,
    .thread_number     = 4,
#endif
    .show_list         = 0,
    .print_banner      = 1,
    .print_help        = 0,
    .debug             = 0,
    .quiet             = 0,
  };

  for (int i = 1; i < argc; ++i)
  {
    if (_micro_tests_strcmp(argv[i], "--help") == 0
        || _micro_tests_strcmp(argv[i], "-h") == 0)
    {
      micro_tests->print_help = 1;
    } else if (_micro_tests_strcmp(argv[i], "--list") == 0)
    {
      micro_tests->show_list = 1;
    } else if (_micro_tests_strcmp(argv[i], "--suite") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Usage: --suite <suite-name>\n");
        return -1;
      }
      micro_tests->run_suite = argv[++i];
    } else if (_micro_tests_strcmp(argv[i], "--test") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Usage: --test <test-name>\n");
        return -1;
      }
      micro_tests->run_test = argv[++i];
    } else if (_micro_tests_strcmp(argv[i], "--no-banner") == 0)
    {
      micro_tests->print_banner = 0;
    } else if (_micro_tests_strcmp(argv[i], "--debug") == 0)
    {
      micro_tests->debug = 1;
    } else if (_micro_tests_strcmp(argv[i], "--quiet") == 0)
    {
      micro_tests->quiet = 1;
#ifdef MICRO_TESTS_MULTITHREADED
    } else if (_micro_tests_strcmp(argv[i], "--multithreaded") == 0)
    {
      micro_tests->run_multithreaded = 1;
    } else if (_micro_tests_strcmp(argv[i], "--threads") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Usage: --threads <n>\n");
        return -1;
      }
      micro_tests->thread_number = atoi(argv[++i]);
      if (micro_tests->thread_number <= 0)
      {
        fprintf(stderr,
                "Error: Thread number %s must be an integer and positive number",
                argv[i]);
        return -1;
      }
#endif // MICRO_TESTS_MULTITHREADED
    } else {
      printf("Unrecognized argument: %s\n", argv[i]);
      printf("Try --help or -h\n");
      return -1;
    }
  }
  return 0;
}

int _micro_tests_run(MicroTests *micro_tests)
{
  int out = 0;
  size_t count = (__micro_tests_stop - __micro_tests_start) / sizeof(MicroTest);
  MicroTest* test = (MicroTest*)__micro_tests_start;

  for (size_t i = 0; i < count; i++)
  {
    MicroTest* current = &test[i];
    
    if (current->marker == 0xDeadBeaf)
    {
      if (micro_tests->run_suite != NULL &&
          _micro_tests_strcmp(micro_tests->run_suite, current->test_suite) != 0)
        continue;
      if (micro_tests->run_test != NULL &&
          _micro_tests_strcmp(micro_tests->run_test, current->test_name) != 0)
        continue;
      
      int ret = current->function_pointer();       // Execute the test.
      if (ret < 0)
      {
        fprintf(stderr, "suite: %s, test: %s FAILED\n",
                current->test_suite,
                current->test_name);
      } else if (!micro_tests->quiet) {
        printf("suite: %s, test: %s OK\n",
               current->test_suite,
               current->test_name);
      }
      out += ret;
    }
  }

  if (!micro_tests->quiet)
    printf("\nTests done: %d %s failed\n\n", -out, (out == -1) ? "test" : "tests");
  return -out;
}

#ifdef MICRO_TESTS_MULTITHREADED

int micro_tests_current = 0;
pthread_mutex_t micro_tests_current_mutex = PTHREAD_MUTEX_INITIALIZER;

MicroTest *_micro_tests_get_next_test(MicroTests *micro_tests)
{
  pthread_mutex_lock(&micro_tests_current_mutex);

  MicroTest* test = (MicroTest*)__micro_tests_start;
  size_t count = (__micro_tests_stop - __micro_tests_start) / sizeof(MicroTest);

  for (int i = micro_tests_current; i < count; ++i)
  {
    MicroTest* current = &test[i];
    
    if (current->marker == 0xDeadBeaf)
    {
      if (micro_tests->run_suite != NULL &&
          _micro_tests_strcmp(micro_tests->run_suite, current->test_suite) != 0)
        continue;
      if (micro_tests->run_test != NULL &&
          _micro_tests_strcmp(micro_tests->run_test, current->test_name) != 0)
        continue;

      micro_tests_current++;
      pthread_mutex_unlock(&micro_tests_current_mutex);
      return current;
    }
  }
  
  pthread_mutex_unlock(&micro_tests_current_mutex);
  return NULL;
}

void *_micro_tests_thread(void *args)
{
  long ret = 0;
  MicroTests *micro_tests = (MicroTests*) args;
  MicroTest *micro_test = _micro_tests_get_next_test(micro_tests);
  while (micro_test != NULL)
  {
    if (micro_test->marker != 0xDeadBeaf)
      return (void*)-1;

    if (micro_tests->debug)
    {
      printf("(thread %lu) ", pthread_self());
    }
    
    ret += micro_test->function_pointer();       // Execute the test.
    if (ret < 0)
    {
      fprintf(stderr, "suite: %s test: %s FAILED\n",
              micro_test->test_suite,
              micro_test->test_name);
    } else if (!micro_tests->quiet) {
      printf("suite: %s, test: %s OK\n",
             micro_test->test_suite,
             micro_test->test_name);
    }
    micro_test = _micro_tests_get_next_test(micro_tests);
  }
  
  return (void*)ret; 
}

int _micro_tests_run_multithreaded(MicroTests *micro_tests)
{
  if (micro_tests->print_banner)
    printf("Running multithreaded with %d threads.\n\n", micro_tests->thread_number);

  if (pthread_mutex_init(&micro_tests_current_mutex, NULL) < 0)
  {
    perror("pthread_mutex_init");
    return -1;
  }
  
  pthread_t *thread_buff = MICRO_TESTS_CALLOC(micro_tests->thread_number,
                                              sizeof(pthread_t));

  // Spawn threads
  for (int i = 0; i < micro_tests->thread_number; ++i)
  {
    if (pthread_create(&thread_buff[i], NULL, &_micro_tests_thread, (void*) micro_tests) != 0)
      perror("run_multithreaded: Error in pthread_create");
  }

  // Wait for threads
  long ret = 0;
  void *ret_tmp;
  for (int i = 0; i < micro_tests->thread_number; ++i)
  {
    if (pthread_join(thread_buff[i], &ret_tmp) != 0)
      perror("pthread_join");
    ret += (long)ret_tmp;
  }
  
  MICRO_TESTS_FREE(thread_buff);
  pthread_mutex_destroy(&micro_tests_current_mutex);

  if (!micro_tests->quiet)
    printf("\nTests done: %ld %s failed\n\n", -ret, (ret == -1) ? "test" : "tests");
  return ret;
}
#endif // MICRO_TESTS_MULTITHREADED

int micro_tests_run(int argc, char **argv)
{
  MicroTests micro_tests;
  
  if (micro_tests_parse_args(&micro_tests, argc, argv) < 0)
    return 1;

  if (micro_tests.print_help)
  {
    micro_tests_print_help();
    return 0;
  }

  if (micro_tests.show_list)
  {
    micro_tests_show_list(&micro_tests);
    return 0;
  }

  if (micro_tests.print_banner)
    micro_tests_print_banner();
  
  if (micro_tests.debug)
  {
    printf("debug: __micro_tests_start=%p, __micro_tests_stop=%p\n",
           (void*)__micro_tests_start,
           (void*)__micro_tests_stop);
  }

#ifdef MICRO_TESTS_MULTITHREADED
  if (micro_tests.run_multithreaded && micro_tests.thread_number > 0)
    return _micro_tests_run_multithreaded(&micro_tests);
#endif

  return _micro_tests_run(&micro_tests);
}

void micro_tests_print_banner(void)
{
  printf("\n");
  printf("micro-tests.h\n");
  printf("-------------\n");
  printf("\n");
  printf("Running tests...\n\n");
  return;
}

void micro_tests_print_help(void)
{
  printf("micro-tests usage:\n");
  printf("\n");
  printf("  --help,-h             show help message\n");
  printf("  --list                list tests\n");
  printf("  --suite <suite-name>  run a specific suite\n");
  printf("  --test  <test-name>   run a specific test\n");
#ifdef MICRO_TESTS_MULTITHREADED
  printf("  --multithreaded       run tests on multiple threads\n");
  printf("  --threads <n>         specify the number n of threads (use with --multithreaded)\n");
#endif // MICRO_TESTS_MULTITHREADED
  printf("  --no-banner           do not print the banner\n");
  printf("  --debug               additional debug prints\n");
  printf("  --quiet               do not print OK results\n");
}

void micro_tests_show_list(MicroTests *micro_tests)
{
  size_t count = (__micro_tests_stop - __micro_tests_start) / sizeof(MicroTest);
  MicroTest* test = (MicroTest*)__micro_tests_start;

  for (size_t i = 0; i < count; i++)
  {
    MicroTest* current = &test[i];
    
    if (current->marker == 0xDeadBeaf)
    {
      if (micro_tests->run_suite != NULL &&
          _micro_tests_strcmp(micro_tests->run_suite, current->test_suite) != 0)
        continue;
      if (micro_tests->run_test != NULL &&
          _micro_tests_strcmp(micro_tests->run_test, current->test_name) != 0)
        continue;
      
      printf("suite: %s, test: %s\n",
             current->test_suite,
             current->test_name);
    }
  }
}

#endif // MICRO_TESTS_IMPLEMENTATION

//
// Examples
//

#if 0

#define MICRO_TESTS_MULTITHREADED
#define MICRO_TESTS_IMPLEMENTATION
#include "micro-tests.h"

TEST(base_tests, simple_assertion)
{
  ASSERT(1);
  TEST_SUCCESS;
}

TEST(base_tests, simple_assert_eq)
{
  ASSERT_EQ(1, 1);
  TEST_SUCCESS;
}

TEST(base_tests2, simple_assert_not_eq)
{
  ASSERT_NOT_EQ(1, 0);
  TEST_SUCCESS;
}

MICRO_TESTS_MAIN

// then run:
//   $ ./test --multithreaded

#endif //0

#ifdef __cplusplus
}
#endif

#endif // _MICRO_TESTS_H_
