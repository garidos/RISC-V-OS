.extern _ZN5Riscv20handleSupervisorTrapEv
.extern _ZN5Riscv33handleSupervisorSoftwareInterruptEv
.extern _ZN5Riscv33handleSupervisorExternalInterruptEv
.extern _ZN5Riscv16handleExceptionsEv
.extern _ZN3TCB10getContextEv

.global _ZN5Riscv31trapSupervisorSoftwareInterruptEv
.type _ZN5Riscv31trapSupervisorSoftwareInterruptEv, @function
_ZN5Riscv31trapSupervisorSoftwareInterruptEv:

    //cuvaju se a0 i ra na steku jer ce se oni promjeniti pri pozivu getContext
    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN3TCB10getContextEv

    //sada je u a0 pokazivac na context

    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)    //cuva se a0
    ld ra, 1 * 8(sp)    //u ra se vraca ra
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //cuvaju se svi registri osim a0 koji je vec sacuvan
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv33handleSupervisorSoftwareInterruptEv

    call _ZN3TCB10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //restauriraju se svi registri osim a0 u kom je pokazivac na context
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)

    sret

.global _ZN5Riscv31trapSupervisorExternalInterruptEv
.type _ZN5Riscv31trapSupervisorExternalInterruptEv, @function
_ZN5Riscv31trapSupervisorExternalInterruptEv:
    //cuvaju se a0 i ra na steku jer ce se oni promjeniti pri pozivu getContext
    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN3TCB10getContextEv

    //sada je u a0 pokazivac na context

    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)    //cuva se a0
    ld ra, 1 * 8(sp)    //u ra se vraca ra
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //cuvaju se svi registri osim a0 koji je vec sacuvan
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv33handleSupervisorExternalInterruptEv

    call _ZN3TCB10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //restauriraju se svi registri osim a0 u kom je pokazivac na context
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)

    sret

.global _ZN5Riscv14trapExceptionsEv
.type _ZN5Riscv14trapExceptionsEv, @function
_ZN5Riscv14trapExceptionsEv:
    //cuvaju se a0 i ra na steku jer ce se oni promjeniti pri pozivu getContext
    addi sp, sp, -16
    sd a0, 0 * 8(sp)
    sd ra, 1 * 8(sp)

    call _ZN3TCB10getContextEv

    //sada je u a0 pokazivac na context

    ld ra, 0 * 8(sp)
    sd ra, 10 * 8(a0)    //cuva se a0
    ld ra, 1 * 8(sp)    //u ra se vraca ra
    addi sp, sp, 16

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //cuvaju se svi registri osim a0 koji je vec sacuvan
    sd x\index, \index * 8(a0)
    .endr

    call _ZN5Riscv16handleExceptionsEv

    call _ZN3TCB10getContextEv

    .irp index, 1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 //restauriraju se svi registri osim a0 u kom je pokazivac na context
    ld x\index, \index * 8(a0)
    .endr

    ld a0, 10 * 8(a0)
    //treb i a0 jer sam povratnu vrijednost upisao u postojeci kontekst

    sret
