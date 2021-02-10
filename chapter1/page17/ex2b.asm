include '..\include\win32ax.inc'

.code
  start:
        push 0xAABBCCDD
        call alter_me
        invoke  ExitProcess,0
alter_me:
        push ebp
        mov ebp,esp
        mov esp,ebp
        pop ebp
        pop ebx
        ret
.end start
