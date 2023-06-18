# 1 "src/vectorTable.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/vectorTable.S"
.extern _ZN5Riscv14supervisorTrapEv
.extern _ZN5Riscv31trapSupervisorSoftwareInterruptEv
.extern _ZN5Riscv31trapSupervisorExternalInterruptEv
.extern _ZN5Riscv14trapExceptionsEv


.align 4
.global _ZN5Riscv11vectorTableEv
.type _ZN5Riscv11vectorTableEv, @function
_ZN5Riscv11vectorTableEv:
    .org _ZN5Riscv11vectorTableEv + 0*4
    jal zero, _ZN5Riscv14trapExceptionsEv
    .org _ZN5Riscv11vectorTableEv + 1*4
    jal zero, _ZN5Riscv31trapSupervisorSoftwareInterruptEv
    .org _ZN5Riscv11vectorTableEv + 9*4
    jal zero, _ZN5Riscv31trapSupervisorExternalInterruptEv
