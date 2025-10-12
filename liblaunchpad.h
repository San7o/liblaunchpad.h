//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// liblaunchpad.h
// ==============
//
// An abstraction layer over Novation's Launchpad S through ALSA, as an
// header-only C99 library.
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o
//
//
// Example
// =======
//
// ```
// #define LIBLAUNCHPAD_IMPLEMENTATION
// #include "liblaunchpad.h"
//
// #define LP_DEVICENAME "hw:1,0,0"
//
// int main(void)
// {
//   LP lp;
//   lp_open(&lp, LP_DEVICENAME, true);
//
//   // Turn on note 0,0 with color red
//   lp_set_note(&lp, LP_NOTE(LP_NOTE_ON, LP_KEY(0,0), LP_COLOR_RED_FULL));
//   // Turn on note 0,3 with color green
//   lp_set_note(&lp, LP_NOTE(LP_NOTE_ON, LP_KEY(0,3), LP_COLOR_GREEN_FULL));
//
//   sleep(1);
// 
//   // Cleanup
//   lp_reset(&lp);
//   lp_close(&lp);
//   return 0;
// }
// ```
//
//
// Overview
// ========
//
// The Launchpad S is an USB device that can be controlled via MIDI
// 1.0, you operating is doing the heavy work for us so we can use its
// audio api to send / read bytes through MIDI. This library provides
// an abstraction layer to control the device more easily, using
// libasound for the communication.
//
// Indexing
// --------
//
// The buttons ("notes") on the Launchpad S are indexed like this:
//
//               |------------------------------|
//               | A0 A1 A2 A3 A4 A5 A6 A7      |
//               |  0  1  2  3  4  5  6  7    8 |
//               | 16 17 18 19 20 21 22 23   24 |
//               | 32  .  .  .  .  .  .  .   40 |
//               | 48  .  .  .  .  .  .  .   56 |
//               | 64  .  .  .  .  .  .  .   72 |
//               | 80  .  .  .  .  .  .  .   88 |
//               | 96  .  .  .  .  .  .  .  104 |
//               | 112 .  .  .  .  .  .  .  120 |
//               |------------------------------|
//
// The top row is a special row that is referred to as Automap / Live.
// In this library it will be referred to only as "Automap".  All the
// other notes are in a grid, where each row starts with a multiple
// of 16 and is 9 notes long. In this library you can index the grid
// by rows and columns from 0 to 7, which is nicer, using `lp_set_note`
// or `lp_set_notes`. Notes can either be on or off, with a color value.
//
// Colors
// ------
//
// Each note can have either a low, medium, full or no intensity for
// green and red colors. By mixing green and red you get yellow or
// amber. These are the only colors you can get on the Launchpad S.
// Additionally, you can specify some flags for advanced handling of
// colors when double buffering, and for flashing. This library
// provides some macros to create colors, and some default colors.
//
// Double buffering
// ----------------
//
// The Launchpad S supports two buffers, 0 and 1. One is used for
// drawing and the other for updating; when the Launchpad's notes need
// to be updated, the buffers can be swapped. This is a performance
// improvement that is commonly seen videogames. This library provides
// the function `lp_swap_buffers` to, guess what, swap the buffers.
//
// Input
// -----
//
// When a note is pressed or release, the Launchpad will send some
// bytes with the index of the note and a velocity to be used to
// distinguish if the button was pressed or released. This library
// provides an event api where you can read events, either in a
// blocking or non-blocking way (specified during initialization),
// with the `lp_check_event` and then inspecting the `LPEvent` struct.
//
// Full reference:
//
//    https://leemans.ch/latex/doc_launchpad-programmers-reference.pdf
//
//
// Usage
// =====
//
// Do this:
//
//   #define LIBLAUNCHPAD_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define LIBLAUNCHPAD_IMPLEMENTATION
//   #include "liblaunchpad.h"
//
//
// Code
// ====
//
// The official git repository of liblaunchpad.h is hosted at:
//
//     https://github.com/San7o/liblaunchpad.h
//
// If you liked this library, you may find useful a bigger collection
// of header-only C99 libraries called "micro-headers", contributions
// are welcome:
//
//     https://github.com/San7o/micro-headers
//


#ifndef _LIBLAUNCHPAD_H_
#define _LIBLAUNCHPAD_H_

#define LIBLAUNCHPAD_MAJOR 0
#define LIBLAUNCHPAD_MINOR 1

