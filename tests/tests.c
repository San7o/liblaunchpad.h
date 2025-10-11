// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define MICRO_TESTS_IMPLEMENTATION
#include "micro-tests.h"

#define LAUNCHPAD_IMPLEMENTATION
#include "../liblaunchpad.h"

#define LP_DEVICENAME "hw:1,0,0"

TEST(lp_tests, set_note)
{
  LP lp;
  ASSERT(lp_open(&lp, LP_DEVICENAME, true) == LP_OK);

  ASSERT(lp_set_note(&lp,
                     LP_NOTE(LP_NOTE_ON, LP_KEY(0,0), LP_COLOR_RED_FULL))
         == LP_OK);
  sleep(1);

  ASSERT(lp_set_note(&lp,
                     LP_NOTE(LP_NOTE_ON, LP_KEY(0,0), LP_COLOR_GREEN_FULL))
         == LP_OK);
  sleep(1);

  ASSERT(lp_set_note(&lp,
                     LP_NOTE(LP_NOTE_ON, LP_KEY(0,0), LP_COLOR_YELLOW_FULL))
         == LP_OK);
  sleep(1);

  ASSERT(lp_set_note(&lp,
                     LP_NOTE(LP_NOTE_OFF, LP_KEY(0,0), 0)) == LP_OK);
  sleep(1);

  ASSERT(lp_reset(&lp) == LP_OK);
  ASSERT(lp_close(&lp) == LP_OK);
  
  TEST_SUCCESS;
}

TEST(lp_tests, set_multiple_notes)
{
  LP lp;
  ASSERT(lp_open(&lp, LP_DEVICENAME, false) == LP_OK);


  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      ASSERT(lp_set_note(&lp,
                         LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_RED_FULL))
             == LP_OK);
  sleep(1);
  
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      ASSERT(lp_set_note(&lp,
                         LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_GREEN_FULL))
             == LP_OK);
  sleep(1);
    
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      ASSERT(lp_set_note(&lp,
                         LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_YELLOW_FULL))
             == LP_OK);
  sleep(1);

  ASSERT(lp_reset(&lp) == LP_OK);
  ASSERT(lp_close(&lp) == LP_OK);

  TEST_SUCCESS;
}

TEST(lp_tests, double_buffering)
{
  LP lp;
  ASSERT(lp_open(&lp, LP_DEVICENAME, false) == LP_OK);

  ASSERT(lp_set_double_buffering_flags(&lp, LP_DOUBLE_BUFFERING_DISPLAY_1
                                       | LP_DOUBLE_BUFFERING_UPDATE_0
                                       | LP_DOUBLE_BUFFERING_COPY) == LP_OK);
  
  LPNote notes[LP_ROWS * LP_COLS];
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      notes[i * LP_ROWS + j] =
        LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_GREEN_FULL);
  ASSERT(lp_set_notes(&lp, notes) == LP_OK);
  ASSERT(lp_set_double_buffering_flags(&lp, LP_DOUBLE_BUFFERING_DISPLAY_0
                                       | LP_DOUBLE_BUFFERING_UPDATE_1
                                       | LP_DOUBLE_BUFFERING_COPY) == LP_OK);
  sleep(1);
  
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      notes[i * LP_ROWS + j] =
        LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_YELLOW_FULL);
  ASSERT(lp_set_notes(&lp, notes) == LP_OK);
  ASSERT(lp_set_double_buffering_flags(&lp, LP_DOUBLE_BUFFERING_DISPLAY_1
                                       | LP_DOUBLE_BUFFERING_UPDATE_0
                                       | LP_DOUBLE_BUFFERING_COPY) == LP_OK);
  sleep(1);

  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      notes[i * LP_ROWS + j] =
        LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_RED_FULL);
  ASSERT(lp_set_notes(&lp, notes) == LP_OK);
  ASSERT(lp_set_double_buffering_flags(&lp, LP_DOUBLE_BUFFERING_DISPLAY_0
                                       | LP_DOUBLE_BUFFERING_UPDATE_1
                                       | LP_DOUBLE_BUFFERING_COPY) == LP_OK);
  sleep(1);

  
  ASSERT(lp_reset(&lp) == LP_OK);
  ASSERT(lp_close(&lp) == LP_OK);

  TEST_SUCCESS;
}

MICRO_TESTS_MAIN
