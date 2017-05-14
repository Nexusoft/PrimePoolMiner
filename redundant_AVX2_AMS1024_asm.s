


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
.LAndMask_1024:
.quad 0x1fffffff,0x1fffffff,0x1fffffff,0x1fffffff
.type   .LAndMask_1024,@object
.size   .LAndMask_1024,32


.type AMS,@function
.globl AMS
.align 64
AMS:

#void AMS_WW_1024(
.set rp,  %rdi   # uint64_t *rp,
.set ap,  %rsi   # const uint64_t *ap,
.set npi, %rdx   # const uint64_t *np,
.set n0,  %rcx   # const uint64_t n0,
.set rep, %r8d   # int repeat);

# The registers that hold the accumulated redundant result
# The AMS works on 1024 bit operands, and redundant word size is 29
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

.set Y1, %ymm12
.set Y1_xmm, %xmm12
.set Y2, %ymm13
.set Y2_xmm, %xmm13

.set TEMP0, %ymm11
.set TEMP1, %ymm14
.set TEMP1_xmm, %xmm14

.set r0, %r9
.set r1, %r10
.set r2, %r11
.set r3, %r12

.set np, %r13
.set i, %r14
.set tmp, %r15

.set tp, (%rsp)
.set shifted_ap, 576
.set shifted_np1, 32*9
.set shifted_np2, 32*18
.set shifted_np3, 32*27
.set stackSize, (576+256)

    push    %rbp
	push    %r12
	push	%r13
	push	%r14
	push	%r15
    
	mov     %rsp, %rbp
    # allocate space on stack for temp result, and for the shifted value of a
	sub     $stackSize, %rsp
	and     $-64, %rsp
    
# free rdx to be used by mulx
    mov npi, np
    vzeroall
    vmovdqu 32*0(ap), ACC0
    vmovdqu 32*1(ap), ACC1
    vmovdqu 32*2(ap), ACC2
    vmovdqu 32*3(ap), ACC3
    vmovdqu 32*4(ap), ACC4
    vmovdqu 32*5(ap), ACC5
    vmovdqu 32*6(ap), ACC6
    vmovdqu 32*7(ap), ACC7
    vmovdqu 32*8(ap), ACC8
    
# this is the entry point for the external repeat loop

.Lams1024_rep:

    xor tmp, tmp

# the square is performed as described in Variant B of "Speeding up Big-Number Squaring"
# calculate the A*2 vector (for the relevant parts only)
    vpsllq  $1, ACC1, ACC1
    vpsllq  $1, ACC2, ACC2
    vpsllq  $1, ACC3, ACC3
    vpsllq  $1, ACC4, ACC4
    vpsllq  $1, ACC5, ACC5
    vpsllq  $1, ACC6, ACC6
    vpsllq  $1, ACC7, ACC7
    vpsllq  $1, ACC8, ACC8