#ifdef __cplusplus
extern "C" {
#endif

#include <alsa/asoundlib.h>
#include <stdbool.h>

// Errors
#define LP_OK                       0
#define LP_ERROR_LP_NULL           -1
#define LP_ERROR_OPENING_LAUNCHPAD -2
#define LP_ERROR_UNINITIALIZED     -3
#define LP_ERROR_MIDI_WRITE        -4
#define LP_ERROR_MIDI_DRAIN        -5
#define LP_ERROR_MIDI_CLOSE        -6
#define LP_ERROR_ARGUMENT_NULL     -7
#define LP_ERROR_MIDI_READ         -8

// Main grid's rows and columns
#define LP_ROWS 8
#define LP_COLS 8

// Notes can be either ON or OFF
typedef unsigned char LPNoteState;
enum {
  LP_NOTE_ON  = 0x90,
  LP_NOTE_OFF = 0x80,
};

// The NoteKey is the device index for a node
typedef unsigned char LPNoteKey;
// Use LP_KEY to calculate the index from [row] and [col]
#define LP_KEY(row, col) ((0x10 * row) + col)

// The color can have some flags for specific usage
typedef unsigned char LPNoteColor;
typedef enum {
  // For double buffering, clear the other buffer’s copy of this LED.
  LP_COLOR_FLAG_CLEAR = (1<<2),
  // For double buffering, write this LED data to both buffers.
  // Note: this behavior overrides the Clear behavior
  // when both bits are set
  LP_COLOR_FLAG_COPY  = (1<<3),
} LPNoteColorFlags;

// Brightness level for the color greed or red
typedef enum {
  LP_BRIGHTNESS_OFF    = 0,
  LP_BRIGHTNESS_LOW    = 1,
  LP_BRIGHTNESS_MEDIUM = 2,
  LP_BRIGHTNESS_FULL   = 3,
} LPNoteBrightness;
// Use this to define a color
#define LP_COLOR(green, red, flags) ((0x10 * green) + red + flags)

// Some predefined colors
#define LP_COLOR_RED_LOW LP_COLOR(LP_BRIGHTNESS_OFF, LP_BRIGHTNESS_LOW, 0)
#define LP_COLOR_RED_MEDIUM LP_COLOR(LP_BRIGHTNESS_OFF, LP_BRIGHTNESS_MEDIUM, 0)
#define LP_COLOR_RED_FULL LP_COLOR(LP_BRIGHTNESS_OFF, LP_BRIGHTNESS_FULL, 0)
#define LP_COLOR_GREEN_LOW LP_COLOR(LP_BRIGHTNESS_LOW, LP_BRIGHTNESS_OFF, 0)
#define LP_COLOR_GREEN_MEDIUM LP_COLOR(LP_BRIGHTNESS_MEDIUM, LP_BRIGHTNESS_OFF, 0)
#define LP_COLOR_GREEN_FULL LP_COLOR(LP_BRIGHTNESS_FULL, LP_BRIGHTNESS_OFF, 0)
#define LP_COLOR_YELLOW_LOW LP_COLOR(LP_BRIGHTNESS_LOW, LP_BRIGHTNESS_LOW, 0)
#define LP_COLOR_YELLOW_MEDIUM LP_COLOR(LP_BRIGHTNESS_MEDIUM, LP_BRIGHTNESS_MEDIUM, 0)
#define LP_COLOR_YELLOW_FULL LP_COLOR(LP_BRIGHTNESS_FULL, LP_BRIGHTNESS_FULL, 0)

// A note
typedef struct {
  // Either ON or OFF
  LPNoteState state;
  // The identifier of the key in the device
  LPNoteKey key;
  // The device's representation of the color
  LPNoteColor color;
} LPNote;
#define LP_NOTE(state, key, color) (LPNote){ state, key, color }

typedef enum {
  LP_EVENT_PRESSED          = 1,
  LP_EVENT_RELEASED         = 2,
  LP_EVENT_AUTOMAP_PRESSED  = 3,
  LP_EVENT_AUTOMAP_RELEASED = 4,
} LPEventType;

// An event
typedef struct {
  LPEventType type;
  unsigned char note_x;
  unsigned char note_y;
} LPEvent;

// The Launchpad has two LED buffers, 0 and 1. Either one can be
// displayed while either is updated by incoming LED instructions.
//
// Use these flags with lp_set_double_buffering_flags, for example:
//
//     lp_set_double_buffering_flags(midi_out, LP_DOUBLE_BUFFERING_DISPLAY_1
//                                   | LP_DOUBLE_BUFFERING_UPDATE_0
//                                   | LP_DOUBLE_BUFFERING_COPY);
//     // set notes....
//
//     // Swap the buffers
//     lp_set_double_buffering_flags(midi_out, LP_DOUBLE_BUFFERING_DISPLAY_0
//                                   | LP_DOUBLE_BUFFERING_UPDATE_1
//                                   | LP_DOUBLE_BUFFERING_COPY);
//
//     // ...
//
typedef unsigned char LPDoubleBufferingFlag;
enum {
  // Set buffer 0 as the new ‘displaying’ buffer.
  LP_DOUBLE_BUFFERING_DISPLAY_0 = 0,
  // Set buffer 1 as the new ‘displaying’ buffer.
  LP_DOUBLE_BUFFERING_DISPLAY_1 = 1,
  // Set buffer 0 or buffer 1 as the new ‘updating’ buffer.
  LP_DOUBLE_BUFFERING_UPDATE_0  = 0,
  // Set buffer 0 or buffer 1 as the new ‘updating’ buffer.
  LP_DOUBLE_BUFFERING_UPDATE_1  = (1<<2),
  // continually flip ‘displayed’ buffers to make selected LEDs flash.
  LP_DOUBLE_BUFFERING_FLASH   = (1<<3),
  // copy the LED states from the new ‘displayed’ buffer to the new
  // ‘updating’ buffer.
  LP_DOUBLE_BUFFERING_COPY    = (1<<4),
};

// Launchpad S context
typedef struct {
  // Reading channel
  snd_rawmidi_t *midi_in;
  // Writing channel
  snd_rawmidi_t *midi_out;
  // Current buffer displayed, either 0 or 1
  int current_buff;
} LP;

//
// Function declarations
//

// Open a device with name [devicename]
// If [nonblocking] is set to true, reading events will be non-blocking.
// Returns either LP_OK or a negative LP_ERROR.
int lp_open(LP *lp, char* devicename, bool nonblocking);

// Reset all the notes in the Launchpad, turning the lights off
// Returns either LP_OK or a negative LP_ERROR.
int lp_reset(LP *lp);

// Closes communication with the Launchpad
// Returns either LP_OK or a negative LP_ERROR.
int lp_close(LP *lp);

// Set a note on the device with the specified [note]
// Returns either LP_OK or a negative LP_ERROR.
int lp_set_note(LP *lp, LPNote note);

// Set the 8x8 grid on the device with [notes] array
// Returns either LP_OK or a negative LP_ERROR.
int lp_set_notes(LP *lp, LPNote notes[LP_ROWS * LP_COLS]);

// Low level control over double buffering, set [flags] on the device
// Returns either LP_OK or a negative LP_ERROR.
int lp_set_double_buffering_flags(LP *lp, LPDoubleBufferingFlag flags);

// Swap buffers on the device
int lp_swap_buffers(LP *lp);

// Check if an event occurred, setting [event]
// If nonblocking was not set in `lp_open`, this function will block
// until an event or an error happened. Returns 1 in case or an event,
// or an LP_ERROR otherwise.
// If nonblocking was specified in `lp_open`, this function will not
// block and return 0 if no event happened.
int lp_check_event(LP *lp, LPEvent *event);

// Enable flashing, which will repeatedly swap buffers at a default speed
int lp_enable_flashing(LP *lp);

// Disable fleshing, if enabled
int lp_disable_flashing(LP *lp);
  
//
// Implementation
//

#ifdef LIBLAUNCHPAD_IMPLEMENTATION

int lp_open(LP *lp, char *devicename, bool nonblocking)
{
  if (!lp) return LP_ERROR_LP_NULL;
  int flags = 0;
  if (nonblocking) flags |= SND_RAWMIDI_NONBLOCK;
  if (snd_rawmidi_open(&lp->midi_in, &lp->midi_out, devicename, flags) < 0)
    return LP_ERROR_OPENING_LAUNCHPAD;
  
  return LP_OK;
}

int lp_reset(LP *lp)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;
  
  unsigned char msg_buff[3] = { 0xB0, 0, 0};
  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;

  return LP_OK;
}

