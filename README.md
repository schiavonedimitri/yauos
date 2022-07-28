YauOS (Yet another unix-like OS) is an hobby operating system i'm developing for educational purposes.   
The project is still in its earliest stages and although not much functionality is yet found on this kernel, i have already added build instructions for those who might want to try and play with this.
You can find them on the [wiki](https://github.com/schiavonedimitri/yauos/wiki/Build-Instructions)
As soon as the project takes shape more info will be added and the wiki updated.

Project goals:
- Being a Unix like clone with Posix compatibily in the future.
- Support for the x86 32 bit architecture only.
- Support for CPUS with integrated x87 FPU only.
- Support for CPUS with SSE2 instructions only. 
- Support for booting only on legacy MBR BIOS systems (no UEFI GPT support).
- Support only for PC-AT platform.
- Support for SMP only with the old MP specification.
- Support for Intel Hyperthreading.
- Being able to boot and work properly on real hardware (although mostly old hardware).
