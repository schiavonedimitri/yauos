.section .text 

.global enable_interrupts
.type enable_interrupts, @function
    
    enable_interrupts:
    sti
    ret
    .size enable_interrupts, . - enable_interrupts

.global disable_interrupts
.type disable_interrupts, @function
    
    disable_interrupts:
    cli
    ret
    .size disable_interrupts, . - disable_interrupts