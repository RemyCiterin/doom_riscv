/*
 * i_system.c
 *
 * System support code
 *
 * Copyright (C) 1993-1996 by id Software, Inc.
 * Copyright (C) 2021 Sylvain Munaut
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "doomdef.h"
#include "doomstat.h"

#include "d_main.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_video.h"

#include "i_system.h"

#include "console.h"
#include "config.h"


/* Video controller, used as a time base */
	/* Normally running at 70 Hz, although in 640x480 compat
	 * mode, it's 60 Hz so our tick is 15% too slow ... */
static volatile uint32_t * const video_state = (void*)(VID_CTRL_BASE);

/* Video Ticks tracking */
static uint16_t vt_last = 0;
static uint32_t vt_base = 0;


void
I_Init(void)
{
	vt_last = video_state[0] & 0xffff;
}


byte *
I_ZoneBase(int *size)
{
	/* Give 6M to DOOM */
	*size = 6 * 1024 * 1024;
	return (byte *) malloc (*size);
}


int
I_GetTime(void)
{
  uint32_t time;
  asm volatile ("csrr %[time], mcycle" : [time]"=r"(time));
  return (time * 32) / 1000000;

	// uint16_t vt_now = video_state[0] & 0xffff;

	// if (vt_now < vt_last)
	// 	vt_base += 65536;
	// vt_last = vt_now;

	// /* TIC_RATE is 35 in theory */
	// return (vt_base + vt_now) >> 1;
}

static int print_count = 0;

static uint8_t prev_btn = 0;
static volatile uint8_t* btn_ptr = (volatile uint8_t*)0x20000000;

static void I_GetRemoteEvent(void) {
  event_t event;

  uint8_t btn = *btn_ptr;

  if ((btn & 2) && (btn & 4)) {
    btn |= 64;
    btn &= ~4;
  }

  if ((btn & 2) && (btn & 8)) {
    btn |= 128;
    btn &= ~8;
  }

  btn &= ~2;

  uint8_t press_btn = btn & ~prev_btn;
  uint8_t release_btn = prev_btn & ~btn;
  prev_btn = btn;

  while (press_btn) {
    event.type = ev_keydown;

    if (press_btn & 1) {
      event.data1 = KEY_RCTRL;
      press_btn &= ~1;
    } else if (press_btn & 4) {
      event.data1 = KEY_UPARROW;
      press_btn &= ~4;
    } else if (press_btn & 8) {
      event.data1 = KEY_DOWNARROW;
      press_btn &= ~8;
    } else if (press_btn & 16) {
      event.data1 = KEY_LEFTARROW;
      press_btn &= ~16;
    } else if (press_btn & 32) {
      event.data1 = KEY_RIGHTARROW;
      press_btn &= ~32;
    } else if (press_btn & 64) {
      event.data1 = KEY_ENTER;
      press_btn &= ~64;
    } else if (press_btn & 128) {
      event.data1 = ' ';
      press_btn &= ~128;
    }

    D_PostEvent(&event);
  }

  while (release_btn) {
    event.type = ev_keyup;

    if (release_btn & 1) {
      event.data1 = KEY_RCTRL;
      release_btn &= ~1;
    } else if (release_btn & 4) {
      event.data1 = KEY_UPARROW;
      release_btn &= ~4;
    } else if (release_btn & 8) {
      event.data1 = KEY_DOWNARROW;
      release_btn &= ~8;
    } else if (release_btn & 16) {
      event.data1 = KEY_LEFTARROW;
      release_btn &= ~16;
    } else if (release_btn & 32) {
      event.data1 = KEY_RIGHTARROW;
      release_btn &= ~32;
    } else if (release_btn & 64) {
      event.data1 = KEY_ENTER;
      release_btn &= ~64;
    } else if (release_btn & 128) {
      event.data1 = ' ';
      release_btn &= ~128;
    }

    D_PostEvent(&event);
  }


}

static void
I_GetRemoteEvent2(void)
{
	event_t event;

	const char map[] = {
		KEY_LEFTARROW,  // 0
		KEY_RIGHTARROW, // 1
		KEY_DOWNARROW,  // 2
		KEY_UPARROW,    // 3
		KEY_RSHIFT,     // 4
		KEY_RCTRL,      // 5
		KEY_RALT,       // 6
		KEY_ESCAPE,     // 7
		KEY_ENTER,      // 8
		KEY_TAB,        // 9
		KEY_BACKSPACE,  // 10
		KEY_PAUSE,      // 11
		KEY_EQUALS,     // 12
		KEY_MINUS,      // 13
		KEY_F1,         // 14
		KEY_F2,         // 15
		KEY_F3,         // 16
		KEY_F4,         // 17
		KEY_F5,         // 18
		KEY_F6,         // 19
		KEY_F7,         // 20
		KEY_F8,         // 21
		KEY_F9,         // 22
		KEY_F10,        // 23
		KEY_F11,        // 24
		KEY_F12,        // 25
	};

	static byte s_btn = 0;

	boolean mupd = false;
	int mdx = 0;
	int mdy = 0;
  int oldch = 0;

	while (1) {
		int ch = console_getchar_nowait();
		if (ch == -1)
			break;
    if (ch == 0)
      break;
    if (ch == oldch)
      break;
    oldch = ch;

    print_count++;
    if (print_count % 20 == 0)
      printf("read event: %c\n", ch);

		boolean msb = ch & 0x80;
		ch &= 0x7f;

		if (ch < 28) {
			/* Keyboard special */
			event.type = msb ? ev_keydown : ev_keyup;
			event.data1 = map[ch];
			D_PostEvent(&event);
		} else if (ch < 31) {
			/* Mouse buttons */
			if (msb)
				s_btn |= (1 << ((ch & 0x7f) - 28));
			else
				s_btn &= ~(1 << ((ch & 0x7f) - 28));
			mupd = true;
		} else if (ch == 0x1f) {
			/* Mouse movement */
			signed char x = console_getchar();
			signed char y = console_getchar();
			mdx += x;
			mdy += y;
			mupd = true;
		} else {
			/* Keyboard normal */
			event.type = msb ? ev_keydown : ev_keyup;
			event.data1 = ch;
			D_PostEvent(&event);
		}
	}

	if (mupd) {
		event.type = ev_mouse;
		event.data1 = s_btn;
		event.data2 =   mdx << 2;
		event.data3 = - mdy << 2;	/* Doom is sort of inverted ... */
		D_PostEvent(&event);
	}
}

void
I_StartFrame(void)
{
	/* Nothing to do */
}

void
I_StartTic(void)
{
	I_GetRemoteEvent();
}

ticcmd_t *
I_BaseTiccmd(void)
{
	static ticcmd_t emptycmd;
	return &emptycmd;
}


void
I_Quit(void)
{
	D_QuitNetGame();
	M_SaveDefaults();
	I_ShutdownGraphics();
	exit(0);
}


byte *
I_AllocLow(int length)
{
	byte*	mem;
	mem = (byte *)malloc (length);
	memset (mem,0,length);
	return mem;
}


void
I_Tactile
( int on,
  int off,
  int total )
{
	// UNUSED.
	on = off = total = 0;
}


void
I_Error(char *error, ...)
{
	va_list	argptr;

	// Message first.
	va_start (argptr,error);
	fprintf (stderr, "Error: ");
	vfprintf (stderr,error,argptr);
	fprintf (stderr, "\n");
	va_end (argptr);

	fflush( stderr );

	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();

	D_QuitNetGame ();
	I_ShutdownGraphics();

	exit(-1);
}
