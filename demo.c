// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define _POSIX_C_SOURCE 199309L

#define LAUNCHPAD_IMPLEMENTATION
#include "liblaunchpad.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#define HITSOUND "sound/hitsound.mp3"

#define MAGIC1 1664525    // a
#define MAGIC2 1013904223 // c
#define MAGIC3 (1<<31)    // m

// LCG pseudo random number generator
unsigned int lcg(const unsigned int seed)
{
  return (MAGIC1 * seed + MAGIC2) % MAGIC3;
}

int main(void)
{
  LP lp;
  assert(lp_open(&lp, "hw:1,0,0", true) == LP_OK);

  ma_engine audio_engine;
  if (ma_engine_init(NULL, &audio_engine) != MA_SUCCESS) return 1;
  
  LPNote notes[LP_ROWS * LP_COLS];
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      notes[i * LP_ROWS + j] = LP_NOTE(LP_NOTE_OFF, LP_KEY(i,j), 0);
  
  LPEvent event = {0};
  bool loop = true;
  double delta_time = 0.0;
  double fps = 30.0;
  struct timespec frame_start, frame_end;
  
  double generate_delta_time = 0.0;
  double generate_frequency = 0.5;  // notes per second
  unsigned int random = 1337;

  int score = 0;
  bool cleared = true;
  while(loop)
  {
    clock_gettime(CLOCK_MONOTONIC, &frame_start);
    if (delta_time < 1 / fps) goto next;
    delta_time = 0.0;
        
    if (lp_check_event(&lp, &event) > 0)
    {
      if (event.type == LP_EVENT_PRESSED)
      {
        LPNote *note = &notes[event.note_y * LP_ROWS + event.note_x];
        if (note->state == LP_NOTE_ON)
        {
          cleared = true;
          note->state = LP_NOTE_OFF;
          generate_frequency += 0.1;
          score++;
          ma_engine_play_sound(&audio_engine, HITSOUND, NULL);
        }
      }
    }

    lp_swap_buffers(&lp);

    if (generate_delta_time > 1 / generate_frequency)
    {
      if (!cleared) goto lost;
      cleared = false;
      // Generate a new note
      generate_delta_time = 0.0;
      random = lcg(random);
      int random_row = random % 8;
      int random_col = (random / 8) % 8;
      notes[random_row * LP_COLS + random_col] =
        LP_NOTE(LP_NOTE_ON, LP_KEY(random_row, random_col), LP_COLOR_GREEN_FULL);
    }
    
    lp_set_notes(&lp, notes);

  next:
    // Wait a little bit
    nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &frame_end);
    double diff = (frame_end.tv_sec - frame_start.tv_sec)
      + (frame_end.tv_nsec - frame_start.tv_nsec) / 1e9;
    delta_time += diff;
    generate_delta_time += diff;
    continue;

  lost:
    printf("You lost!\n");
    loop = false;
    continue;
  }

  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
      notes[i * LP_ROWS + j] = LP_NOTE(LP_NOTE_ON, LP_KEY(i,j), LP_COLOR_RED_FULL);
  
  printf("Your score: %d\n", score);

  lp_set_notes(&lp, notes);
  
  lp_enable_flashing(&lp);
  sleep(1);
  lp_disable_flashing(&lp);
  lp_swap_buffers(&lp);
  sleep(3);
  
  lp_reset(&lp);
  ma_engine_uninit(&audio_engine);
  lp_close(&lp);
  return 0;
}
