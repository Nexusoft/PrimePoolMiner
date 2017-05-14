


.macro vpimac   Src1, Src2, Dst
    vpmuludq    \Src1, \Src2, TEMP1
    vpaddq      TEMP1, \Dst, \Dst
.endm

.macro vpimac4   Src1, Src2, Src3, Dst
    vpmuludq    \Src1, \Src2, TEMP1
    vpaddq      TEMP1, \Src3, \Dst
.endm

.macro alumac   Src, Dst
    mulx   \Src, %rax, tmp
    add    %rax, \Dst
.endm


#data
.align 32
.LShiftMask_1024:
.quad 29,29,29,64
.LAndMask_1024:
.quad 0x1fffffff,0x1fffffff,0x1fffffff,0x1fffffff
.LAndMask_1024b:
.quad 0x1fffffff,0x1fffffff,0x1fffffff,0xffffffffffffffff
.type   .LAndMask_1024,@object
.size   .LAndMask_1024,32


.type AMM,@function
.globl AMM
.align 64
AMM:

#void AMS_WW_1024(
.set rp,  %rdi   # uint64_t *rp,
.set ap,  %rsi   # const uint64_t *ap,
.set bpi, %rdx   # const uint64_t *bp,
.set np,  %rcx   # const uint64_t *np,
.set n0,  %r8    # const uint64_t n0);

# The registers that hold the accumulated redundant result
# The AMM works on 1024 bit operands, and redundant word size is 29
# Therefore: ceil(1024/29)/4 = 9

.set ACC0, %ymm0
.set ACC0_xmm, %xmm0
.set ACC1, %ymm1
.set ACC2, %ymm2
.set ACC3, %ymm3
.set ACC4, %ymm4
.set ACC5, %ymm5
.set ACC6, %ymm6
.set ACC7, %ymm7
.set ACC8, %ymm8
.set ACC9, %ymm9

.set B1, %ymm10
.set B1_xmm, %xmm10

.set Y1, %ymm12
.set Y1_xmm, %xmm12

.set ZERO, %ymm13

.set TEMP0, %ymm11
.set TEMP1, %ymm14
.set TEMP1_xmm, %xmm14

.set r0, %r9
.set r1, %r10
.set r2, %r11
.set r3, %r12

.set bp, %r13
.set i, %r14
.set tmp, %r15

.set shifted_np1, 32*9
.set shifted_np2, 32*18
.set shifted_np3, 32*27

	push    %r12
	push	%r13
	push	%r14
	push	%r15
    
    # free rdx to be used by mulx
    mov bpi, bp
    vzeroall
    
    xor r0, r0
    xor r1, r1
    xor r2, r2
    xor r3, r3
    
# There are 36 redundant words we need to multiply by, each iteration
# there are 4 such multiplications + reductions, therefore total of 9 iterations
# however after 7 iterations it is necessary to make a little fix to the ACC 
# to avoid overflow

    mov $9, i
       
    jmp .Lamm1024_loop
    
