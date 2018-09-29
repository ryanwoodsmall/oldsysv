# @(#)scb.s	6.2
# System control block
#
	.text
	.set	ISTK,1		# handle this interrupt on the interrupt stack
	.align	512
	.globl	Scbbase
Scbbase:
	.long	Xrandom + 0x000
	.long	Xmacheck + ISTK	# machine check interrupt
	.long	Xkspinv + ISTK	# kernal stack not valid
	.long	Xpowfail + ISTK	# power fail
	.long	Xprivinflt	# privileged instruction 
	.long	Xxfcflt		# xfc instruction 
	.long	Xresopflt	# reserved operand 
	.long	Xresadflt	# reserved addressing 
	.long	Xprotflt	# protection 
	.long	Xvalflt		# translation not valid 
	.long	Xtracep		# trace pending
	.long	Xbptflt		# bpt instruction
	.long	Xcompatflt	# compatibility mode fault
	.long	Xarithtrap	# arithmetic trap
	.long	Xrandom + 0x038
	.long	Xrandom + 0x03c
	.long	Xsyscall	# chmk
	.long	Xchme		# chme
	.long	Xchms		# chms
	.long	Xchmu		# chmu
	.long	Xsbi + 0x00 + ISTK	# SBI silo compare
	.long	Xcrdrds + ISTK		# CRD/RDS
	.long	Xsbi + 0x08 + ISTK	# SBI alert
	.long	Xsbi + 0x0c + ISTK	# SBI fault
	.long	Xmwt + ISTK		# Memory Write Timeout
	.long	Xrandom + 0x064
	.long	Xrandom + 0x068
	.long	Xrandom + 0x06c
	.long	Xrandom + 0x070
	.long	Xrandom + 0x074
	.long	Xrandom + 0x078
	.long	Xrandom + 0x07c
	.long	Xrandom + 0x080
# software interrupts
	.long	Xquiescent
	.long	Xasttrap
	.long	Xresched	# reschedule nudge
	.long	Xrandom + 0x090
	.long	Xrandom + 0x094
	.long	Xrandom + 0x098
	.long	Xrandom + 0x09c
	.long	Xrandom + 0x0a0
	.long	Xrandom + 0x0a4
	.long	Xrandom + 0x0a8
	.long	Xrandom + 0x0ac
	.long	Xrandom + 0x0b0
	.long	Xrandom + 0x0b4
	.long	Xrandom + 0x0b8
	.long	Xtimein + ISTK	# timeout function
	.long	Xclock + ISTK	# clock
	.long	Xrandom + 0x0c4
	.long	Xrandom + 0x0c8
	.long	Xrandom + 0x0cc
	.long	Xrandom + 0x0d0
	.long	Xrandom + 0x0d4
	.long	Xrandom + 0x0d8
	.long	Xrandom + 0x0dc
	.long	Xrandom + 0x0e0
	.long	Xrandom + 0x0e4
	.long	Xrandom + 0x0e8
	.long	Xrandom + 0x0ec
	.long	Xcdtrint + ISTK # console DECtape receiver
	.long	Xcdtxint + ISTK # console DECtape transmitter
	.long	Xconrint + ISTK	# console receiver 
	.long	Xconxint + ISTK	# console transmitter

#	I/O vectors

# IPL 14
	.long	Xrandom + 0x100
	.long	Xrandom + 0x104
	.long	Xrandom + 0x108
	.long	Xrandom + 0x10c
#	.long	Xubaint		# UBA br4
	.long	Xrandom + 0x110
	.long	Xrandom + 0x114
	.long	Xrandom + 0x118
	.long	Xrandom + 0x11c
	.long	Xrandom + 0x120
	.long	Xrandom + 0x124
	.long	Xrandom + 0x128
	.long	Xrandom + 0x12c
	.long	Xrandom + 0x130
	.long	Xrandom + 0x134
	.long	Xrandom + 0x138
	.long	Xrandom + 0x13c

# IPL 15
	.long	Xrandom + 0x140
	.long	Xrandom + 0x144
	.long	Xrandom + 0x148
	.long	Xrandom + 0x14c
