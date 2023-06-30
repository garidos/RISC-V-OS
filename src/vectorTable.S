.extern _ZN5Riscv14supervisorTrapEv
.extern _ZN5Riscv31trapSupervisorSoftwareInterruptEv
.extern _ZN5Riscv31trapSupervisorExternalInterruptEv
.extern _ZN5Riscv14trapExceptionsEv


.align 4
.global _ZN5Riscv11vectorTableEv
.type _ZN5Riscv11vectorTableEv, @function
_ZN5Riscv11vectorTableEv:
    j _ZN5Riscv14trapExceptionsEv                       // nulti ulaz
    j _ZN5Riscv31trapSupervisorSoftwareInterruptEv      // prvi ulaz
    .org _ZN5Riscv11vectorTableEv + 9*4
    j _ZN5Riscv31trapSupervisorExternalInterruptEv      // deveti ulaz

    //za izuzetke se uzima BASE ( nulti ulaz, odnosno adresa koja se nalazi u stvec - adresa ove tabele ), a za prekide se uzima BASE + 4 * kod prekida
    //adresa mora biti poravnata na 4, jer se prva dva bita koriste za mod - 0 = direktno ( svi prekidi idu na BASE ), 1 - vektorski ( ovo iznad )