# store shifted values of A
    vmovdqa ACC1, 576+32*0(%rsp)
    vmovdqa ACC2, 576+32*1(%rsp)
    vmovdqa ACC3, 576+32*2(%rsp)
    vmovdqa ACC4, 576+32*3(%rsp)
    vmovdqa ACC5, 576+32*4(%rsp)
    vmovdqa ACC6, 576+32*5(%rsp)
    vmovdqa ACC7, 576+32*6(%rsp)
    vmovdqa ACC8, 576+32*7(%rsp)
    
    # B1 = a0||a0||a0||a0
    vpbroadcastq    8*0(ap), B1
    
    # multiply all words of a by a0, as optimization, 
    # use the values of a*2 where needed, to save future multiplications
    vpmuludq    ACC0, B1, ACC0
    vpmuludq    ACC1, B1, ACC1
    vpmuludq    ACC2, B1, ACC2
    vpmuludq    ACC3, B1, ACC3
    vpmuludq    ACC4, B1, ACC4
    vpmuludq    ACC5, B1, ACC5
    vpmuludq    ACC6, B1, ACC6
    vpmuludq    ACC7, B1, ACC7
    vpmuludq    ACC8, B1, ACC8
    
    # nothing to do for ACC0 and ACC1, store them, so we can reuse the registers
    vmovdqa ACC0, 32*0(tp)
    vmovdqa ACC1, 32*1(tp)
    
    
    # multiplying by a1 now would require to realign the ACCs, instead we multiply
    # by a4, to get similarly aligned ACCs
    
    # B1 = a4||a4||a4||a4
    vpbroadcastq    8*4(ap), B1
    
    vpimac      32*1(ap), B1, ACC2 
    vpimac      shifted_ap+32*1(%rsp), B1, ACC3
    vpimac      shifted_ap+32*2(%rsp), B1, ACC4
    vpimac      shifted_ap+32*3(%rsp), B1, ACC5
    vpimac      shifted_ap+32*4(%rsp), B1, ACC6
    vpimac      shifted_ap+32*5(%rsp), B1, ACC7
    vpimac      shifted_ap+32*6(%rsp), B1, ACC8
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC0
    
    vmovdqa ACC2, 32*2(tp)
    vmovdqa ACC3, 32*3(tp)
    
    # continue in the same manner, with a8, a12, a16 ... a32
    
    vpbroadcastq    8*8(ap), B1
    
    vpimac      32*2(ap), B1, ACC4
    vpimac      shifted_ap+32*2(%rsp), B1, ACC5
    vpimac      shifted_ap+32*3(%rsp), B1, ACC6
    vpimac      shifted_ap+32*4(%rsp), B1, ACC7
    vpimac      shifted_ap+32*5(%rsp), B1, ACC8
    vpimac      shifted_ap+32*6(%rsp), B1, ACC0
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC1
    
    vmovdqa ACC4, 32*4(tp)
    vmovdqa ACC5, 32*5(tp)
    ###
    vpbroadcastq    8*12(ap), B1
    
    vpimac      32*3(ap), B1, ACC6
    vpimac      shifted_ap+32*3(%rsp), B1, ACC7
    vpimac      shifted_ap+32*4(%rsp), B1, ACC8
    vpimac      shifted_ap+32*5(%rsp), B1, ACC0
    vpimac      shifted_ap+32*6(%rsp), B1, ACC1
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC2
    
    vmovdqa ACC6, 32*6(tp)
    vmovdqa ACC7, 32*7(tp)
    ###
    vpbroadcastq    8*16(ap), B1
    
    vpimac      32*4(ap), B1, ACC8
    vpimac      shifted_ap+32*4(%rsp), B1, ACC0
    vpimac      shifted_ap+32*5(%rsp), B1, ACC1
    vpimac      shifted_ap+32*6(%rsp), B1, ACC2
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC3
    
    vmovdqa ACC8, 32*8(tp)
    vmovdqa ACC0, 32*9(tp)
    ###
    vpbroadcastq    8*20(ap), B1
    
    vpimac      32*5(ap), B1, ACC1
    vpimac      shifted_ap+32*5(%rsp), B1, ACC2
    vpimac      shifted_ap+32*6(%rsp), B1, ACC3
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC4
    
    vmovdqa ACC1, 32*10(tp)
    vmovdqa ACC2, 32*11(tp)
    ###
    vpbroadcastq    8*24(ap), B1
    
    vpimac      32*6(ap), B1, ACC3
    vpimac      shifted_ap+32*6(%rsp), B1, ACC4
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC5
    
    vmovdqa ACC3, 32*12(tp)
    vmovdqa ACC4, 32*13(tp)
    ###
    vpbroadcastq    8*28(ap), B1
    
    vpimac      32*7(ap), B1, ACC5
    vpmuludq    shifted_ap+32*7(%rsp), B1, ACC6
    
    vmovdqa ACC5, 32*14(tp)
    vmovdqa ACC6, 32*15(tp)
    ###
    vpbroadcastq    8*32(ap), B1
    
    vpmuludq    32*8(ap), B1, ACC7
    vpxor       ACC8, ACC8, ACC8
    
    vmovdqa ACC7, 32*16(tp)
    vmovdqa ACC8, 32*17(tp)
    
    add $8, tmp
    mov $3, i
    
    jmp .Lams1024_sqr
    
    .align 32
