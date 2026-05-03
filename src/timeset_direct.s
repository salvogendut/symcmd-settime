.code

; Direct call to Device_TimeSet (#810F)
; void Time_Set_Direct(unsigned char sec, unsigned char min, unsigned char hour,
;                      unsigned char day, unsigned char month, unsigned short year,
;                      signed char tz);
.export _Time_Set_Direct
_Time_Set_Direct:
    push ix
    push iy
    ld ix,2
    add ix,sp

    ; Stack layout after "push ix; push iy; ld ix,2; add ix,sp":
    ; IMPORTANT: char parameters are promoted to int (2 bytes) in C!
    ; SP+0  = saved IY (2 bytes)
    ; SP+2  = saved IX (2 bytes)  <- IX points here
    ; SP+4  = return address (2 bytes)
    ; SP+6  = parameter 1: sec (2 bytes, low byte at +6)
    ; SP+8  = parameter 2: min (2 bytes, low byte at +8)
    ; SP+10 = parameter 3: hour (2 bytes, low byte at +10)
    ; SP+12 = parameter 4: day (2 bytes, low byte at +12)
    ; SP+14 = parameter 5: month (2 bytes, low byte at +14)
    ; SP+16 = parameter 6: year (2 bytes, already a short)
    ; SP+18 = parameter 7: timezone (2 bytes, low byte at +18)

    ; Load parameters from stack into correct registers for syscall
    ; Device_TimeSet expects: A=sec, B=min, C=hour, D=day, E=month, HL=year, IXL=tz

    ld b,(ix+6)      ; min (low byte of 2-byte int)
    ld c,(ix+8)      ; hour (low byte of 2-byte int)
    ld d,(ix+10)     ; day (low byte of 2-byte int)
    ld e,(ix+12)     ; month (low byte of 2-byte int)
    ld l,(ix+14)     ; year low
    ld h,(ix+15)     ; year high

    ; Load timezone into IXL using the DD prefix trick
    push hl          ; save year
    ld a,(ix+16)     ; timezone (low byte of 2-byte int)
    .byte #0xDD
    ld l,a           ; ld ixl,a
    pop hl           ; restore year

    ; Finally load second into A (must be last since we used A for timezone)
    ld a,(ix+4)      ; sec (low byte of 2-byte int)

    ; Call Device_TimeSet
    rst #0x20
    .word #0x810F

    pop iy
    pop ix
    ret
