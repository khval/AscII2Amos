VERSION = 1
REVISION = 2

.macro DATE
.ascii "29.1.2018"
.endm

.macro VERS
.ascii "ascii2amos.exe 1.2"
.endm

.macro VSTRING
.ascii "ascii2amos.exe 1.2 (29.1.2018)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: ascii2amos.exe 1.2 (29.1.2018)"
.byte 0
.endm