#	.long	Xubaint		# UBA br5
	.long	Xrandom + 0x150
	.long	Xrandom + 0x154
	.long	Xrandom + 0x158
	.long	Xrandom + 0x15c
	.long	Xrandom + 0x160
	.long	Xrandom + 0x164
	.long	Xrandom + 0x168
	.long	Xrandom + 0x16c
	.long	Xrandom + 0x170
	.long	Xrandom + 0x174
	.long	Xrandom + 0x178
	.long	Xrandom + 0x17c

# IPL 16
	.long	Xrandom + 0x180
	.long	Xrandom + 0x184
	.long	Xrandom + 0x188
	.long	Xrandom + 0x18c
#	.long	Xubaint		# UBA br6
	.long	Xrandom + 0x190
	.long	Xrandom + 0x194
	.long	Xrandom + 0x198
	.long	Xrandom + 0x19c
	.long	Xrandom + 0x1a0
	.long	Xrandom + 0x1a4
	.long	Xrandom + 0x1a8
	.long	Xrandom + 0x1ac
	.long	Xrandom + 0x1b0
	.long	Xrandom + 0x1b4
	.long	Xrandom + 0x1b8
	.long	Xrandom + 0x1bc

# IPL 17
	.long	Xrandom + 0x1c0
	.long	Xrandom + 0x1c4
	.long	Xrandom + 0x1c8
	.long	Xrandom + 0x1cc
	.long	Xrandom + 0x1d0
	.long	Xrandom + 0x1d4
	.long	Xrandom + 0x1d8
	.long	Xrandom + 0x1dc
	.long	Xrandom + 0x1e0
	.long	Xrandom + 0x1e4
	.long	Xrandom + 0x1e8
	.long	Xrandom + 0x1ec
	.long	Xrandom + 0x1f0
	.long	Xrandom + 0x1f4
	.long	Xrandom + 0x1f8
	.long	Xrandom + 0x1fc

