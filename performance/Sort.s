	.file "/home/doctor/Lama/performance/Sort.lama"

	.stabs "/home/doctor/Lama/performance/Sort.lama",100,0,0,.Ltext

	.globl	main

	.data

string_0:	.string	"Function %s called with incorrect arguments count. Expected: %d. Actual: %d\n"

string_4:	.string	"Sort.lama"

string_2:	.string	"bubbleSort"

string_1:	.string	"generate"

string_5:	.string	"inner_7"

string_3:	.string	"rec_7"

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

# LINE (25)

	.stabn 68,0,25,.L0

.L0:

# LINE (27)

	.stabn 68,0,27,.L1

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

# LABEL ("Lgenerate")

Lgenerate:

# BEGIN ("Lgenerate", 1, 0, [], ["n"], [{ blab="L5"; elab="L6"; names=[]; subs=[{ blab="L8"; elab="L9"; names=[]; subs=[{ blab="L19"; elab="L20"; names=[]; subs=[]; }; { blab="L12"; elab="L13"; names=[]; subs=[]; }]; }]; }])

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
Lgenerate_argc_correct:

# SLABEL ("L5")

L5:

# SLABEL ("L8")

L8:

# LINE (24)

	.stabn 68,0,24,0

	.stabn 68,0,24,.L2-Lgenerate

.L2:

# LD (Arg (0))

	movq	%rdi,	%r10
# CJMP ("z", "L11")

	sarq	%r10
	cmpq	$0,	%r10
	jz	L11
# SLABEL ("L12")

L12:

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
# SLABEL ("L13")

L13:

# JMP ("L7")

	jmp	L7
# LABEL ("L11")

L11:

# SLABEL ("L19")

L19:

# CONST (0)

	movq	$1,	%r10
# SLABEL ("L20")

L20:

# JMP ("L7")

	jmp	L7
# SLABEL ("L9")

L9:

# LABEL ("L7")

L7:

# SLABEL ("L6")

L6:

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

# BEGIN ("LbubbleSort", 1, 0, [], ["l"], [{ blab="L21"; elab="L22"; names=[]; subs=[{ blab="L24"; elab="L25"; names=[]; subs=[]; }]; }])

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
LbubbleSort_argc_correct:

# SLABEL ("L21")

L21:

# SLABEL ("L24")

L24:

# LINE (18)

	.stabn 68,0,18,0

	.stabn 68,0,18,.L3-LbubbleSort

.L3:

# LINE (20)

	.stabn 68,0,20,.L4-LbubbleSort

.L4:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Lrec_7", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_7
# SLABEL ("L25")

L25:

# LABEL ("L23")

L23:

# SLABEL ("L22")

L22:

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

# LABEL ("Lrec_7")

Lrec_7:

# BEGIN ("Lrec_7", 1, 1, [], ["l"], [{ blab="L27"; elab="L28"; names=[]; subs=[{ blab="L30"; elab="L31"; names=[]; subs=[{ blab="L45"; elab="L46"; names=[("l", 0)]; subs=[{ blab="L47"; elab="L48"; names=[]; subs=[]; }]; }; { blab="L38"; elab="L39"; names=[("l", 0)]; subs=[{ blab="L40"; elab="L41"; names=[]; subs=[]; }]; }]; }]; }])

	.type rec_7, @function

	.stabs "rec_7:F1",36,0,0,Lrec_7

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L45-Lrec_7

	.stabn 224,0,0,L46-Lrec_7

	.stabs "l:1",128,0,0,-8

	.stabn 192,0,0,L38-Lrec_7

	.stabn 224,0,0,L39-Lrec_7

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLrec_7_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLrec_7_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Lrec_7_argc_correct
	movq	%r11,	%r13
	movq	$1,	%r12
	leaq	string_3(%rip),	%r11
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
Lrec_7_argc_correct:

# SLABEL ("L27")

L27:

# SLABEL ("L30")

L30:

# LINE (14)

	.stabn 68,0,14,0

	.stabn 68,0,14,.L5-Lrec_7

.L5:

# LD (Arg (0))

	movq	%rdi,	%r10
