# 1 "src/contextSwitch.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/contextSwitch.S"
.global _ZN6Thread13contextSwitchEPmS0_
.type _ZN6Thread13contextSwitchEPmS0_, @function
_ZN6Thread13contextSwitchEPmS0_:
    sd ra, 0 * 8(a0)
    sd sp, 32 * 8(a0)

    ld ra, 0 * 8(a1)
    ld sp, 32 * 8(a1)

    ret
