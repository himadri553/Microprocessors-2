#include <avr/io.h>

.global set_red_leds_high
.type set_red_leds_high, @function

set_red_leds_high:
    ; --- Set pins 48 (PL1) and 49 (PL0) as outputs ---
    lds r24, DDRL        ; load DDRL into r24
    ori r24, (1 << 0)    ; set bit 0 (pin 49)
    ori r24, (1 << 1)    ; set bit 1 (pin 48)
    sts DDRL, r24        ; store back

    ; --- Drive both pins HIGH ---
    lds r24, PORTL       ; load PORTL
    ori r24, (1 << 0)    ; set bit 0
    ori r24, (1 << 1)    ; set bit 1
    sts PORTL, r24       ; store back

    ret
