global sum
                                                ; rdi = * x
                                                ; rsi = n
                                                ; w komentarzach będę przyjmował, że liczby są zapisane w little-endian (mimo że to sprawi, że np shr będzie przesunięciem w LEWO itd.)
sum:
    xor r8, r8                                  ; r8 = i - iterator pętli która idzie od 1 do n - 1
    xor r9, r9                                  ; r9 = poziomU2 - do kiedy mam przekonwertowane - czyli x[0,...,poziomU2] to liczba w systemie U2 na 64*(poziomU2 + 1) bitach
    xor r11, r11                                ; r11 to moje carry - czyli liczba bitów które "wyskoczyły" ponad moje U2
loop:
    add r8, 1                                   ; zwiększam iterator pętli
    cmp rsi, r8 
    jle end                                     ; sprawdzam czy to już koniec

    mov r10, [rdi + r8*8]                       ; r10 = x[i] -  biorę komórkę x-a do rejestru
    cqo                                         ; zeruję rdxa bo będę dzielił

    lea rax, [r8*8]                             ; przenoszę do raxa mój iterator i z r8 pomnożony od razu przez 8
    mul rax                                     ; podnoszę rax do kwadratu (mul mnoży domyślnie raxa) otrzymując teraz 64 * i^2
    div rsi                                     ; dzielę rdx:rax = 64 * i * i przez rsi = n otrzymując oczekiwane przesunięcie w bitach
    cqo                                         ; zeruję rdxa
    mov cl, 64
    div rcx                                     ; dzielę przesunięcie w bitach przez 64 = rcx = ecx = cl
    mov ecx, edx                                ; w rdx zostaje modulo które wrzucam do rcx (shift), a w rax mam przesunięcie w komórkach 64-bitowych
                                                
where_U2:                                       ; stan obecny: r8 = i, r9 = poziomU2, r10 = x[i], r11 = carry, rax - gdzie_wsadzić, rcx - shift, wolne: rdx
    cmp r9, rax                                 ; porównuję gdzie_wsadzić i gdzieU2 i gdy (r9 <= rax) to skaczę aby zwiększyć r9 o jeden i wracam wyżej aby znów porównać
    jle bigger_U2                               ; kończę dopiero gdy r9 = rax + 1 - moje U2 jest o komórkę dalej niż miejsce w które dodaję raxa 
                                                
    mov rdx, r10                                ; chcę teraz, żeby w r10:rdx było słowo 128 bitowe w U2 które jest słowem r10 przesuniętym o cl
    shl r10, cl                                 ; robię z r10 lewą (mniej znaczącą) połowę tego 128 bitowego słowa

    test rcx, rcx                               ; sprawdzam czy cl nie jest zerem - wtedy (64 - cl) = 64, a sar nie pozwala na przesunięcia o 64 bity
    jz increment_cl                             ; jeśli tak jest to zwiększam cl o jeden - wtedy będę shiftował o 63 a to też wypełni rejestr bitem znaku
back1:  
    neg cl                                      
    add cl, 64                                  ; dwie ostanie instrukcje skutkują w cl = (64 - cl)
    sar rdx, cl                                 ; przesuwam r10 odpowiednio o (64 - shift) bitów, teraz moje rdx:r10 jest gotowe

    add [rdi + rax*8], r10                      ; dodaję do odpowiedniej komórki pierwszą (lewą) połowę słowa
    adc [rdi + rax*8 + 0x8], rdx                ; dodaję do odpowiedniej komórki drugą połowę (prawą)
    jo overflow                                 ; patrzę czy nie doszło do overflowa i jeśli doszło to w overflow odpowiednio modyfikuję carry          
    jmp loop   

end:
    ret

increment_cl:
    mov cl, 1
    jmp back1

bigger_U2:
    mov rdx, [rdi + r9*8]                       ; biorę to co jest w ostatniej komórce mojego przekonwertowanego U2
    sar rdx, 63                                 ; przesuwam odpowiednio rdx, żeby dostać wydłużony bit znaku
    mov [rdi + r9*8 + 0x8], rdx                 ; do następnej komórki pamięci wkładam rdx, żeby otrzymać faktycznie wydłużone U2 do r9 + 1

    mov rdx, r11                                ; teraz zajmę się moim carry - jest to liczba, która pierwszy bit ma na poziomie ostatniego bitu [rdi + r9*8]
    shl rdx, 63                                 ; dlatego podzielę carry na dwa rejestry rdx:r10 i dodam podobnie jak w loop | robię z rdx młodszą połówkę carry
    sar r11, 1                                  ; i strszą połówką będzie r11 | wydłużam znak r11 bo carry może być ujemne
    add [rdi + r9*8], rdx                       ; dodaję młodszą
    adc [rdi + r9*8 + 0x8], r11                 ; dodaję starszą
    xor r11, r11                                ; zeruję carry, bo już je dodałem
    add r9, 1                                   ; zwiększam mój zasięg U2
    jmp where_U2                                ; wracam tam, gdzie znowu porównuję r9 z raxem

overflow:
    sar rdx, 63                                 ; biorę najstarszy bit rdx - czyli znak tego co dodałem gdy doszło do overflowa
    jz plus                                     ; jeśli overflow był dodatni (czyli rdx = 0, bo rdx jest teraz swoim bitem znaku) to chcę dodać jeden do carry 
back2:                                          ; a oczywiście gdy overflow był ujemny to chcę odjąć jeden od carry - wtedy rdx = -1 i tak zostaje
    add r11, rdx                                ; modyfikuję carry w odpowiedni sposób
    shl rdx, 63                                 ; rdx = 00...01  -  tworzę taką liczbę (wiem , że rdx = +=1, więc takie przesunięcie istotnie tak zrobi)        
    xor [rdi + rax*8 + 0x8], rdx                ; zmieniam xorem najstarszy bit (czyli znaku) bo zabrałem go do carry
    jmp loop

plus:
    mov dl, 1
    jmp back2