format PE CONSOLE
entry start

include '%include%\win32a.inc'

section '.data' data readable writeable
msg db 'This is a printf example',0
format1 db '%s',0

section '.code' code readable executable
  start:
        xor eax,eax
        call get_eip
        cinvoke printf,formatstring,eax ;the 'get_eip' function returns the eip that points at this very line of code.
        invoke  ExitProcess,0

 get_eip:
        push ebp
        mov ebp, esp
        mov eax, [esp+4]
        leave
        ret

formatstring    db "%d",13,10,0
invoke ExitProcess,0

section '.idata' import data readable writeable

  library kernel32,'kernel32.dll',\
          crtdll,'crtdll.dll'

  import kernel32,\
         ExitProcess,'ExitProcess'

  import crtdll,\
         printf,'printf'
