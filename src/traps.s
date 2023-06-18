# 1 "src/traps.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/traps.S"
.extern _ZN5Riscv20handleSupervisorTrapEv
.extern _ZN5Riscv33handleSupervisorSoftwareInterruptEv
.extern _ZN5Riscv33handleSupervisorExternalInterruptEv
.extern _ZN5Riscv16handleExceptionsEv
.extern _ZN6Thread10getContextEv

.global _ZN5Riscv31trapSupervisorSoftwareInterruptEv
.type _ZN5Riscv31trapSupervisorSoftwareInterruptEv, @function
_ZN5Riscv31trapSupervisorSoftwareInterruptEv:


    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN6Thread10getContextEv



    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)
    ld ra, 1 * 8(sp)
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv33handleSupervisorSoftwareInterruptEv

    call _ZN6Thread10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)

    sret

.global _ZN5Riscv31trapSupervisorExternalInterruptEv
.type _ZN5Riscv31trapSupervisorExternalInterruptEv, @function
_ZN5Riscv31trapSupervisorExternalInterruptEv:

    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN6Thread10getContextEv



    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)
    ld ra, 1 * 8(sp)
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv33handleSupervisorExternalInterruptEv

    call _ZN6Thread10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)

    sret

.global _ZN5Riscv14trapExceptionsEv
.type _ZN5Riscv14trapExceptionsEv, @function
_ZN5Riscv14trapExceptionsEv:

    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN6Thread10getContextEv



    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)
    ld ra, 1 * 8(sp)
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv16handleExceptionsEv

    call _ZN6Thread10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)

    sret