int lp_close(LP *lp)
{
  if (!lp) return LP_OK;
  if (lp->midi_in)
    if (snd_rawmidi_close(lp->midi_in) < 0) return LP_ERROR_MIDI_CLOSE;
  if (lp->midi_out)
    if (snd_rawmidi_close(lp->midi_out) < 0) return LP_ERROR_MIDI_CLOSE;
  
  return LP_OK;
}

int lp_set_note(LP *lp, LPNote note)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;
    
  unsigned char msg_buff[3] = { note.state, note.key, note.color };
  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;
  
  return LP_OK;
}

int lp_set_notes(LP *lp, LPNote notes[LP_ROWS * LP_COLS])
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;
  if (!notes) return LP_ERROR_ARGUMENT_NULL;
  
  unsigned char msg_buff[3 * LP_ROWS * LP_COLS];
  for (int i = 0; i < LP_ROWS; ++i)
    for (int j = 0; j < LP_COLS; ++j)
    {
      unsigned char command_buff[3] = { notes[i * LP_ROWS + j].state,
                                        notes[i * LP_ROWS + j].key,
                                        notes[i * LP_ROWS + j].color };
      memcpy(msg_buff + 3 * (i * LP_COLS + j), command_buff, sizeof(command_buff));
    }

  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3 * LP_ROWS * LP_COLS) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;
  
  return LP_OK;
}

