; ******************************************************************************
; Header: SecureEngineCustomVMs_fasm32.inc
; Description: FASM32 macros definitions
;
; Author/s: Oreans Technologies 
; (c) 2015 Oreans Technologies
;
; --- File generated automatically from Oreans VM Generator (16/6/2015) ---
; ******************************************************************************

; ******************************************************************************
;                                 Macros definition
; ******************************************************************************

macro VM_TIGER_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    100
    dd    0
    dd    20204c57h
}


macro VM_TIGER_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    500
    dd    0
    dd    20204c57h
}


macro VM_TIGER_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    101
    dd    0
    dd    20204c57h
}


macro VM_TIGER_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    501
    dd    0
    dd    20204c57h
}


macro VM_TIGER_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    102
    dd    0
    dd    20204c57h
}


macro VM_TIGER_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    502
    dd    0
    dd    20204c57h
}


macro VM_FISH_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    106
    dd    0
    dd    20204c57h
}


macro VM_FISH_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    506
    dd    0
    dd    20204c57h
}


macro VM_FISH_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    108
    dd    0
    dd    20204c57h
}


macro VM_FISH_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    508
    dd    0
    dd    20204c57h
}


macro VM_FISH_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    110
    dd    0
    dd    20204c57h
}


macro VM_FISH_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    510
    dd    0
    dd    20204c57h
}


macro VM_PUMA_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    112
    dd    0
    dd    20204c57h
}


macro VM_PUMA_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    512
    dd    0
    dd    20204c57h
}


macro VM_PUMA_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    114
    dd    0
    dd    20204c57h
}


macro VM_PUMA_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    514
    dd    0
    dd    20204c57h
}


macro VM_PUMA_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    116
    dd    0
    dd    20204c57h
}


macro VM_PUMA_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    516
    dd    0
    dd    20204c57h
}


macro VM_SHARK_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    118
    dd    0
    dd    20204c57h
}


macro VM_SHARK_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    518
    dd    0
    dd    20204c57h
}


macro VM_SHARK_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    120
    dd    0
    dd    20204c57h
}


macro VM_SHARK_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    520
    dd    0
    dd    20204c57h
}


macro VM_SHARK_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    122
    dd    0
    dd    20204c57h
}


macro VM_SHARK_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    522
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    134
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    534
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    136
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    536
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    138
    dd    0
    dd    20204c57h
}


macro VM_DOLPHIN_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    538
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_WHITE_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    146
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_WHITE_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    546
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_RED_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    148
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_RED_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    548
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_BLACK_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    150
    dd    0
    dd    20204c57h
}


macro VM_EAGLE_BLACK_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    550
    dd    0
    dd    20204c57h
}


macro VM_MUTATE_ONLY_START
{
    db    0ebh, 10h
    dd    20204c57h
    dd    16
    dd    0
    dd    20204c57h
}


macro VM_MUTATE_ONLY_END
{
    db    0ebh, 10h
    dd    20204c57h
    dd    17
    dd    0
    dd    20204c57h
}


