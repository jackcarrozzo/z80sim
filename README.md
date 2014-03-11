z80sim
======

Fork of z80sim from z80pack-1.17

Changes so far:
- Interrupt handler (only Mode 1 and NMI were supported before)
- Bug with CLI during interrupts
- Added standard z84 family periphery io devices to match my hardware
- Changed file names to be meaninful
- Added support for loading flat binary memory files, and fixed filetype detection

TODO:
- Add flag to exit after halt
- Add configurable debug prints for various emulated chips
- Make CTC clock counting accurate
