/*
 * start.S
 *
 * Startup code
 *
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

.section .text.init
.global _start
_start:
  la t0, __bss_start
  la t1, __bss_end
  bgeu t0, t1, .bss_zero_loop_end
.bss_zero_loop:
  sb zero, (t0)
  addi t0, t0, 1
  bltu t0, t1, .bss_zero_loop
.bss_zero_loop_end:

  la sp, stack_top
  jal main
.infinite_loop:
  j .infinite_loop

.global	_exit
_exit:
	j _exit

.section .rawdata
.align 16
.global _binary_doom1_wad_start
_binary_doom1_wad_start:
  .incbin "doom1.wad"

.section .bss
.align 4
  .skip 0x1000
stack_top:
  .skip 0x4