int lp_set_double_buffering_flags(LP *lp, LPDoubleBufferingFlag flags)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;
  
  unsigned char msg_buff[3] = { 0xB0, 0, flags + 0x20 };
  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;
  
  return LP_OK;
}

int lp_swap_buffers(LP *lp)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;

  if (lp->current_buff == 0)
  {
    lp->current_buff = 1;
    return lp_set_double_buffering_flags(lp, LP_DOUBLE_BUFFERING_DISPLAY_1
                                         | LP_DOUBLE_BUFFERING_UPDATE_0
                                         | LP_DOUBLE_BUFFERING_COPY);
  }
  else
  {
    lp->current_buff = 0;
    return lp_set_double_buffering_flags(lp, LP_DOUBLE_BUFFERING_DISPLAY_0
                                         | LP_DOUBLE_BUFFERING_UPDATE_1
                                         | LP_DOUBLE_BUFFERING_COPY);
  }
}
  
int lp_check_event(LP *lp, LPEvent *event)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_in) return LP_ERROR_UNINITIALIZED;

  unsigned char event_buff[3];
  int err = snd_rawmidi_read(lp->midi_in, event_buff, sizeof(event_buff));
  if (err == -EAGAIN) return 0; // nothing to read
  if (err < 0) return LP_ERROR_MIDI_READ;
  if (err == 3) {
    unsigned char status = event_buff[0];
    unsigned char note   = event_buff[1];
    unsigned char velocity = event_buff[2];
    
    if (event && status == 0x90)
    {
      event->note_x = (note - (note / 16) * 16) % 9;
      event->note_y = note / 16;
      if (velocity > 0)
      {
        event->type = LP_EVENT_PRESSED;
      }
      else
      {
        event->type = LP_EVENT_RELEASED;
      }
    }
    else if (event && status == 0xB0) {
      event->note_x = note - 0x68;
      event->note_y = 0;
      if (velocity > 0)
      {
        event->type = LP_EVENT_AUTOMAP_PRESSED;
      }
      else
      {
        event->type = LP_EVENT_AUTOMAP_RELEASED;
      }
    }

    return 1;
  }
  
  return 0;
}

int lp_enable_flashing(LP *lp)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;

  unsigned char msg_buff[3] = { 0xB0, 0, 0x28 };
  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;

  return LP_OK;
}

int lp_disable_flashing(LP *lp)
{
  if (!lp) return LP_ERROR_LP_NULL;
  if (!lp->midi_out) return LP_ERROR_UNINITIALIZED;

  unsigned char msg_buff[3] = { 0xB0, 0, 0x21 };
  size_t bytes = snd_rawmidi_write(lp->midi_out, msg_buff, sizeof(msg_buff));
  if (bytes != 3) return LP_ERROR_MIDI_WRITE;
  if (snd_rawmidi_drain(lp->midi_out) < 0) return LP_ERROR_MIDI_DRAIN;

  return LP_OK;
}
  
#endif // LIBLAUNCHPAD_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // _LIBLAUNCHPAD_H_
