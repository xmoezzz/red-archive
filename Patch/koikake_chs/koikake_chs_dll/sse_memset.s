.model flat
.code

_memset32_sse2 PROC
    
    ; Creates a stack frame, so we can save registers, making them available
    ; to use. Otherwise, only 3 registers are safe, which is not enough here
    push    ebp
    mov     ebp,        esp
    
    ; Saves EDI, ESI and EBX as we are going to use them
    push    edi
    push    esi
    push    ebx
    
    ; Gets the arguments from the stack
    mov     edi,        [ ebp +  8 ]
    mov     esi,        [ ebp + 12 ]
    mov     edx,        [ ebp + 16 ]
    
    ; Saves the original memory pointer, as we'll need to return it
    mov     ebx,        edi
    
    ; Checks for a NULL memory pointer
    test    edi,        edi
    jz      _end
    
    ; Checks for a zero size
    test    edx,        edx
    jz      _end
    
    ; memset() is often called to set a buffer to zero, so prepare xmm0
    ; for such a case
    pxor    xmm0,       xmm0
    
    ; Checks if bytes needs to be set to zero
    test    esi,        esi
    jz      _padded
    
    _pad:
        
        ; A character other than zero will be used. Fills the ESI registers
        ; with four time the character value, so it will be easier to fill
        ; a XMM register with the character to set
        mov     ecx,        esi
        shl     esi,        8
        or      esi,        ecx
        mov     ecx,        esi
        shl     esi,        16
        or      esi,        ecx
        
        ; Fills the XMM0 register with the character to write, so we'll be
        ; able to write 16 bytes at a time
        movd    xmm0,       esi
        shufps  xmm0,       xmm0,   0
        
    _padded:
        
        ; Aligns the memory pointer in EDI to a 16-byte boundary,
        ; so we can safelfy use the SSE instructions
        and     edi,        -16
        
        ; Gets the number of misaligned bytes in the original memory pointer (EBX)
        mov     eax,        ebx
        sub     eax,        edi
        
        ; Checks if the pointer is already aligned
        test     eax,        eax
        jz      _aligned
        
        ; If not, computes the number of bytes to be written until we are
        ; aligned on the next 16-byte boundary
        mov     ecx,        16
        sub     ecx,        eax
        
    _notaligned:
        
        ; Restores the original pointer in EDI
        mov     edi,        ebx
        
        ; Stores the character to write in EAX, so we can access it as a byte
        ; using AL
        mov     eax,        esi
        
    _notaligned_loop:
        
        ; Checks if we have bytes to write
        test     edx,        edx
        jz      _end
        
        ; Writes a byte into the memory buffer
        mov     [ edi ],    al
        
        ; Advances the memory pointer
        inc     edi
        
        ; Decreases the number of bytes to write (EDX is the total, ECX is the
        ; number of bytes to write until we're aligned to a 16-byte boundary)
        dec     ecx
        dec     edx
        
        ; Checks if we are aligned to a 16-byte boundary. If so, the SSE
        ; instructions can be safely used
        test     ecx,        ecx
        jz      _aligned
        jmp     _notaligned_loop
        
    _aligned:
        
        ; Writes 128 bytes at a time, if possible
        cmp     edx,        128
        jge     _aligned_128
        
        ; Writes 64 bytes at a time, if possible
        cmp     edx,        64
        jge     _aligned_64
        
        ; Writes 32 bytes at a time, if possible
        cmp     edx,        32
        jge     _aligned_32
    
        ; Writes 16 bytes at a time, if possible
        cmp     edx,        16
        jge     _aligned_16
        
    _aligned_end:
        
        ; We're aligned on a 16-byte boundary, but there's not enough bytes
        ; to write in order to use the SSE instructions, so writes the
        ; remaining bytes one by one
        mov     ecx,            edx
        mov     eax,            esi
        jmp     _notaligned_loop
        
    _aligned_128:
        
        ; Writes 128 bytes into the memory buffer
        movdqa  [ edi       ],  xmm0
        movdqa  [ edi +  16 ],  xmm0
        movdqa  [ edi +  32 ],  xmm0
        movdqa  [ edi +  48 ],  xmm0
        movdqa  [ edi +  64 ],  xmm0
        movdqa  [ edi +  80 ],  xmm0
        movdqa  [ edi +  96 ],  xmm0
        movdqa  [ edi + 112 ],  xmm0
        
        ; Advances the memory pointer and decreases the number of bytes to write
        add     edi,            128
        sub     edx,            128
        
        ; Next bytes - Writes 128 bytes at a time, if possible
        cmp     edx,            128
        jge     _aligned_128
        
        ; Next bytes - Writes 64 bytes at a time, if possible
        cmp     edx,            64
        jge     _aligned_64
        
        ; Next bytes - Writes 32 bytes at a time, if possible
        cmp     edx,            32
        jge     _aligned_32
        
        ; Next bytes - Writes 16 bytes at a time, if possible
        cmp     edx,            16
        jge     _aligned_16
        
        ; Checks if we're done writing bytes
        test    edx,            edx
        jz      _end
        
        ; If we still have byte to write, writes them one by one
        jmp     _aligned_end
        
    _aligned_64:
        
        ; Writes 64 bytes into the memory buffer
        movdqa  [ edi      ],   xmm0
        movdqa  [ edi + 16 ],   xmm0
        movdqa  [ edi + 32 ],   xmm0
        movdqa  [ edi + 48 ],   xmm0
        
        ; Advances the memory pointer and decreases the number of bytes to write
        add     edi,            64
        sub     edx,            64
        
        ; Next bytes - Writes 64 bytes at a time, if possible
        cmp     edx,            64
        jge     _aligned_64
        
        ; Next bytes - Writes 32 bytes at a time, if possible
        cmp     edx,            32
        jge     _aligned_32
        
        ; Next bytes - Writes 16 bytes at a time, if possible
        cmp     edx,            16
        jge     _aligned_16
        
        ; Checks if we're done writing bytes
        test    edx,            edx
        jz      _end
        
        ; If we still have byte to write, writes them one by one
        jmp     _aligned_end
        
    _aligned_32:
        
        ; Writes 32 bytes into the memory buffer
        movdqa  [ edi      ],   xmm0
        movdqa  [ edi + 16 ],   xmm0
        
        ; Advances the memory pointer and decreases the number of bytes to write
        add     edi,            32
        sub     edx,            32
        
        ; Next bytes - Writes 32 bytes at a time, if possible
        cmp     edx,            32
        jge     _aligned_32
        
        ; Next bytes - Writes 16 bytes at a time, if possible
        cmp     edx,            16
        jge     _aligned_16
        
        ; Checks if we're done writing bytes
        test    edx,            edx
        jz      _end
        
        ; If we still have byte to write, writes them one by one
        jmp     _aligned_end
        
    _aligned_16:
        
        ; Writes 16 bytes into the memory buffer
        movdqa  [ edi ],        xmm0
        
        ; Advances the memory pointer and decreases the number of bytes to write
        add     edi,            16
        sub     edx,            16
        
        ; Next bytes - Writes 16 bytes at a time, if possible
        cmp     edx,            16
        jge     _aligned
        
        ; Checks if we're done writing bytes
        test    edx,            edx
        jz      _end
        
        ; If we still have byte to write, writes them one by one
        jmp     _aligned_end
    
    _end:
        
        ; Return value - Gets the original pointer
        mov     eax,            ebx
        
        ; Restores saved registers
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        
        ret

		_memset32_sse2 ENDP

		END