.align 32
.Lamm1024_loop:
        # small fix to avoid overflow 
        vpsrlvq  .LShiftMask_1024(%rip), ACC3, TEMP0
        vpand   .LAndMask_1024b(%rip), ACC3, ACC3
        
        ########################################################################
        mov 8*0(bp), %rdx
        vpbroadcastq    8*0(bp), B1
        
        # multiply low 4 words of a by bi
        alumac 8*0(ap), r0
        alumac 8*1(ap), r1
        alumac 8*2(ap), r2
        alumac 8*3(ap), r3
        
        # compute n0 * ri mod 2^29
        mov     r0, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        vmovq   %rdx, Y1_xmm
        vpermq  $0, Y1, Y1
        
        # multiply the rest of a by bi
        vpimac  32*1-8*0(ap), B1, ACC1
        vpimac  32*2-8*0(ap), B1, ACC2
        vpimac  32*3-8*0(ap), B1, ACC3
        vpimac  32*4-8*0(ap), B1, ACC4
        vpimac  32*5-8*0(ap), B1, ACC5
        vpimac  32*6-8*0(ap), B1, ACC6
        vpimac  32*7-8*0(ap), B1, ACC7
        vpimac  32*8-8*0(ap), B1, ACC8
        vpermq  $0x93, TEMP0, TEMP0
        
        # multiply (n0 * ri mod 2^29) by r, only for the low 4 words
        alumac 8*0(np), r0
        alumac 8*1(np), r1
        alumac 8*2(np), r2
        alumac 8*3(np), r3
        
        # now the low 29 bit of r0 are zeroed, the top bits we add to r1
        shr $29, r0
        add r0, r1
        
        # multiply the rest of r by (n0 * ri mod 2^29)
        vpimac  32*1(np), Y1, ACC1
        vpimac  32*2(np), Y1, ACC2
        vpimac  32*3(np), Y1, ACC3
        vpimac  32*4(np), Y1, ACC4
        vpimac  32*5(np), Y1, ACC5
        vpimac  32*6(np), Y1, ACC6
        vpimac  32*7(np), Y1, ACC7
        vpimac  32*8(np), Y1, ACC8
        vpaddq  TEMP0, ACC3, ACC3
        ########################################################################
        mov 8*1(bp), %rdx
        vpbroadcastq    8*1(bp), B1       
        
        # multiply low 3 words of a by bi
        alumac 8*0(ap), r1
        alumac 8*1(ap), r2
        alumac 8*2(ap), r3
        
        # compute n0 * ri mod 2^29
        mov     r1, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        vmovq   %rdx, Y1_xmm
        vpermq  $0, Y1, Y1
        
        # multiply the rest of a by bi
        vpimac  32*1-8*1(ap), B1, ACC1
        vpimac  32*2-8*1(ap), B1, ACC2
        vpimac  32*3-8*1(ap), B1, ACC3
        vpimac  32*4-8*1(ap), B1, ACC4
        vpimac  32*5-8*1(ap), B1, ACC5
        vpimac  32*6-8*1(ap), B1, ACC6
        vpimac  32*7-8*1(ap), B1, ACC7
        vpimac  32*8-8*1(ap), B1, ACC8
        vpmuludq   32*9-8*1(ap), B1, ACC9
        
        alumac 8*0(np), r1
        alumac 8*1(np), r2
        alumac 8*2(np), r3
        
        # now the low 29 bit of r1 are zeroed, the top bits we add to r2
        shr $29, r1
        add r1, r2
        
        vpimac  32*0+shifted_np1(np), Y1, ACC1
        vpimac  32*1+shifted_np1(np), Y1, ACC2
        vpimac  32*2+shifted_np1(np), Y1, ACC3
        vpimac  32*3+shifted_np1(np), Y1, ACC4
        vpimac  32*4+shifted_np1(np), Y1, ACC5
        vpimac  32*5+shifted_np1(np), Y1, ACC6
        vpimac  32*6+shifted_np1(np), Y1, ACC7
        vpimac  32*7+shifted_np1(np), Y1, ACC8
        vpimac  32*8+shifted_np1(np), Y1, ACC9
        ########################################################################
        mov 8*2(bp), %rdx
        vpbroadcastq   8*2(bp), B1       
        
        # multiply low 2 words of a by bi
        alumac 8*0(ap), r2
        alumac 8*1(ap), r3
        
        # compute n0 * ri mod 2^29
        mov     r2, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        vmovq   %rdx, Y1_xmm
        vpermq  $0, Y1, Y1
        
        # multiply the rest of a by bi
        vpimac  32*1-8*2(ap), B1, ACC1
        vpimac  32*2-8*2(ap), B1, ACC2
        vpimac  32*3-8*2(ap), B1, ACC3
        vpimac  32*4-8*2(ap), B1, ACC4
        vpimac  32*5-8*2(ap), B1, ACC5
        vpimac  32*6-8*2(ap), B1, ACC6
        vpimac  32*7-8*2(ap), B1, ACC7
        vpimac  32*8-8*2(ap), B1, ACC8
        vpimac  32*9-8*2(ap), B1, ACC9
        
        alumac 8*0(np), r2
        alumac 8*1(np), r3
        
        # now the low 29 bit of r2 are zeroed, the top bits we add to r3
        shr $29, r2
        add r2, r3
        
        vpimac  32*0+shifted_np2(np), Y1, ACC1
        vpimac  32*1+shifted_np2(np), Y1, ACC2
        vpimac  32*2+shifted_np2(np), Y1, ACC3
        vpimac  32*3+shifted_np2(np), Y1, ACC4
        vpimac  32*4+shifted_np2(np), Y1, ACC5
        vpimac  32*5+shifted_np2(np), Y1, ACC6
        vpimac  32*6+shifted_np2(np), Y1, ACC7
        vpimac  32*7+shifted_np2(np), Y1, ACC8
        vpimac  32*8+shifted_np2(np), Y1, ACC9
        ########################################################################
        mov 8*3(bp), %rdx
        vpbroadcastq    8*3(bp), B1       
        
        # multiply low word of a by bi
        alumac 8*0(ap), r3
        
        # compute n0 * ri mod 2^29
        mov     r3, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        vmovq   %rdx, Y1_xmm
        vpermq  $0, Y1, Y1
        
        # multiply the rest of a by bi
        vpimac  32*1-8*3(ap), B1, ACC1
        vpimac  32*2-8*3(ap), B1, ACC2
        vpimac  32*3-8*3(ap), B1, ACC3
        vpimac  32*4-8*3(ap), B1, ACC4
        vpimac  32*5-8*3(ap), B1, ACC5
        vpimac  32*6-8*3(ap), B1, ACC6
        vpimac  32*7-8*3(ap), B1, ACC7
        vpimac  32*8-8*3(ap), B1, ACC8
        vpimac  32*9-8*3(ap), B1, ACC9
        
        alumac 8*0(np), r3
        
        # now the low 29 bit of r2 are zeroed, the top bits we add to r3
        shr $29, r3
        mov r3, tmp
        
        vpimac4 32*0+shifted_np3(np), Y1, ACC1, ACC0
        vpimac4 32*1+shifted_np3(np), Y1, ACC2, ACC1
        vpimac4 32*2+shifted_np3(np), Y1, ACC3, ACC2
        vpimac4 32*3+shifted_np3(np), Y1, ACC4, ACC3
        vpimac4 32*4+shifted_np3(np), Y1, ACC5, ACC4
        vpimac4 32*5+shifted_np3(np), Y1, ACC6, ACC5
        vpimac4 32*6+shifted_np3(np), Y1, ACC7, ACC6
        vpimac4 32*7+shifted_np3(np), Y1, ACC8, ACC7
        vpimac4 32*8+shifted_np3(np), Y1, ACC9, ACC8
        
        # now we want to transfer the 4 words in ACC0, which became the low 4
        # to ALU registers        
        vextracti128 $1, ACC0, TEMP1_xmm
        vmovq   ACC0_xmm, r0
        vpextrq $1, ACC0_xmm, r1
        vmovq   TEMP1_xmm, r2
        vpextrq $1, TEMP1_xmm, r3
        
        # finally add the former r3, to the new r0
        add tmp, r0
        
        lea 32(bp), bp
        dec i
    jne .Lamm1024_loop

