# Stack backtrace code, borrowed from the Osdev wiki:
# https://wiki.osdev.org/Stack_Trace
# Walks backwards through the call stack and builds a list of return addresses.
# Args:
#  * Array of 32-bit addresses.
#  * Maximum number of elements in array.
# Return value: The number of addresses stored in the array.
# Calling convention: cdecl

.section .text

.globl walk_stack
.type walk_stack, @function

walk_stack:
    
    # Create stack frame & save caller's EDI and EBX.
    
    push %ebp
    movl  %esp, %ebp
    subl  $8, %esp
    movl  %edi, -4(%ebp)
    movl  %ebx, -8(%ebp)
    
    # Set up local registers.
    # EAX = return value (number of stack frames found).
    
    xorl  %eax, %eax
    
    # EBX = old EBP.  
    
    movl 8(%esp), %ebx

    # Destination array pointer in EDI.

    movl 16(%esp), %edi
    
    # Maximum array size in ECX.
    
    movl 20(%esp), %ecx

.walk:

    # Walk backwards through EBP linked list, storing return addresses in EDI array.
    
    testl %ebx, %ebx
    jz   .done
    
    # EDX = previous stack frame's IP.    
    
    movl 4(%ebx), %edx

    # EBX = previous stack frame's BP.

    movl (%ebx), %ebx
    
    # Copy IP.
    
    movl  %edx, (%edi)
    addl  $4, %edi
    incl  %eax
    loop .walk
.done:
    
    # Restore caller's EDI and EBX, leave stack frame & return EAX.
    
    movl  -4(%ebp), %edi
    movl  -8(%ebp), %ebx
    leave
    ret
.size walk_stack, . - walk_stack