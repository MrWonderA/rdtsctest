	.text
	.align 4

	.globl _RDTSC
_RDTSC:
	.byte	0x0f
	.byte	0x31
	ret


	.globl _LMULDWORD
_LMULDWORD:
	pushl	%ecx
	movl	12(%esp),%eax
	mull	16(%esp)
	movl	%eax,%ecx

	movl	8(%esp),%eax
	mull	16(%esp)
	addl	%ecx,%edx

	popl	%ecx
	ret

	
	.globl _EXECMASM
_EXECMASM:
	pushl	%eax
	pushl	%ecx

	movl	$10000,%ecx
l1:	
	addl	%ecx,%eax
	subl	%ecx,%eax
	jmp	l2
l2:	
	loop	l1

	popl	%ecx
	popl	%eax
	ret


	.globl AsmSchedule
AsmSchedule:
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp

	movl	0x20(%esp),%eax
	movl	%esp,%ecx
	pushl	%eax
	pushl	%ecx
	.globl	Schedule
	call	Schedule
	movl	%eax,%esp

	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax
	ret
