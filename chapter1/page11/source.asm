; original solution: Johannes Bader https://johannesbader.ch/blog/practical-reverse-engineering-exercises-page-11/

include '..\include\win32ax.inc' ; you can simply switch between win32ax, win32wx, win64ax and win64wx here

.code
  start:
            push 'x'                ; second function parameter
            push dword string       ; first function parameter
            call black_out          ; call function
            invoke  ExitProcess,0   ; call ExitProcess

  black_out:
            push ebp                ; function prologue, save stack base pointer
            mov ebp, esp            ; point base pointer to ESP    
            ; ------------ start code from book ---------
            mov edi, [ebp+8]        ; put first parameter (char*) in edi 
            mov edx, edi            ; make a copy of edi (scasb will change edi)
            xor eax, eax            ; set eax to 0 (i.e., null byte)
            or ecx, 0FFFFFFFFh      ; set ecs to 0xFFFFFFFF = (-1)
            repne scasb             ; search eax (null byte) in memory at edi (first)
                                    ; parameter), decrement ecx for each tested char.
            add ecx, 2              ; increment ecx by 2 (to compensate for starting
                                    ; at -1, and the final null byte
            neg ecx                 ; change sign of ecx. ecx is now length of string
            mov al, [ebp+0Ch]       ; copy second parameter (char) to al (byte)
            mov edi, edx            ; restore edi from backup (points at string again)
            rep stosb               ; write byte at eax (second function parameter) to
                                    ; memory at edi (first function parameter), for edi
                                    ; times (length of string)
            mov eax, edx            ; set eax (return value) to address of (changed) string
            ; ------------ end code from book -----------
            mov esp, ebp            ; restore stack pointer
            pop ebp                 ; restore stack base pointer
            ret


.end start
.data
string: db     'amor omnia vincit', 0
