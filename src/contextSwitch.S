.global _ZN3TCB13contextSwitchEPmS0_
.type _ZN3TCB13contextSwitchEPmS0_, @function
_ZN3TCB13contextSwitchEPmS0_:
    sd ra, 0 * 8(a0)
    sd sp, 32 * 8(a0)

    ld ra, 0 * 8(a1)
    ld sp, 32 * 8(a1)

    ret

    //cuva sra i ssp trenutnog procesa, a ucitava sra i ssp novog

    //slicno kao kod sa vjezbi