# CALL ("Linner_7", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_7
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L38")

L38:

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
# CJMP ("nz", "L36")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L36
# LABEL ("L37")

L37:

# DROP

# JMP ("L35")

	jmp	L35
# LABEL ("L36")

L36:

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
# CJMP ("z", "L37")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L37
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

# SLABEL ("L40")

L40:

# LINE (15)

	.stabn 68,0,15,.L6-Lrec_7

.L6:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# CALL ("Lrec_7", 1, true)

	movq	%r10,	%rdi
	movq	%rbp,	%rsp
	popq	%rbp
	movq	$1,	%r11
	jmp	Lrec_7
# SLABEL ("L41")

L41:

# JMP ("L29")

	jmp	L29
# SLABEL ("L39")

L39:

# SLABEL ("L45")

L45:

# LABEL ("L35")

L35:

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
# CJMP ("nz", "L43")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L43
# LABEL ("L44")

L44:

# DROP

# JMP ("L32")

	jmp	L32
# LABEL ("L43")

L43:

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
# CJMP ("z", "L44")

	sarq	%r12
	cmpq	$0,	%r12
	jz	L44
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

# SLABEL ("L47")

L47:

# LINE (16)

	.stabn 68,0,16,.L7-Lrec_7

.L7:

# LD (Local (0))

	movq	-8(%rbp),	%r10
# SLABEL ("L48")

L48:

# SLABEL ("L46")

L46:

# JMP ("L29")

	jmp	L29
# LABEL ("L32")

L32:

# FAIL ((14, 9), true)

	movq	$19,	%r14
	movq	$29,	%r13
	leaq	string_4(%rip),	%r12
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
# JMP ("L29")

	jmp	L29
# SLABEL ("L31")

L31:

# LABEL ("L29")

L29:

# SLABEL ("L28")

L28:

# END

	movq	%r10,	%rax
LLrec_7_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLrec_7_SIZE,	16

	.set	LSLrec_7_SIZE,	1

	.size Lrec_7, .-Lrec_7

# LABEL ("Linner_7")

Linner_7:

# BEGIN ("Linner_7", 1, 6, [], ["l"], [{ blab="L49"; elab="L50"; names=[]; subs=[{ blab="L52"; elab="L53"; names=[]; subs=[{ blab="L95"; elab="L96"; names=[]; subs=[{ blab="L97"; elab="L98"; names=[]; subs=[]; }]; }; { blab="L61"; elab="L62"; names=[("x", 3); ("z", 2); ("y", 1); ("tl", 0)]; subs=[{ blab="L63"; elab="L64"; names=[]; subs=[{ blab="L80"; elab="L81"; names=[]; subs=[{ blab="L87"; elab="L88"; names=[("f", 5); ("z", 4)]; subs=[{ blab="L89"; elab="L90"; names=[]; subs=[]; }]; }]; }; { blab="L69"; elab="L70"; names=[]; subs=[]; }]; }]; }]; }]; }])

	.type inner_7, @function

	.stabs "inner_7:F1",36,0,0,Linner_7

	.stabs "x:1",128,0,0,-32

	.stabs "z:1",128,0,0,-24

	.stabs "y:1",128,0,0,-16

	.stabs "tl:1",128,0,0,-8

	.stabn 192,0,0,L61-Linner_7

	.stabs "f:1",128,0,0,-48

	.stabs "z:1",128,0,0,-40

	.stabn 192,0,0,L87-Linner_7

	.stabn 224,0,0,L88-Linner_7

	.stabn 224,0,0,L62-Linner_7

	.cfi_startproc

	pushq	%rbp
	.cfi_def_cfa_offset	8

	.cfi_offset 5, -8

	movq	%rsp,	%rbp
	.cfi_def_cfa_register	5

	subq	$LLinner_7_SIZE,	%rsp
	movq	%rdi,	%r12
	movq	%rsi,	%r13
	movq	%rcx,	%r14
	movq	%rsp,	%rdi
	leaq	filler(%rip),	%rsi
	movq	$LSLinner_7_SIZE,	%rcx
	rep movsq	
	movq	%r12,	%rdi
	movq	%r13,	%rsi
	movq	%r14,	%rcx
