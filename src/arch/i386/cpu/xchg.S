.section .text
        .globl _xchg
        .type _xchg, @function
        _xchg:
                movl 4(%esp), %eax
                movl 8(%esp), %edx
                xchgl %eax, (%edx)
                ret                
        .size _xchg, . - _xchg