.set TEMP2, ACC9
.set TEMP3, B1
.set TEMP4, Y1
    
    # now fix the output to be *almost* radix 2^29
    vpxor   TEMP1, TEMP1, TEMP1
    vmovq   tmp, TEMP1_xmm
    vpaddq  TEMP1, ACC0, ACC0
    vpxor   ZERO, ZERO, ZERO
    
    vpsrlq  $29, ACC0, TEMP0
    vpsrlq  $29, ACC1, TEMP1
    vpsrlq  $29, ACC2, TEMP2
    vpsrlq  $29, ACC3, TEMP3
    
    vpand   .LAndMask_1024(%rip), ACC0, ACC0
    vpand   .LAndMask_1024(%rip), ACC1, ACC1
    vpand   .LAndMask_1024(%rip), ACC2, ACC2
    vpand   .LAndMask_1024(%rip), ACC3, ACC3
    
    vpermq  $0x93, TEMP0, TEMP0
    vpermq  $0x93, TEMP1, TEMP1
    vpermq  $0x93, TEMP2, TEMP2
    vpermq  $0x93, TEMP3, TEMP3
    
    vpblendd $3, TEMP3, ZERO, TEMP4
    vpblendd $3, TEMP2, TEMP3, TEMP3
    vpblendd $3, TEMP1, TEMP2, TEMP2
    vpblendd $3, TEMP0, TEMP1, TEMP1
    vpblendd $3, ZERO, TEMP0, TEMP0
    
    vpaddq  TEMP0, ACC0, ACC0
    vpaddq  TEMP1, ACC1, ACC1
    vpaddq  TEMP2, ACC2, ACC2
    vpaddq  TEMP3, ACC3, ACC3
    vpaddq  TEMP4, ACC4, ACC4
    
    vpsrlq  $29, ACC0, TEMP0
    vpsrlq  $29, ACC1, TEMP1
    vpsrlq  $29, ACC2, TEMP2
    vpsrlq  $29, ACC3, TEMP3
    
    vpand   .LAndMask_1024(%rip), ACC0, ACC0
    vpand   .LAndMask_1024(%rip), ACC1, ACC1
    vpand   .LAndMask_1024(%rip), ACC2, ACC2
    vpand   .LAndMask_1024(%rip), ACC3, ACC3
    
    vpermq  $0x93, TEMP0, TEMP0
    vpermq  $0x93, TEMP1, TEMP1
    vpermq  $0x93, TEMP2, TEMP2
    vpermq  $0x93, TEMP3, TEMP3
    
    vpblendd $3, TEMP3, ZERO, TEMP4
    vpblendd $3, TEMP2, TEMP3, TEMP3
    vpblendd $3, TEMP1, TEMP2, TEMP2
    vpblendd $3, TEMP0, TEMP1, TEMP1
    vpblendd $3, ZERO, TEMP0, TEMP0
    
    vpaddq  TEMP0, ACC0, ACC0
    vpaddq  TEMP1, ACC1, ACC1
    vpaddq  TEMP2, ACC2, ACC2
    vpaddq  TEMP3, ACC3, ACC3
    vpaddq  TEMP4, ACC4, ACC4
    
    vpsrlq  $29, ACC4, TEMP0
    vpsrlq  $29, ACC5, TEMP1
    vpsrlq  $29, ACC6, TEMP2
    vpsrlq  $29, ACC7, TEMP3
    vpsrlq  $29, ACC8, TEMP4
    
    vpand   .LAndMask_1024(%rip), ACC4, ACC4
    vpand   .LAndMask_1024(%rip), ACC5, ACC5
    vpand   .LAndMask_1024(%rip), ACC6, ACC6
    vpand   .LAndMask_1024(%rip), ACC7, ACC7
    vpand   .LAndMask_1024(%rip), ACC8, ACC8
    
    vpermq  $0x93, TEMP0, TEMP0
    vpermq  $0x93, TEMP1, TEMP1
    vpermq  $0x93, TEMP2, TEMP2
    vpermq  $0x93, TEMP3, TEMP3
    vpermq  $0x93, TEMP4, TEMP4
    
    vpblendd $3, TEMP3, TEMP4, TEMP4
    vpblendd $3, TEMP2, TEMP3, TEMP3
    vpblendd $3, TEMP1, TEMP2, TEMP2
    vpblendd $3, TEMP0, TEMP1, TEMP1
    vpblendd $3, ZERO, TEMP0, TEMP0
    
    vpaddq  TEMP0, ACC4, ACC4
    vpaddq  TEMP1, ACC5, ACC5
    vpaddq  TEMP2, ACC6, ACC6
    vpaddq  TEMP3, ACC7, ACC7
    vpaddq  TEMP4, ACC8, ACC8
    
    vpsrlq  $29, ACC4, TEMP0
    vpsrlq  $29, ACC5, TEMP1
    vpsrlq  $29, ACC6, TEMP2
    vpsrlq  $29, ACC7, TEMP3
    vpsrlq  $29, ACC8, TEMP4
    
    vpand   .LAndMask_1024(%rip), ACC4, ACC4
    vpand   .LAndMask_1024(%rip), ACC5, ACC5
    vpand   .LAndMask_1024(%rip), ACC6, ACC6
    vpand   .LAndMask_1024(%rip), ACC7, ACC7
    vpand   .LAndMask_1024(%rip), ACC8, ACC8
    
    vpermq  $0x93, TEMP0, TEMP0
    vpermq  $0x93, TEMP1, TEMP1
    vpermq  $0x93, TEMP2, TEMP2
    vpermq  $0x93, TEMP3, TEMP3
    vpermq  $0x93, TEMP4, TEMP4
    
    vpblendd $3, TEMP3, TEMP4, TEMP4
    vpblendd $3, TEMP2, TEMP3, TEMP3
    vpblendd $3, TEMP1, TEMP2, TEMP2
    vpblendd $3, TEMP0, TEMP1, TEMP1
    vpblendd $3, ZERO, TEMP0, TEMP0
    
    vpaddq  TEMP0, ACC4, ACC4
    vpaddq  TEMP1, ACC5, ACC5
    vpaddq  TEMP2, ACC6, ACC6
    vpaddq  TEMP3, ACC7, ACC7
    vpaddq  TEMP4, ACC8, ACC8
    
    vmovdqu ACC0, 32*0(rp)
    vmovdqu ACC1, 32*1(rp)
    vmovdqu ACC2, 32*2(rp)
    vmovdqu ACC3, 32*3(rp)
    vmovdqu ACC4, 32*4(rp)
    vmovdqu ACC5, 32*5(rp)
    vmovdqu ACC6, 32*6(rp)
    vmovdqu ACC7, 32*7(rp)
    vmovdqu ACC8, 32*8(rp)
    
    bail:
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
    
    ret
    