.Lams1024_sqr:

        # now we proceed with multiplications by a1, a5 ... a33 in the first iteration
        # then by a2, a6, ... a34, and then by a3, a7 ... a35, this way we keep the ACCs
        # relatively aligned throughout the iteration
        
        vmovdqu 32*0(tp, tmp), ACC0
        vmovdqu 32*1(tp, tmp), ACC1
        vmovdqu 32*2(tp, tmp), ACC2
        vmovdqu 32*3(tp, tmp), ACC3
        vmovdqu 32*4(tp, tmp), ACC4
        vmovdqu 32*5(tp, tmp), ACC5
        vmovdqu 32*6(tp, tmp), ACC6
        vmovdqu 32*7(tp, tmp), ACC7
        vmovdqu 32*8(tp, tmp), ACC8
        ###
        vpbroadcastq    8*0(ap, tmp), B1
        
        vpimac  32*0(ap), B1, ACC0
        vpimac  shifted_ap+32*0(%rsp), B1, ACC1
        vpimac  shifted_ap+32*1(%rsp), B1, ACC2
        vpimac  shifted_ap+32*2(%rsp), B1, ACC3
        vpimac  shifted_ap+32*3(%rsp), B1, ACC4
        vpimac  shifted_ap+32*4(%rsp), B1, ACC5
        vpimac  shifted_ap+32*5(%rsp), B1, ACC6
        vpimac  shifted_ap+32*6(%rsp), B1, ACC7
        vpimac  shifted_ap+32*7(%rsp), B1, ACC8    
        
        vmovdqu ACC0, 32*0(tp, tmp)
        vmovdqu ACC1, 32*1(tp, tmp)
        ###
        vpbroadcastq    8*4(ap, tmp), B1
        vmovdqu 32*9(tp, tmp), ACC0
        
        vpimac  32*1(ap), B1, ACC2 
        vpimac  shifted_ap+32*1(%rsp), B1, ACC3
        vpimac  shifted_ap+32*2(%rsp), B1, ACC4
        vpimac  shifted_ap+32*3(%rsp), B1, ACC5
        vpimac  shifted_ap+32*4(%rsp), B1, ACC6
        vpimac  shifted_ap+32*5(%rsp), B1, ACC7
        vpimac  shifted_ap+32*6(%rsp), B1, ACC8
        vpimac  shifted_ap+32*7(%rsp), B1, ACC0
        
        vmovdqu ACC2, 32*2(tp, tmp)
        vmovdqu ACC3, 32*3(tp, tmp)
        ###
        vpbroadcastq    8*8(ap, tmp), B1
        vmovdqu 32*10(tp, tmp), ACC1
        
        vpimac  32*2(ap), B1, ACC4
        vpimac  shifted_ap+32*2(%rsp), B1, ACC5
        vpimac  shifted_ap+32*3(%rsp), B1, ACC6
        vpimac  shifted_ap+32*4(%rsp), B1, ACC7
        vpimac  shifted_ap+32*5(%rsp), B1, ACC8
        vpimac  shifted_ap+32*6(%rsp), B1, ACC0
        vpimac  shifted_ap+32*7(%rsp), B1, ACC1
        
        vmovdqu ACC4, 32*4(tp, tmp)
        vmovdqu ACC5, 32*5(tp, tmp)
        ###
        vpbroadcastq    8*12(ap, tmp), B1
        vmovdqu 32*11(tp, tmp), ACC2
        
        vpimac  32*3(ap), B1, ACC6
        vpimac  shifted_ap+32*3(%rsp), B1, ACC7
        vpimac  shifted_ap+32*4(%rsp), B1, ACC8
        vpimac  shifted_ap+32*5(%rsp), B1, ACC0
        vpimac  shifted_ap+32*6(%rsp), B1, ACC1
        vpimac  shifted_ap+32*7(%rsp), B1, ACC2
        
        vmovdqu ACC6, 32*6(tp, tmp)
        vmovdqu ACC7, 32*7(tp, tmp)
        ###
        vpbroadcastq    8*16(ap, tmp), B1
        vmovdqu 32*12(tp, tmp), ACC3
        
        vpimac  32*4(ap), B1, ACC8
        vpimac  shifted_ap+32*4(%rsp), B1, ACC0
        vpimac  shifted_ap+32*5(%rsp), B1, ACC1
        vpimac  shifted_ap+32*6(%rsp), B1, ACC2
        vpimac  shifted_ap+32*7(%rsp), B1, ACC3
        
        vmovdqu ACC8, 32*8(tp, tmp)
        vmovdqu ACC0, 32*9(tp, tmp)
        ###
        vpbroadcastq    8*20(ap, tmp), B1
        vmovdqu 32*13(tp, tmp), ACC4
        
        vpimac  32*5(ap), B1, ACC1
        vpimac  shifted_ap+32*5(%rsp), B1, ACC2
        vpimac  shifted_ap+32*6(%rsp), B1, ACC3
        vpimac  shifted_ap+32*7(%rsp), B1, ACC4
        
        vmovdqu ACC1, 32*10(tp, tmp)
        vmovdqu ACC2, 32*11(tp, tmp)
        ###
        vpbroadcastq    8*24(ap, tmp), B1
        vmovdqu 32*14(tp, tmp), ACC5
        
        vpimac  32*6(ap), B1, ACC3
        vpimac  shifted_ap+32*6(%rsp), B1, ACC4
        vpimac  shifted_ap+32*7(%rsp), B1, ACC5
        
        vmovdqu ACC3, 32*12(tp, tmp)
        vmovdqu ACC4, 32*13(tp, tmp)
        ###
        vpbroadcastq    8*28(ap, tmp), B1
        vmovdqu 32*15(tp, tmp), ACC6
    
        vpimac  32*7(ap), B1, ACC5
        vpimac  shifted_ap+32*7(%rsp), B1, ACC6
    
        vmovdqu ACC5, 32*14(tp, tmp)
        vmovdqu ACC6, 32*15(tp, tmp)
        ###
        vpbroadcastq    8*32(ap, tmp), B1
        vmovdqu 32*16(tp, tmp), ACC7
    
        vpimac    32*8(ap), B1, ACC7   
    
        vmovdqu ACC7, 32*16(tp, tmp)
        
        add $8, tmp
        dec i
    jne .Lams1024_sqr
        
