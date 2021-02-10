include '..\include\win32ax.inc'

.code
  start:
        call alter_me
        invoke  ExitProcess,0
alter_me:
        push ebp
        mov ebp, esp
        mov esp, ebp
        pop ebp
        push 0xAABBCCDD
        ret
.end start
                     
