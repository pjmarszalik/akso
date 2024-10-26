; kompiluj tak:
; nasm -f elf64 -w+all -w+error -o scopy.o scopy.asm && ld --fatal-warnings -o scopy scopy.o
; uruchom tak: 
;./scopy plik_wejsciowy plik_wyjsciowy

section .data
SYS_EXIT equ 60
SYS_READ equ 0
SYS_WRITE equ 1
SYS_OPEN equ 2
SYS_CLOSE equ 3

O_RDONLY equ 0o
O_WRONLY equ 1o
O_CREATE equ 100o
O_EXCL equ 200o
WRITE_FLAGS equ O_CREATE | O_EXCL | O_WRONLY
ERROR_CODE equ 1
READ_ONLY_MODE equ 0444
WRITE_MODE equ 0644o

READ_BUFFER_SIZE equ 32768
WRITE_BUFFER_SIZE equ 65536


section .bss
read_buffer resw READ_BUFFER_SIZE
write_buffer resw WRITE_BUFFER_SIZE

fd_in resd 1
fd_out resd 1

section .text
global  _start
_start:
    mov dl, [rsp]                              ; sprawdzenie liczby parametrów
    sub dl, 3
    jnz _error_exit                             ; jak zła liczba to wychodzę z kodem błędu odpowiednim
                                                ; otwieram pierwszy plik
    mov rdi, QWORD [rsp + 16]                   
    mov rax, SYS_OPEN
    xor rsi, rsi
    mov rdx, READ_ONLY_MODE
    syscall
    test rax, rax                               ; sprawdzam czy się udało
    js _error_exit;                             ; jeśli nie to wychodzę z odpowiednim kodem
    mov [fd_in], eax
                                                ; otwieram drugi plik
    mov rdi, QWORD [rsp + 24]                   
    mov rax, SYS_OPEN
    mov rsi, WRITE_FLAGS
    mov rdx, WRITE_MODE
    syscall
    test rax, rax                               ; sprawdzam czy się udało
    js _close_exit;                             ; jeśli nie to zamykam plik (pierwszy) i wychodzę
    mov [fd_out], eax

    xor r10, r10                                ; wyzerowanie licznika bajtów nierównych s lub S
_read_write_loop:                               ; pętla czytaj-pisz
                                                ; wczytaj plik do bufora - read_buffer
    xor rax, rax                                ; ponieważ SYS_READ = 0
    mov edi, [fd_in]
    mov rsi, read_buffer
    mov rdx, READ_BUFFER_SIZE
    syscall
    test rax, rax                               ; patrzę czy udało się odczytać z pliku
    js _double_close_exit                       ; jak nie to zamykam oba pliki i wychodzę
    jz _exit                                    ; jak dotarliśmy do końca pliku to wychodzimy
                                                ; teraz będzie pętla przepisjąca z bufora read do bufora write
                                                ; r8 to iterator który idzie od 0 do (BUFFER_SIZE - 1)
                                                ; w każdym przejściu pętli przepisuje dany bajt poprzez raxa
    xor r8, r8                                  ; iterator read
    xor edi, edi                                ; iterator write
    mov r9, rax                                 ; ile bajtów przepisane
    mov dl, 83                                  ; s w ASCII
    mov sil, 115                                ; S w ASCII
_copy_loop:
    cmp r8, r9
    je _end_copy_loop

    mov cl, [read_buffer + r8*1]

    cmp dl, cl                                  ; sprawdzam czy wczytałem s
    je _write
    cmp sil, cl                                 ; sprawdzam czy wczytałem S
    je _write
                                                ; jak to nie jest litera s/S to dodaję 1 do odpowiednich liczników i lecę na górę pętli
    inc r10                                     ; inkrementacja licznika bajtów nierównych s/s
    inc r8                                      ; iterator
    jmp _copy_loop

_write:
    test r10, r10
    jnz _write_number                           ; jeśli licznik bajtów nierównych s/s niezerowy to idę wypisać liczbę
_done_come_back:
    mov [write_buffer + rdi*1], cl
    inc edi                                     ; iterator write
    inc r8                                      ; iterator read
    jmp _copy_loop
_end_copy_loop:
                                                ; przepisz do drugiego pliku
    mov rdx, rdi                                ; count = rdi, bo rdi to iterator w write_buffer - czyli tyle chcę przepisać     
    mov rax, SYS_WRITE
    mov edi, [fd_out]
    mov rsi, write_buffer
    syscall
    test rax, rax
    js _double_close_exit

    jmp _read_write_loop

_exit:  
    test r10, r10
    jz _nothing_to_write       
    mov [write_buffer], WORD r10w
    mov rdx, 2
    mov rax, SYS_WRITE
    mov edi, [fd_out]
    mov rsi, write_buffer
    syscall
    test rax, rax
    js _double_close_exit

_nothing_to_write:           
    mov rax, SYS_CLOSE                          ; zamykam plik wyjściowy
    mov edi, [fd_out]
    syscall
    test rax, rax
    js _close_exit
    mov rax, SYS_CLOSE                          ; zamykam plik wejściowy
    mov edi, [fd_in]
    syscall
    test rax, rax
    js _error_exit
    mov rax, SYS_EXIT
    xor edi, edi
    syscall

_write_number:                                  ; wpisuję w buffor dwubajtowe słowo z ilością bajtów nierównych s/s
    mov [write_buffer + rdi*1], WORD r10w
    xor r10, r10
    add edi, 2
    jmp _done_come_back

_double_close_exit:
    mov rax, SYS_CLOSE                          ; zamykam plik wyjściowy
    mov edi, [fd_out]
    syscall
_close_exit:
    mov rax, SYS_CLOSE                          ; zamykam plik wejściowy
    mov edi, [fd_in]
    syscall
_error_exit:                                    ; wyjdź syscallem sys_exit
    mov rax, SYS_EXIT
    mov edi, 1
    syscall
