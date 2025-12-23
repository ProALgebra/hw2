	.file "/home/doctor/VM/test/hw2/performance/Sort.lama"

	.stabs "/home/doctor/VM/test/hw2/performance/Sort.lama",100,0,0,.Ltext

	.globl	main

	.data

string_0:	.string	"Function %s called with incorrect arguments count. Expected: %d. Actual: %d\n"

string_3:	.string	"Sort.lama"

string_6:	.string	"bubbleSort"

string_1:	.string	"bubbleSort22323"

string_5:	.string	"generate"

string_8:	.string	"inner_25"

string_4:	.string	"inner_3"

string_7:	.string	"rec_25"

string_2:	.string	"rec_3"

init:	.quad 0

	.section custom_data,"aw",@progbits

filler:	.fill	7, 8, 1

	.text

.Ltext:

	.stabs "data:t1=r1;0;4294967295;",128,0,0,0

# IMPORT ("Std")

# PUBLIC ("main")

# EXTERN ("Llowercase")

# EXTERN ("Luppercase")

# EXTERN ("LtagHash")

# EXTERN ("LflatCompare")

# EXTERN ("LcompareTags")

# EXTERN ("LkindOf")

# EXTERN ("Ltime")

# EXTERN ("Lrandom")

# EXTERN ("LdisableGC")

# EXTERN ("LenableGC")

# EXTERN ("Ls__Infix_37")

# EXTERN ("Ls__Infix_47")

# EXTERN ("Ls__Infix_42")

# EXTERN ("Ls__Infix_45")

# EXTERN ("Ls__Infix_43")

# EXTERN ("Ls__Infix_62")

# EXTERN ("Ls__Infix_6261")

# EXTERN ("Ls__Infix_60")

# EXTERN ("Ls__Infix_6061")

# EXTERN ("Ls__Infix_3361")

# EXTERN ("Ls__Infix_6161")

# EXTERN ("Ls__Infix_3838")

# EXTERN ("Ls__Infix_3333")

# EXTERN ("Ls__Infix_58")

# EXTERN ("Li__Infix_4343")

# EXTERN ("Lcompare")

# EXTERN ("Lwrite")

# EXTERN ("Lread")

# EXTERN ("Lfailure")

# EXTERN ("Lfexists")

# EXTERN ("Lfwrite")

# EXTERN ("Lfread")

# EXTERN ("Lfclose")

# EXTERN ("Lfopen")

# EXTERN ("Lfprintf")

# EXTERN ("Lprintf")

# EXTERN ("LmakeString")

# EXTERN ("Lsprintf")

# EXTERN ("LregexpMatch")

# EXTERN ("Lregexp")

# EXTERN ("Lsubstring")

# EXTERN ("LmatchSubString")

# EXTERN ("Lstringcat")

# EXTERN ("LreadLine")

# EXTERN ("Ltl")

# EXTERN ("Lhd")

# EXTERN ("Lsnd")

# EXTERN ("Lfst")

# EXTERN ("Lhash")

# EXTERN ("Lclone")

# EXTERN ("Llength")

# EXTERN ("Lstring")

# EXTERN ("LmakeArray")

# EXTERN ("LstringInt")

# EXTERN ("global_sysargs")

# EXTERN ("Lsystem")

# EXTERN ("LgetEnv")

# EXTERN ("Lassert")

# LABEL ("main")

main:

# BEGIN ("main", 2, 0, [], [], [])

	.type main, @function

	.cfi_startproc

	movq	init(%rip),	%rax
	test	%rax,	%rax
	jz	continue
	ret
_ERROR:

	call	Lbinoperror
	ret
_ERROR2:

	call	Lbinoperror2
	ret
continue:

	movq	$1,	init(%rip)
	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$Lmain_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSmain_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
	movq	$15,	%rax
	test	%rsp,	%rax
	jz	ALIGNED
	pushq	filler(%rip)
ALIGNED:

	pushq	%rdi
	pushq	%rsi
	call	__gc_init
	popq	%rsi
	popq	%rdi
	call	set_args
