.global _ZN6Thread13contextSwitchEPmS0_
.type _ZN6Thread13contextSwitchEPmS0_, @function
_ZN6Thread13contextSwitchEPmS0_:
    sd ra, 0 * 8(a0)
    sd sp, 32 * 8(a0)

    ld ra, 0 * 8(a1)
    ld sp, 32 * 8(a1)

    ret

    //cuva sra i stack trenutnog procesa, a ucitava sra i stack novog