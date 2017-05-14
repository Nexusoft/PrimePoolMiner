.globl red2norm1024
.type red2norm1024,@function
.align 64




red2norm1024:

	movq	(%rsi),%rdx
	movq	$29,%rax
	shlxq	%rax,8(%rsi),%rax

	addq	%rax,%rdx
	movq	$58,%rax
	shlxq	%rax,16(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,(%rdi)

	movq	$6,%rax
	shrxq	%rax,16(%rsi),%rdx
	adcq	$0,%rdx

	movq	$23,%rax
	shlxq	%rax,24(%rsi),%rax
	addq	%rax,%rdx

	movq	$52,%rax
	shlxq	%rax,32(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,8(%rdi)

	movq	$12,%rax
	shrxq	%rax,32(%rsi),%rdx
	adcq	$0,%rdx

	movq	$17,%rax
	shlxq	%rax,40(%rsi),%rax
	addq	%rax,%rdx

	movq	$46,%rax
	shlxq	%rax,48(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,16(%rdi)

	movq	$18,%rax
	shrxq	%rax,48(%rsi),%rdx
	adcq	$0,%rdx

	movq	$11,%rax
	shlxq	%rax,56(%rsi),%rax
	addq	%rax,%rdx

	movq	$40,%rax
	shlxq	%rax,64(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,24(%rdi)

	movq	$24,%rax
	shrxq	%rax,64(%rsi),%rdx
	adcq	$0,%rdx

	movq	$5,%rax
	shlxq	%rax,72(%rsi),%rax
	addq	%rax,%rdx

	movq	$34,%rax
	shlxq	%rax,80(%rsi),%rax
	addq	%rax,%rdx

	movq	$63,%rax
	shlxq	%rax,88(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,32(%rdi)


	movq	$1,%rax
	shrxq	%rax,88(%rsi),%rdx
	adcq	$0,%rdx

	movq	$28,%rax
	shlxq	%rax,96(%rsi),%rax
	addq	%rax,%rdx

	movq	$57,%rax
	shlxq	%rax,104(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,40(%rdi)

	movq	$7,%rax
	shrxq	%rax,104(%rsi),%rdx
	adcq	$0,%rdx

	movq	$22,%rax
	shlxq	%rax,112(%rsi),%rax
	addq	%rax,%rdx

	movq	$51,%rax
	shlxq	%rax,120(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,48(%rdi)

	movq	$13,%rax
	shrxq	%rax,120(%rsi),%rdx
	adcq	$0,%rdx

	movq	$16,%rax
	shlxq	%rax,128(%rsi),%rax
	addq	%rax,%rdx

	movq	$45,%rax
	shlxq	%rax,136(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,56(%rdi)

	movq	$19,%rax
	shrxq	%rax,136(%rsi),%rdx
	adcq	$0,%rdx

	movq	$10,%rax
	shlxq	%rax,144(%rsi),%rax
	addq	%rax,%rdx

	movq	$39,%rax
	shlxq	%rax,152(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,64(%rdi)

	movq	$25,%rax
	shrxq	%rax,152(%rsi),%rdx
	adcq	$0,%rdx

	movq	$4,%rax
	shlxq	%rax,160(%rsi),%rax
	addq	%rax,%rdx

	movq	$33,%rax
	shlxq	%rax,168(%rsi),%rax
	addq	%rax,%rdx

	movq	$62,%rax
	shlxq	%rax,176(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,72(%rdi)


	movq	$2,%rax
	shrxq	%rax,176(%rsi),%rdx
	adcq	$0,%rdx

	movq	$27,%rax
	shlxq	%rax,184(%rsi),%rax
	addq	%rax,%rdx

	movq	$56,%rax
	shlxq	%rax,192(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,80(%rdi)

	movq	$8,%rax
	shrxq	%rax,192(%rsi),%rdx
	adcq	$0,%rdx

	movq	$21,%rax
	shlxq	%rax,200(%rsi),%rax
	addq	%rax,%rdx

	movq	$50,%rax
	shlxq	%rax,208(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,88(%rdi)

	movq	$14,%rax
	shrxq	%rax,208(%rsi),%rdx
	adcq	$0,%rdx

	movq	$15,%rax
	shlxq	%rax,216(%rsi),%rax
	addq	%rax,%rdx

	movq	$44,%rax
	shlxq	%rax,224(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,96(%rdi)

	movq	$20,%rax
	shrxq	%rax,224(%rsi),%rdx
	adcq	$0,%rdx

	movq	$9,%rax
	shlxq	%rax,232(%rsi),%rax
	addq	%rax,%rdx

	movq	$38,%rax
	shlxq	%rax,240(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,104(%rdi)

	movq	$26,%rax
	shrxq	%rax,240(%rsi),%rdx
	adcq	$0,%rdx

	movq	$3,%rax
	shlxq	%rax,248(%rsi),%rax
	addq	%rax,%rdx

	movq	$32,%rax
	shlxq	%rax,256(%rsi),%rax
	addq	%rax,%rdx

	movq	$61,%rax
	shlxq	%rax,264(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,112(%rdi)


	movq	$3,%rax
	shrxq	%rax,264(%rsi),%rdx
	adcq	$0,%rdx

	movq	$26,%rax
	shlxq	%rax,272(%rsi),%rax
	addq	%rax,%rdx

	movq	$55,%rax
	shlxq	%rax,280(%rsi),%rax
	addq	%rax,%rdx
	movq	%rdx,120(%rdi)

	.byte	0xf3,0xc3
.globl	red2norm2048
.type	red2norm2048,@function
.align	64




red2norm2048:

	xorq	%rdx,%rdx

	movq	$4,%r8

.Lred2norm2048_loop:
	addq	(%rsi),%rdx
	movq	$28,%rax
	shlxq	%rax,8(%rsi),%rax
	addq	%rax,%rdx

	movq	$56,%rax
	shlxq	%rax,16(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,(%rdi)

	movq	$8,%rax
	shrxq	%rax,16(%rsi),%rdx
	adcq	$0,%rdx

	movq	$20,%rax
	shlxq	%rax,24(%rsi),%rax
	addq	%rax,%rdx

	movq	$48,%rax
	shlxq	%rax,32(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,8(%rdi)

	movq	$16,%rax
	shrxq	%rax,32(%rsi),%rdx
	adcq	$0,%rdx

	movq	$12,%rax
	shlxq	%rax,40(%rsi),%rax
	addq	%rax,%rdx

	movq	$40,%rax
	shlxq	%rax,48(%rsi),%rax
	addq	%rax,%rdx

	movq	%rdx,16(%rdi)

	movq	$24,%rax
	shrxq	%rax,48(%rsi),%rdx
	adcq	$0,%rdx

	movq	$4,%rax
	shlxq	%rax,56(%rsi),%rax
	addq	%rax,%rdx

	movq	$32,%rax
	shlxq	%rax,64(%rsi),%rax
	addq	%rax,%rdx

	movq	$60,%rax
	shlxq	%rax,72(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,24(%rdi)

	movq	$4,%rax
	shrxq	%rax,72(%rsi),%rdx
	adcq	$0,%rdx

	movq	$24,%rax
	shlxq	%rax,80(%rsi),%rax
	addq	%rax,%rdx

	movq	$52,%rax
	shlxq	%rax,88(%rsi),%rax
	addq	%rax,%rdx

	movq	%rdx,32(%rdi)


	movq	$12,%rax
	shrxq	%rax,88(%rsi),%rdx
	adcq	$0,%rdx

	movq	$16,%rax
	shlxq	%rax,96(%rsi),%rax
	addq	%rax,%rdx

	movq	$44,%rax
	shlxq	%rax,104(%rsi),%rax
	addq	%rax,%rdx

	movq	%rdx,40(%rdi)

	movq	$20,%rax
	shrxq	%rax,104(%rsi),%rdx
	adcq	$0,%rdx

	movq	$8,%rax
	shlxq	%rax,112(%rsi),%rax
	addq	%rax,%rdx

	movq	$36,%rax
	shlxq	%rax,120(%rsi),%rax
	addq	%rax,%rdx

	movq	%rdx,48(%rdi)

	movq	$28,%rax
	shrxq	%rax,120(%rsi),%rdx
	adcq	$0,%rdx

	leaq	128(%rsi),%rsi
	leaq	56(%rdi),%rdi

	decq	%r8
	jnz	.Lred2norm2048_loop

	addq	(%rsi),%rdx
	movq	$28,%rax
	shlxq	%rax,8(%rsi),%rax
	addq	%rax,%rdx

	movq	$56,%rax
	shlxq	%rax,16(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,(%rdi)

	movq	$8,%rax
	shrxq	%rax,16(%rsi),%rdx
	adcq	$0,%rdx

	movq	$20,%rax
	shlxq	%rax,24(%rsi),%rax
	addq	%rax,%rdx

	movq	$48,%rax
	shlxq	%rax,32(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,8(%rdi)

	movq	$16,%rax
	shrxq	%rax,32(%rsi),%rdx
	adcq	$0,%rdx

	movq	$12,%rax
	shlxq	%rax,40(%rsi),%rax
	addq	%rax,%rdx

	movq	$40,%rax
	shlxq	%rax,48(%rsi),%rax
	addq	%rax,%rdx

	movq	%rdx,16(%rdi)

	movq	$24,%rax
	shrxq	%rax,48(%rsi),%rdx
	adcq	$0,%rdx

	movq	$4,%rax
	shlxq	%rax,56(%rsi),%rax
	addq	%rax,%rdx

	movq	$32,%rax
	shlxq	%rax,64(%rsi),%rax
	addq	%rax,%rdx

	movq	$60,%rax
	shlxq	%rax,72(%rsi),%rax
	addq	%rax,%rdx


	movq	%rdx,24(%rdi)

	.byte	0xf3,0xc3