# SLABEL ("L1")

L1:

# LINE (47)

	.stabn 68,0,47,.L0

.L0:

# LINE (49)

	.stabn 68,0,49,.L1

.L1:

# CONST (1000)

	movq	$2001,	%r10
# CALL ("Lgenerate", 1, false)

	pushq	%rdi
	pushq	%rsi
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Lgenerate
	popq	%rsi
	popq	%rdi
	movq	%rax,	%r10
# CALL ("LbubbleSort", 1, false)

	pushq	%rdi
	pushq	%rsi
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	LbubbleSort
	popq	%rsi
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L2")

L2:

# END

	movq	%r10,	%rax
Lmain_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	xorq	%rax,	%rax
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	Lmain_SIZE,	0

	.set	LSmain_SIZE,	0

	.size main, .-main

# LABEL ("LbubbleSort22323")

LbubbleSort22323:

# BEGIN ("LbubbleSort22323", 1, 0, [], ["l"], [{ blab="L5"; elab="L6"; names=[]; subs=[{ blab="L8"; elab="L9"; names=[]; subs=[]; }]; }])

	.type bubbleSort22323, @function

	.stabs "bubbleSort22323:F1",36,0,0,LbubbleSort22323

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLbubbleSort22323_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLbubbleSort22323_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	LbubbleSort22323_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_1(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	%rsi
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	popq	%rsi
	popq	%rdi
	movq	%rax,	%r10
LbubbleSort22323_argc_correct:

# SLABEL ("L5")

L5:

# SLABEL ("L8")

L8:

# LINE (44)

	.stabn 68,0,44,0

	.stabn 68,0,44,.L2-LbubbleSort22323

.L2:

# LINE (46)

	.stabn 68,0,46,.L3-LbubbleSort22323

.L3:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Lrec_3", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_3
# SLABEL ("L9")

L9:

# LABEL ("L7")

L7:

# SLABEL ("L6")

L6:

# END

	movq	%r10,	%rax
LLbubbleSort22323_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLbubbleSort22323_SIZE,	0

	.set	LSLbubbleSort22323_SIZE,	0

	.size LbubbleSort22323, .-LbubbleSort22323

# LABEL ("Lrec_3")

Lrec_3:

# BEGIN ("Lrec_3", 1, 1, [], ["l"], [{ blab="L11"; elab="L12"; names=[]; subs=[{ blab="L14"; elab="L15"; names=[]; subs=[{ blab="L29"; elab="L30"; names=[("l", 0)]; subs=[{ blab="L31"; elab="L32"; names=[]; subs=[]; }]; }; { blab="L22"; elab="L23"; names=[("l", 0)]; subs=[{ blab="L24"; elab="L25"; names=[]; subs=[]; }]; }]; }]; }])

	.type rec_3, @function

	.stabs "rec_3:F1",36,0,0,Lrec_3

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L29-Lrec_3

	.stabn 224,0,0,L30-Lrec_3

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L22-Lrec_3

	.stabn 224,0,0,L23-Lrec_3

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLrec_3_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLrec_3_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Lrec_3_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_2(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
Lrec_3_argc_correct:

# SLABEL ("L11")

L11:

# SLABEL ("L14")

L14:

# LINE (40)

	.stabn 68,0,40,0

	.stabn 68,0,40,.L4-Lrec_3

.L4:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Linner_3", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_3
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L22")

L22:

# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L20")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L20
# LABEL ("L21")

L21:

# DROP

# JMP ("L19")

	jmp	L19
# LABEL ("L20")

L20:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (1)

	movq	$3,	%r13
# BINOP ("==")

	xorq	%rax,	%rax
	cmpq	%r13,	%r12
	sete	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r12
# CJMP ("z", "L21")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L21
# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L24")

L24:

# LINE (41)

	.stabn 68,0,41,.L5-Lrec_3

.L5:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# CALL ("Lrec_3", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_3
# SLABEL ("L25")

L25:

# JMP ("L13")

	jmp	L13
# SLABEL ("L23")

L23:

# SLABEL ("L29")

L29:

# LABEL ("L19")

L19:

# DUP

	movq	%r10,	%r11
# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L27")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L27
# LABEL ("L28")

L28:

# DROP

# JMP ("L16")

	jmp	L16
# LABEL ("L27")

L27:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (0)

	movq	$1,	%r13
# BINOP ("==")

	xorq	%rax,	%rax
	cmpq	%r13,	%r12
	sete	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r12
# CJMP ("z", "L28")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L28
# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L31")

L31:

# LINE (42)

	.stabn 68,0,42,.L6-Lrec_3

.L6:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# SLABEL ("L32")

L32:

# SLABEL ("L30")

L30:

# JMP ("L13")

	jmp	L13
# LABEL ("L16")

L16:

# FAIL ((40, 9), true)

	movq	$19,	%r14
	movq	$81,	%r13
	leaq	string_3(%rip),	%r12
	movq	%r10,	%r11
	pushq	%rdi
	pushq	%r10
	movq	%r14,	%rcx
	movq	%r13,	%rdx
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$4,	%r11
	call	Bmatch_failure
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# JMP ("L13")

	jmp	L13
# SLABEL ("L15")

L15:

# LABEL ("L13")

L13:

# SLABEL ("L12")

L12:

# END

	movq	%r10,	%rax
LLrec_3_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLrec_3_SIZE,	16

	.set	LSLrec_3_SIZE,	1

	.size Lrec_3, .-Lrec_3

# LABEL ("Linner_3")

Linner_3:

# BEGIN ("Linner_3", 1, 6, [], ["l"], [{ blab="L33"; elab="L34"; names=[]; subs=[{ blab="L36"; elab="L37"; names=[]; subs=[{ blab="L79"; elab="L80"; names=[]; subs=[{ blab="L81"; elab="L82"; names=[]; subs=[]; }]; }; { blab="L45"; elab="L46"; names=[("x", 3); ("z", 2); ("y", 1); ("tl", 0)]; subs=[{ blab="L47"; elab="L48"; names=[]; subs=[{ blab="L64"; elab="L65"; names=[]; subs=[{ blab="L71"; elab="L72"; names=[("f", 5); ("z", 4)]; subs=[{ blab="L73"; elab="L74"; names=[]; subs=[]; }]; }]; }; { blab="L53"; elab="L54"; names=[]; subs=[]; }]; }]; }]; }]; }])

	.type inner_3, @function

	.stabs "inner_3:F1",36,0,0,Linner_3

	.stabs "x:1",128,0,0,-32

	.stabs "z:1",128,0,0,-24

	.stabs "y:1",128,0,0,-16

	.stabs "tl:1",128,0,0,-8

	.stabn 192,0,0,L45-Linner_3

	.stabs "f:1",128,0,0,-48

	.stabs "z:1",128,0,0,-40

	.stabn 192,0,0,L71-Linner_3

	.stabn 224,0,0,L72-Linner_3

	.stabn 224,0,0,L46-Linner_3

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLinner_3_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLinner_3_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Linner_3_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_4(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
Linner_3_argc_correct:

# SLABEL ("L33")

L33:

# SLABEL ("L36")

L36:

# LINE (29)

	.stabn 68,0,29,0

	.stabn 68,0,29,.L7-Linner_3

.L7:

# LD (Arg (0))

	movq	%rdi,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L45")

L45:

# DUP

	movq	%r11,	%r12
# TAG ("cons", 2)

	movq	$1697575,	%r13
	movq	$5,	%r14
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r14,	%rdx
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$3,	%r11
	call	Btag
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L41")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L41
# LABEL ("L42")

L42:

# DROP

# JMP ("L40")

	jmp	L40
# LABEL ("L41")

L41:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DUP

	movq	%r12,	%r13
# TAG ("cons", 2)

	movq	$1697575,	%r14
	movq	$5,	-56(%rbp)
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	-56(%rbp),	%rdx
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$3,	%r11
	call	Btag
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# CJMP ("nz", "L43")

	sarq	%r13
	cmpq	$0,	%r13
	jnz	L43
# LABEL ("L44")

L44:

# DROP

# JMP ("L42")

	jmp	L42
# LABEL ("L43")

L43:

# DUP

	movq	%r12,	%r13
# CONST (0)

	movq	$1,	%r14
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# DROP

# DUP

	movq	%r12,	%r13
# CONST (1)

	movq	$3,	%r14
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# DROP

# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (3))

	movq	%r11,	-32(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (2))

	movq	%r11,	-24(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (1))

	movq	%r11,	-16(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L47")

L47:

# LINE (31)

	.stabn 68,0,31,.L8-Linner_3

.L8:

# LD (Local (3))

	movq	-32(%rbp),	%r10
# LD (Local (1))

	movq	-16(%rbp),	%r11
# BINOP (">")

	xorq	%rax,	%rax
	cmpq	%r11,	%r10
	setg	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r10
# CJMP ("z", "L50")

	sarq	%r10
	cmpq	$0,	%r10
	jz	L50
# SLABEL ("L53")

L53:

# CONST (1)

	movq	$3,	%r10
# LINE (32)

	.stabn 68,0,32,.L9-Linner_3

.L9:

# LD (Local (1))

	movq	-16(%rbp),	%r11
# LD (Local (3))

	movq	-32(%rbp),	%r12
# LD (Local (0))

	movq	-8(%rbp),	%r13
# SEXP ("cons", 2)

	movq	$1697575,	%r14
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r14
	pushq	%r13
	pushq	%r12
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$24,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CALL ("Linner_3", 1, false)

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r12,	%rdi
	movq	$1,	%r11
	call	Linner_3
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# SEXP ("cons", 2)

	movq	$1697575,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	filler(%rip)
	pushq	%r13
	pushq	%r12
	pushq	%r11
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$32,	%rsp
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L54")

L54:

# JMP ("L35")

	jmp	L35
# LABEL ("L50")

L50:

# SLABEL ("L64")

L64:

# LINE (33)

	.stabn 68,0,33,.L10-Linner_3

.L10:

# LD (Local (2))

	movq	-24(%rbp),	%r10
# CALL ("Linner_3", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_3
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L71")

L71:

# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L69")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L69
# LABEL ("L70")

L70:

# DROP

# JMP ("L66")

	jmp	L66
# LABEL ("L69")

L69:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (5))

	movq	%r11,	-48(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (4))

	movq	%r11,	-40(%rbp)
# DROP

# DROP

# SLABEL ("L73")

L73:

# LD (Local (5))

	movq	-48(%rbp),	%r10
# LD (Local (3))

	movq	-32(%rbp),	%r11
# LD (Local (4))

	movq	-40(%rbp),	%r12
# SEXP ("cons", 2)

	movq	$1697575,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	filler(%rip)
	pushq	%r13
	pushq	%r12
	pushq	%r11
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$32,	%rsp
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L74")

L74:

# SLABEL ("L72")

L72:

# JMP ("L35")

	jmp	L35
# LABEL ("L66")

L66:

# FAIL ((33, 17), true)

	movq	$35,	%r14
	movq	$67,	%r13
	leaq	string_3(%rip),	%r12
	movq	%r10,	%r11
	pushq	%rdi
	pushq	%r10
	movq	%r14,	%rcx
	movq	%r13,	%rdx
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$4,	%r11
	call	Bmatch_failure
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# JMP ("L35")

	jmp	L35
# SLABEL ("L65")

L65:

# SLABEL ("L48")

L48:

# JMP ("L35")

# SLABEL ("L46")

L46:

# SLABEL ("L79")

L79:

# LABEL ("L40")

L40:

# DUP

	movq	%r10,	%r11
# DROP

# DROP

# SLABEL ("L81")

L81:

# CONST (0)

	movq	$1,	%r10
# LINE (35)

	.stabn 68,0,35,.L11-Linner_3

.L11:

# LD (Arg (0))

	movq	%rdi,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L82")

L82:

# SLABEL ("L80")

L80:

# JMP ("L35")

	jmp	L35
# SLABEL ("L37")

L37:

# LABEL ("L35")

L35:

# SLABEL ("L34")

L34:

# END

	movq	%r10,	%rax
LLinner_3_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLinner_3_SIZE,	64

	.set	LSLinner_3_SIZE,	7

	.size Linner_3, .-Linner_3

# LABEL ("Lgenerate")

Lgenerate:

# BEGIN ("Lgenerate", 1, 0, [], ["n"], [{ blab="L85"; elab="L86"; names=[]; subs=[{ blab="L88"; elab="L89"; names=[]; subs=[{ blab="L99"; elab="L100"; names=[]; subs=[]; }; { blab="L92"; elab="L93"; names=[]; subs=[]; }]; }]; }])

	.type generate, @function

	.stabs "generate:F1",36,0,0,Lgenerate

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLgenerate_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLgenerate_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Lgenerate_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_5(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
Lgenerate_argc_correct:

# SLABEL ("L85")

L85:

# SLABEL ("L88")

L88:

# LINE (24)

	.stabn 68,0,24,0

	.stabn 68,0,24,.L12-Lgenerate

.L12:

# LD (Arg (0))

	movq	%rdi,	%r10
# CJMP ("z", "L91")

	sarq	%r10
	cmpq	$0,	%r10
	jz	L91
# SLABEL ("L92")

L92:

# LD (Arg (0))

	movq	%rdi,	%r10
# LD (Arg (0))

	movq	%rdi,	%r11
# CONST (1)

	movq	$3,	%r12
# BINOP ("-")

	subq	%r12,	%r11
	orq	$0x0001,	%r11
# CALL ("Lgenerate", 1, false)

	pushq	%rdi
	pushq	%r10
	movq	%r11,	%rdi
	movq	$1,	%r11
	call	Lgenerate
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# SEXP ("cons", 2)

	movq	$1697575,	%r12
	pushq	%rdi
	pushq	%r12
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L93")

L93:

# JMP ("L87")

	jmp	L87
# LABEL ("L91")

L91:

# SLABEL ("L99")

L99:

# CONST (0)

	movq	$1,	%r10
# SLABEL ("L100")

L100:

# JMP ("L87")

	jmp	L87
# SLABEL ("L89")

L89:

# LABEL ("L87")

L87:

# SLABEL ("L86")

L86:

# END

	movq	%r10,	%rax
LLgenerate_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLgenerate_SIZE,	0

	.set	LSLgenerate_SIZE,	0

	.size Lgenerate, .-Lgenerate

# LABEL ("LbubbleSort")

LbubbleSort:

# BEGIN ("LbubbleSort", 1, 0, [], ["l"], [{ blab="L101"; elab="L102"; names=[]; subs=[{ blab="L104"; elab="L105"; names=[]; subs=[]; }]; }])

	.type bubbleSort, @function

	.stabs "bubbleSort:F1",36,0,0,LbubbleSort

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLbubbleSort_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLbubbleSort_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	LbubbleSort_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_6(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
LbubbleSort_argc_correct:

# SLABEL ("L101")

L101:

# SLABEL ("L104")

L104:

# LINE (18)

	.stabn 68,0,18,0

	.stabn 68,0,18,.L13-LbubbleSort

.L13:

# LINE (20)

	.stabn 68,0,20,.L14-LbubbleSort

.L14:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Lrec_25", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_25
# SLABEL ("L105")

L105:

# LABEL ("L103")

L103:

# SLABEL ("L102")

L102:

# END

	movq	%r10,	%rax
LLbubbleSort_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLbubbleSort_SIZE,	0

	.set	LSLbubbleSort_SIZE,	0

	.size LbubbleSort, .-LbubbleSort

# LABEL ("Lrec_25")

Lrec_25:

# BEGIN ("Lrec_25", 1, 1, [], ["l"], [{ blab="L107"; elab="L108"; names=[]; subs=[{ blab="L110"; elab="L111"; names=[]; subs=[{ blab="L125"; elab="L126"; names=[("l", 0)]; subs=[{ blab="L127"; elab="L128"; names=[]; subs=[]; }]; }; { blab="L118"; elab="L119"; names=[("l", 0)]; subs=[{ blab="L120"; elab="L121"; names=[]; subs=[]; }]; }]; }]; }])

	.type rec_25, @function

	.stabs "rec_25:F1",36,0,0,Lrec_25

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L125-Lrec_25

	.stabn 224,0,0,L126-Lrec_25

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L118-Lrec_25

	.stabn 224,0,0,L119-Lrec_25

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLrec_25_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLrec_25_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Lrec_25_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_7(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
Lrec_25_argc_correct:

# SLABEL ("L107")

L107:

# SLABEL ("L110")

L110:

# LINE (14)

	.stabn 68,0,14,0

	.stabn 68,0,14,.L15-Lrec_25

.L15:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Linner_25", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_25
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L118")

L118:

# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L116")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L116
# LABEL ("L117")

L117:

# DROP

# JMP ("L115")

	jmp	L115
# LABEL ("L116")

L116:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (1)

	movq	$3,	%r13
# BINOP ("==")

	xorq	%rax,	%rax
	cmpq	%r13,	%r12
	sete	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r12
# CJMP ("z", "L117")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L117
# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L120")

L120:

# LINE (15)

	.stabn 68,0,15,.L16-Lrec_25

.L16:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# CALL ("Lrec_25", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_25
# SLABEL ("L121")

L121:

# JMP ("L109")

	jmp	L109
# SLABEL ("L119")

L119:

# SLABEL ("L125")

L125:

# LABEL ("L115")

L115:

# DUP

	movq	%r10,	%r11
# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L123")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L123
# LABEL ("L124")

L124:

# DROP

# JMP ("L112")

	jmp	L112
# LABEL ("L123")

L123:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (0)

	movq	$1,	%r13
# BINOP ("==")

	xorq	%rax,	%rax
	cmpq	%r13,	%r12
	sete	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r12
# CJMP ("z", "L124")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L124
# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L127")

L127:

# LINE (16)

	.stabn 68,0,16,.L17-Lrec_25

.L17:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# SLABEL ("L128")

L128:

# SLABEL ("L126")

L126:

# JMP ("L109")

	jmp	L109
# LABEL ("L112")

L112:

# FAIL ((14, 9), true)

	movq	$19,	%r14
	movq	$29,	%r13
	leaq	string_3(%rip),	%r12
	movq	%r10,	%r11
	pushq	%rdi
	pushq	%r10
	movq	%r14,	%rcx
	movq	%r13,	%rdx
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$4,	%r11
	call	Bmatch_failure
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# JMP ("L109")

	jmp	L109
# SLABEL ("L111")

L111:

# LABEL ("L109")

L109:

# SLABEL ("L108")

L108:

# END

	movq	%r10,	%rax
LLrec_25_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLrec_25_SIZE,	16

	.set	LSLrec_25_SIZE,	1

	.size Lrec_25, .-Lrec_25

# LABEL ("Linner_25")

Linner_25:

# BEGIN ("Linner_25", 1, 6, [], ["l"], [{ blab="L129"; elab="L130"; names=[]; subs=[{ blab="L132"; elab="L133"; names=[]; subs=[{ blab="L175"; elab="L176"; names=[]; subs=[{ blab="L177"; elab="L178"; names=[]; subs=[]; }]; }; { blab="L141"; elab="L142"; names=[("x", 3); ("z", 2); ("y", 1); ("tl", 0)]; subs=[{ blab="L143"; elab="L144"; names=[]; subs=[{ blab="L160"; elab="L161"; names=[]; subs=[{ blab="L167"; elab="L168"; names=[("f", 5); ("z", 4)]; subs=[{ blab="L169"; elab="L170"; names=[]; subs=[]; }]; }]; }; { blab="L149"; elab="L150"; names=[]; subs=[]; }]; }]; }]; }]; }])

	.type inner_25, @function

	.stabs "inner_25:F1",36,0,0,Linner_25

	.stabs "x:1",128,0,0,-32

	.stabs "z:1",128,0,0,-24

	.stabs "y:1",128,0,0,-16

	.stabs "tl:1",128,0,0,-8

	.stabn 192,0,0,L141-Linner_25

	.stabs "f:1",128,0,0,-48

	.stabs "z:1",128,0,0,-40

	.stabn 192,0,0,L167-Linner_25

	.stabn 224,0,0,L168-Linner_25

	.stabn 224,0,0,L142-Linner_25

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLinner_25_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLinner_25_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Linner_25_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_8(%rip),	%r11
	leaq	string_0(%rip),	%r10
	pushq	%rdi
	pushq	filler(%rip)
	movq	%r13,	%rcx
	movq	%r12,	%rdx
	movq	%r11,	%rsi
	movq	%r10,	%rdi
	movq	$4,	%r11
	call	failure
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
Linner_25_argc_correct:

# SLABEL ("L129")

L129:

# SLABEL ("L132")

L132:

# LINE (3)

	.stabn 68,0,3,0

	.stabn 68,0,3,.L18-Linner_25

.L18:

# LD (Arg (0))

	movq	%rdi,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L141")

L141:

# DUP

	movq	%r11,	%r12
# TAG ("cons", 2)

	movq	$1697575,	%r13
	movq	$5,	%r14
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r14,	%rdx
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$3,	%r11
	call	Btag
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L137")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L137
# LABEL ("L138")

L138:

# DROP

# JMP ("L136")

	jmp	L136
# LABEL ("L137")

L137:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DUP

	movq	%r12,	%r13
# TAG ("cons", 2)

	movq	$1697575,	%r14
	movq	$5,	-56(%rbp)
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	-56(%rbp),	%rdx
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$3,	%r11
	call	Btag
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# CJMP ("nz", "L139")

	sarq	%r13
	cmpq	$0,	%r13
	jnz	L139
# LABEL ("L140")

L140:

# DROP

# JMP ("L138")

	jmp	L138
# LABEL ("L139")

L139:

# DUP

	movq	%r12,	%r13
# CONST (0)

	movq	$1,	%r14
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# DROP

# DUP

	movq	%r12,	%r13
# CONST (1)

	movq	$3,	%r14
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r12
	movq	%r14,	%rsi
	movq	%r13,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r13
# DROP

# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (3))

	movq	%r11,	-32(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (2))

	movq	%r11,	-24(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (1))

	movq	%r11,	-16(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (0))

	movq	%r11,	-8(%rbp)
# DROP

# DROP

# SLABEL ("L143")

L143:

# LINE (5)

	.stabn 68,0,5,.L19-Linner_25

.L19:

# LD (Local (3))

	movq	-32(%rbp),	%r10
# LD (Local (1))

	movq	-16(%rbp),	%r11
# BINOP (">")

	xorq	%rax,	%rax
	cmpq	%r11,	%r10
	setg	%al
	salq	%rax
	orq	$0x0001,	%rax
	movq	%rax,	%r10
# CJMP ("z", "L146")

	sarq	%r10
	cmpq	$0,	%r10
	jz	L146
# SLABEL ("L149")

L149:

# CONST (1)

	movq	$3,	%r10
# LINE (6)

	.stabn 68,0,6,.L20-Linner_25

.L20:

# LD (Local (1))

	movq	-16(%rbp),	%r11
# LD (Local (3))

	movq	-32(%rbp),	%r12
# LD (Local (0))

	movq	-8(%rbp),	%r13
# SEXP ("cons", 2)

	movq	$1697575,	%r14
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	%r14
	pushq	%r13
	pushq	%r12
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$24,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CALL ("Linner_25", 1, false)

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r12,	%rdi
	movq	$1,	%r11
	call	Linner_25
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# SEXP ("cons", 2)

	movq	$1697575,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	filler(%rip)
	pushq	%r13
	pushq	%r12
	pushq	%r11
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$32,	%rsp
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L150")

L150:

# JMP ("L131")

	jmp	L131
# LABEL ("L146")

L146:

# SLABEL ("L160")

L160:

# LINE (7)

	.stabn 68,0,7,.L21-Linner_25

.L21:

# LD (Local (2))

	movq	-24(%rbp),	%r10
# CALL ("Linner_25", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_25
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L167")

L167:

# DUP

	movq	%r11,	%r12
# ARRAY (2)

	movq	$5,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Barray_patt
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# CJMP ("nz", "L165")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L165
# LABEL ("L166")

L166:

# DROP

# JMP ("L162")

	jmp	L162
# LABEL ("L165")

L165:

# DUP

	movq	%r11,	%r12
# CONST (0)

	movq	$1,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DUP

	movq	%r11,	%r12
# CONST (1)

	movq	$3,	%r13
# ELEM

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r13,	%rsi
	movq	%r12,	%rdi
	movq	$2,	%r11
	call	Belem
	addq	$8,	%rsp
	popq	%r11
	popq	%r10
	popq	%rdi
	movq	%rax,	%r12
# DROP

# DROP

# DUP

	movq	%r10,	%r11
# CONST (0)

	movq	$1,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (5))

	movq	%r11,	-48(%rbp)
# DROP

# DUP

	movq	%r10,	%r11
# CONST (1)

	movq	$3,	%r12
# ELEM

	pushq	%rdi
	pushq	%r10
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$2,	%r11
	call	Belem
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# ST (Local (4))

	movq	%r11,	-40(%rbp)
# DROP

# DROP

# SLABEL ("L169")

L169:

# LD (Local (5))

	movq	-48(%rbp),	%r10
# LD (Local (3))

	movq	-32(%rbp),	%r11
# LD (Local (4))

	movq	-40(%rbp),	%r12
# SEXP ("cons", 2)

	movq	$1697575,	%r13
	pushq	%rdi
	pushq	%r10
	pushq	filler(%rip)
	pushq	%r13
	pushq	%r12
	pushq	%r11
	movq	%rsp,	%rdi
	movq	$7,	%rsi
	call	Bsexp
	addq	$32,	%rsp
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L170")

L170:

# SLABEL ("L168")

L168:

# JMP ("L131")

	jmp	L131
# LABEL ("L162")

L162:

# FAIL ((7, 17), true)

	movq	$35,	%r14
	movq	$15,	%r13
	leaq	string_3(%rip),	%r12
	movq	%r10,	%r11
	pushq	%rdi
	pushq	%r10
	movq	%r14,	%rcx
	movq	%r13,	%rdx
	movq	%r12,	%rsi
	movq	%r11,	%rdi
	movq	$4,	%r11
	call	Bmatch_failure
	popq	%r10
	popq	%rdi
	movq	%rax,	%r11
# JMP ("L131")

	jmp	L131
# SLABEL ("L161")

L161:

# SLABEL ("L144")

L144:

# JMP ("L131")

# SLABEL ("L142")

L142:

# SLABEL ("L175")

L175:

# LABEL ("L136")

L136:

# DUP

	movq	%r10,	%r11
# DROP

# DROP

# SLABEL ("L177")

L177:

# CONST (0)

	movq	$1,	%r10
# LINE (9)

	.stabn 68,0,9,.L22-Linner_25

.L22:

# LD (Arg (0))

	movq	%rdi,	%r11
# CALL (".array", 2, true)

	pushq	%rdi
	pushq	filler(%rip)
	pushq	%r11
	pushq	%r10
	movq	%rsp,	%rdi
	movq	$5,	%rsi
	call	Barray
	addq	$24,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# SLABEL ("L178")

L178:

# SLABEL ("L176")

L176:

# JMP ("L131")

	jmp	L131
# SLABEL ("L133")

L133:

# LABEL ("L131")

L131:

# SLABEL ("L130")

L130:

# END

	movq	%r10,	%rax
LLinner_25_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLinner_25_SIZE,	64

	.set	LSLinner_25_SIZE,	7

	.size Linner_25, .-Linner_25