# uni-bus direct vectors
	.long	Xunibus + 0x000 + ISTK
	.long	Xunibus + 0x004 + ISTK
	.long	Xunibus + 0x008 + ISTK
	.long	Xunibus + 0x00c + ISTK
	.long	Xunibus + 0x010 + ISTK
	.long	Xunibus + 0x014 + ISTK
	.long	Xunibus + 0x018 + ISTK
	.long	Xunibus + 0x01c + ISTK
	.long	Xunibus + 0x020 + ISTK
	.long	Xunibus + 0x024 + ISTK
	.long	Xunibus + 0x028 + ISTK
	.long	Xunibus + 0x02c + ISTK
	.long	Xunibus + 0x030 + ISTK
	.long	Xunibus + 0x034 + ISTK
	.long	Xunibus + 0x038 + ISTK
	.long	Xunibus + 0x03c + ISTK
	.long	Xunibus + 0x040 + ISTK
	.long	Xunibus + 0x044 + ISTK
	.long	Xunibus + 0x048 + ISTK
	.long	Xunibus + 0x04c + ISTK
	.long	Xunibus + 0x050 + ISTK
	.long	Xunibus + 0x054 + ISTK
	.long	Xunibus + 0x058 + ISTK
	.long	Xunibus + 0x05c + ISTK
	.long	Xunibus + 0x060 + ISTK
	.long	Xunibus + 0x064 + ISTK
	.long	Xunibus + 0x068 + ISTK
	.long	Xunibus + 0x06c + ISTK
	.long	Xunibus + 0x070 + ISTK
	.long	Xunibus + 0x074 + ISTK
	.long	Xunibus + 0x078 + ISTK
	.long	Xunibus + 0x07c + ISTK
	.long	Xunibus + 0x080 + ISTK
	.long	Xunibus + 0x084 + ISTK
	.long	Xunibus + 0x088 + ISTK
	.long	Xunibus + 0x08c + ISTK
	.long	Xunibus + 0x090 + ISTK
	.long	Xunibus + 0x094 + ISTK
	.long	Xunibus + 0x098 + ISTK
	.long	Xunibus + 0x09c + ISTK
	.long	Xunibus + 0x0a0 + ISTK
	.long	Xunibus + 0x0a4 + ISTK
	.long	Xunibus + 0x0a8 + ISTK
	.long	Xunibus + 0x0ac + ISTK
	.long	Xunibus + 0x0b0 + ISTK
	.long	Xunibus + 0x0b4 + ISTK
	.long	Xunibus + 0x0b8 + ISTK
	.long	Xunibus + 0x0bc + ISTK
	.long	Xunibus + 0x0c0 + ISTK
	.long	Xunibus + 0x0c4 + ISTK
	.long	Xunibus + 0x0c8 + ISTK
	.long	Xunibus + 0x0cc + ISTK
	.long	Xunibus + 0x0d0 + ISTK
	.long	Xunibus + 0x0d4 + ISTK
	.long	Xunibus + 0x0d8 + ISTK
	.long	Xunibus + 0x0dc + ISTK
	.long	Xunibus + 0x0e0 + ISTK
	.long	Xunibus + 0x0e4 + ISTK
	.long	Xunibus + 0x0e8 + ISTK
	.long	Xunibus + 0x0ec + ISTK
	.long	Xunibus + 0x0f0 + ISTK
	.long	Xunibus + 0x0f4 + ISTK
	.long	Xunibus + 0x0f8 + ISTK
	.long	Xunibus + 0x0fc + ISTK
	.long	Xunibus + 0x100 + ISTK
	.long	Xunibus + 0x104 + ISTK
	.long	Xunibus + 0x108 + ISTK
	.long	Xunibus + 0x10c + ISTK
	.long	Xunibus + 0x110 + ISTK
	.long	Xunibus + 0x114 + ISTK
	.long	Xunibus + 0x118 + ISTK
	.long	Xunibus + 0x11c + ISTK
	.long	Xunibus + 0x120 + ISTK
	.long	Xunibus + 0x124 + ISTK
	.long	Xunibus + 0x128 + ISTK
	.long	Xunibus + 0x12c + ISTK
	.long	Xunibus + 0x130 + ISTK
	.long	Xunibus + 0x134 + ISTK
	.long	Xunibus + 0x138 + ISTK
	.long	Xunibus + 0x13c + ISTK
	.long	Xunibus + 0x140 + ISTK
	.long	Xunibus + 0x144 + ISTK
	.long	Xunibus + 0x148 + ISTK
	.long	Xunibus + 0x14c + ISTK
	.long	Xunibus + 0x150 + ISTK
	.long	Xunibus + 0x154 + ISTK
	.long	Xunibus + 0x158 + ISTK
	.long	Xunibus + 0x15c + ISTK
	.long	Xunibus + 0x160 + ISTK
	.long	Xunibus + 0x164 + ISTK
	.long	Xunibus + 0x168 + ISTK
	.long	Xunibus + 0x16c + ISTK
	.long	Xunibus + 0x170 + ISTK
	.long	Xunibus + 0x174 + ISTK
	.long	Xunibus + 0x178 + ISTK
	.long	Xunibus + 0x17c + ISTK
	.long	Xunibus + 0x180 + ISTK
	.long	Xunibus + 0x184 + ISTK
	.long	Xunibus + 0x188 + ISTK
	.long	Xunibus + 0x18c + ISTK
	.long	Xunibus + 0x190 + ISTK
	.long	Xunibus + 0x194 + ISTK
	.long	Xunibus + 0x198 + ISTK
	.long	Xunibus + 0x19c + ISTK
	.long	Xunibus + 0x1a0 + ISTK
	.long	Xunibus + 0x1a4 + ISTK
	.long	Xunibus + 0x1a8 + ISTK
	.long	Xunibus + 0x1ac + ISTK
	.long	Xunibus + 0x1b0 + ISTK
	.long	Xunibus + 0x1b4 + ISTK
	.long	Xunibus + 0x1b8 + ISTK
	.long	Xunibus + 0x1bc + ISTK
	.long	Xunibus + 0x1c0 + ISTK
	.long	Xunibus + 0x1c4 + ISTK
	.long	Xunibus + 0x1c8 + ISTK
	.long	Xunibus + 0x1cc + ISTK
	.long	Xunibus + 0x1d0 + ISTK
	.long	Xunibus + 0x1d4 + ISTK
	.long	Xunibus + 0x1d8 + ISTK
	.long	Xunibus + 0x1dc + ISTK
	.long	Xunibus + 0x1e0 + ISTK
	.long	Xunibus + 0x1e4 + ISTK
	.long	Xunibus + 0x1e8 + ISTK
	.long	Xunibus + 0x1ec + ISTK
	.long	Xunibus + 0x1f0 + ISTK
	.long	Xunibus + 0x1f4 + ISTK
	.long	Xunibus + 0x1f8 + ISTK
	.long	Xunibus + 0x1fc + ISTK