.set ZERO, ACC9
.set TEMP2, B1
.set TEMP3, Y1
.set TEMP4, Y2

    # we need to fix indexes 31-38 of the result, to avoid overflow during reduction stage

    vmovdqu 8*31(tp), ACC0
    vmovdqu 8*35(tp), ACC1
    vmovdqu 8*39(tp), ACC2
    
    vpxor   ZERO, ZERO, ZERO
    
    vpsrlq  $29, ACC0, TEMP0
    vpsrlq  $29, ACC1, TEMP1
    
    vpand   .LAndMask_1024(%rip), ACC0, ACC0
    vpand   .LAndMask_1024(%rip), ACC1, ACC1
    
    vpermq  $0x93, TEMP0, TEMP0
    vpermq  $0x93, TEMP1, TEMP1
    
    vpblendd    $3, TEMP1, ZERO, TEMP2
    vpblendd    $3, TEMP0, TEMP1, TEMP1
    vpblendd    $3, ZERO, TEMP0, TEMP0
    
    vpaddq  TEMP0, ACC0, ACC0
    vpaddq  TEMP1, ACC1, ACC1
    vpaddq  TEMP2, ACC2, ACC2
    
    vmovdqu ACC0, 8*31(tp)
    vmovdqu ACC1, 8*35(tp)
    vmovdqu ACC2, 8*39(tp)
    # fix done
    
    # start the reduction phase, similarly to regular montgomery, but with word size of 29-bit
    
    # the low 4 words of the reduced operand are stored in ALU register, for lower latency
    mov 8*0(tp), r0
    mov 8*1(tp), r1
    mov 8*2(tp), r2
    mov 8*3(tp), r3
    # the rest are placed in SIMD registers for high throughput
    vmovdqa 32*1(tp), ACC1
    vmovdqa 32*2(tp), ACC2
    vmovdqa 32*3(tp), ACC3
    vmovdqa 32*4(tp), ACC4
    vmovdqa 32*5(tp), ACC5
    vmovdqa 32*6(tp), ACC6
    vmovdqa 32*7(tp), ACC7
    vmovdqa 32*8(tp), ACC8
    lea 32*9(tp), ap
    
    # compute n0 * r0 mod 2^29
    mov     r0, %rdx
    mulx    n0, %rdx, %rax
    and     $0x1fffffff, %rdx
    vmovq   %rdx, Y1_xmm
    vpermq  $0, Y1, Y1
    
    mov $9, i
    jmp .Lams1024_reduce
    