# Check arguments count

	cmpq	$1,	%r11
	je	Linner_7_argc_correct
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
Linner_7_argc_correct:

# SLABEL ("L49")

L49:

# SLABEL ("L52")

L52:

# LINE (3)

	.stabn 68,0,3,0

	.stabn 68,0,3,.L8-Linner_7

.L8:

# LD (Arg (0))

	movq	%rdi,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L61")

L61:

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
# CJMP ("nz", "L57")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L57
# LABEL ("L58")

L58:

# DROP

# JMP ("L56")

	jmp	L56
# LABEL ("L57")

L57:

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
# CJMP ("nz", "L59")

	sarq	%r13
	cmpq	$0,	%r13
	jnz	L59
# LABEL ("L60")

L60:

# DROP

# JMP ("L58")

	jmp	L58
# LABEL ("L59")

L59:

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

# SLABEL ("L63")

L63:

# LINE (5)

	.stabn 68,0,5,.L9-Linner_7

.L9:

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
# CJMP ("z", "L66")

	sarq	%r10
	cmpq	$0,	%r10
	jz	L66
# SLABEL ("L69")

L69:

# CONST (1)

	movq	$3,	%r10
# LINE (6)

	.stabn 68,0,6,.L10-Linner_7

.L10:

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
# CALL ("Linner_7", 1, false)

	pushq	%rdi
	pushq	%r10
	pushq	%r11
	pushq	filler(%rip)
	movq	%r12,	%rdi
	movq	$1,	%r11
	call	Linner_7
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
# SLABEL ("L70")

L70:

# JMP ("L51")

	jmp	L51
# LABEL ("L66")

L66:

# SLABEL ("L80")

L80:

# LINE (7)

	.stabn 68,0,7,.L11-Linner_7

.L11:

# LD (Local (2))

	movq	-24(%rbp),	%r10
# CALL ("Linner_7", 1, false)

	pushq	%rdi
	pushq	filler(%rip)
	movq	%r10,	%rdi
	movq	$1,	%r11
	call	Linner_7
	addq	$8,	%rsp
	popq	%rdi
	movq	%rax,	%r10
# DUP

	movq	%r10,	%r11
# SLABEL ("L87")

L87:

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
# CJMP ("nz", "L85")

	sarq	%r12
	cmpq	$0,	%r12
	jnz	L85
# LABEL ("L86")

L86:

# DROP

# JMP ("L82")

	jmp	L82
# LABEL ("L85")

L85:

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

# SLABEL ("L89")

L89:

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
# SLABEL ("L90")

L90:

# SLABEL ("L88")

L88:

# JMP ("L51")

	jmp	L51
# LABEL ("L82")

L82:

# FAIL ((7, 17), true)

	movq	$35,	%r14
	movq	$15,	%r13
	leaq	string_4(%rip),	%r12
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
# JMP ("L51")

	jmp	L51
# SLABEL ("L81")

L81:

# SLABEL ("L64")

L64:

# JMP ("L51")

# SLABEL ("L62")

L62:

# SLABEL ("L95")

L95:

# LABEL ("L56")

L56:

# DUP

	movq	%r10,	%r11
# DROP

# DROP

# SLABEL ("L97")

L97:

# CONST (0)

	movq	$1,	%r10
# LINE (9)

	.stabn 68,0,9,.L12-Linner_7

.L12:

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
# SLABEL ("L98")

L98:

# SLABEL ("L96")

L96:

# JMP ("L51")

	jmp	L51
# SLABEL ("L53")

L53:

# LABEL ("L51")

L51:

# SLABEL ("L50")

L50:

# END

	movq	%r10,	%rax
LLinner_7_epilogue:

	movq	%rbp,	%rsp
	popq	%rbp
	.cfi_restore	rbp

	.cfi_def_cfa	4, 4

	ret
	.cfi_endproc

	.set	LLinner_7_SIZE,	64

	.set	LSLinner_7_SIZE,	7

	.size Linner_7, .-Linner_7

