	.text
	.align 4

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