.align 32
.Lams1024_reduce:
    
        # multiply (n0 * ri mod 2^29) by r, only for the low 4 words
        alumac 8*0(np), r0
        alumac 8*1(np), r1
        alumac 8*2(np), r2
        alumac 8*3(np), r3
        
        # now the low 29 bit of r0 are zeroed, the top bits we add to r1
        shr $29, r0
        add r0, r1
        
        # compute n0 * r(i+1) mod 2^29
        mov     r1, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx  

        # multiply (n0 * ri mod 2^29) by r, for the rest of the vector
        # multiply (n0 * r(i+1) mod 2^29) by r, for the low 3 words
        vpimac  32*1(np), Y1, ACC1
        vpimac  32*2(np), Y1, ACC2
            vmovq   %rdx, Y2_xmm
            vpermq  $0, Y2, Y2
            alumac  8*0(np), r1
        vpimac  32*3(np), Y1, ACC3
            alumac  8*1(np), r2
        vpimac  32*4(np), Y1, ACC4
            alumac  8*2(np), r3
        vpimac  32*5(np), Y1, ACC5
        vpimac  32*6(np), Y1, ACC6
        vpimac  32*7(np), Y1, ACC7
        vpimac  32*8(np), Y1, ACC8
        # continue loading the following part of the ACC
        vmovdqa (ap), ACC9
        lea     32(ap), ap
        
        # now the low 29 bit of r1 are zeroed, the top bits we add to r2
        shr $29, r1
        add r1, r2
        
        # compute n0 * r(i+2) mod 2^29
        mov     r2, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        
        # multiply (n0 * r(i+1) mod 2^29) by r, for the rest of the vector
        # multiply (n0 * r(i+2) mod 2^29) by r, for the low 2 words
        vpimac  32*0+shifted_np1(np), Y2, ACC1
        vpimac  32*1+shifted_np1(np), Y2, ACC2
            vmovq   %rdx, Y1_xmm
            vpermq  $0, Y1, Y1
            alumac  8*0(np), r2
        vpimac  32*2+shifted_np1(np), Y2, ACC3
            alumac  8*1(np), r3
        vpimac  32*3+shifted_np1(np), Y2, ACC4
        vpimac  32*4+shifted_np1(np), Y2, ACC5
        vpimac  32*5+shifted_np1(np), Y2, ACC6
        vpimac  32*6+shifted_np1(np), Y2, ACC7
        vpimac  32*7+shifted_np1(np), Y2, ACC8
        vpimac  32*8+shifted_np1(np), Y2, ACC9
        
        # now the low 29 bit of r2 are zeroed, the top bits we add to r3
        shr $29, r2
        add r2, r3
        
        # compute n0 * r(i+3) mod 2^29
        mov     r3, %rdx
        mulx    n0, %rdx, %rax
        and     $0x1fffffff, %rdx
        
        # multiply (n0 * r(i+2) mod 2^29) by r, for the rest of the vector
        # multiply (n0 * r(i+3) mod 2^29) by r, for the low word
        vpimac  32*0+shifted_np2(np), Y1, ACC1
        vpimac  32*1+shifted_np2(np), Y1, ACC2
            vmovq   %rdx, Y2_xmm
            vpermq  $0, Y2, Y2
            alumac  8*0(np), r3
        vpimac  32*2+shifted_np2(np), Y1, ACC3
        vpimac  32*3+shifted_np2(np), Y1, ACC4
        vpimac  32*4+shifted_np2(np), Y1, ACC5
                # we want to resolve final value of ACC0 early
                vpimac4 32*0+shifted_np3(np), Y2, ACC1, ACC0
        vpimac  32*5+shifted_np2(np), Y1, ACC6
        vpimac  32*6+shifted_np2(np), Y1, ACC7
        vpimac  32*7+shifted_np2(np), Y1, ACC8
        vpimac  32*8+shifted_np2(np), Y1, ACC9
        
        # low 29 bits of r3 are zeroed, store the top bits
        shr $29, r3
        mov r3, tmp
        
        # now we want to transfer the 4 words in ACC0, which became the low 4
        # to ALU registers        
        vextracti128 $1, ACC0, TEMP1_xmm
        vmovq   ACC0_xmm, r0
        vpextrq $1, ACC0_xmm, r1
        vmovq   TEMP1_xmm, r2
        vpextrq $1, TEMP1_xmm, r3
        
        # finally add the former r3, to the new r0
        add tmp, r0
        
        vpimac4 32*1+shifted_np3(np), Y2, ACC2, ACC1
        vpimac4 32*2+shifted_np3(np), Y2, ACC3, ACC2
        vpimac4 32*3+shifted_np3(np), Y2, ACC4, ACC3
        vpimac4 32*4+shifted_np3(np), Y2, ACC5, ACC4
            # compute n0 * ri mod 2^29 for next iteration of the loop
            mov     r0, %rdx
            mulx    n0, %rdx, %rax
            and     $0x1fffffff, %rdx
        vpimac4 32*5+shifted_np3(np), Y2, ACC6, ACC5
        vpimac4 32*6+shifted_np3(np), Y2, ACC7, ACC6
        vpimac4 32*7+shifted_np3(np), Y2, ACC8, ACC7
        vpimac4 32*8+shifted_np3(np), Y2, ACC9, ACC8
        
            vmovq   %rdx, Y1_xmm
            vpermq  $0, Y1, Y1
        
        dec i
    jne .Lams1024_reduce
    
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
    
    mov rp, ap
    dec rep
    jne .Lams1024_rep
    bail:
    mov %rbp, %rsp
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbp
    
    ret
    
