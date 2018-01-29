VERSION		EQU	1
REVISION	EQU	2

DATE	MACRO
		dc.b '29.1.2018'
		ENDM

VERS	MACRO
		dc.b 'ascii2amos.exe 1.2'
		ENDM

VSTRING	MACRO
		dc.b 'ascii2amos.exe 1.2 (29.1.2018)',13,10,0
		ENDM

VERSTAG	MACRO
		dc.b 0,'$VER: ascii2amos.exe 1.2 (29.1.2018)',0
		ENDM
