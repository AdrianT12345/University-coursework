; program przyk³adowy (wersja 32-bitowa) 
.686 
.model flat 
extern   _ExitProcess@4  : PROC 
extern   __write         : PROC   ; (dwa znaki podkreœlenia) 
public  _main  

.data 
tekst	db 10, 'Nazywam sie Adrian ' , 10  
		db 'Moj pierwszy 32-bitowy program '   
		db 'asemblerowy dziala juz poprawnie!', 10  

.code 
_main PROC
;wersja gitowa
; obliczenie sumy wyrazow ciagu 3 + 5 + 7 + 9 + 11

; obliczenie bez uzycia petli rozkazowej
	mov	eax, 3	; pierwszy element ciagu
	add	eax, 5	; dodanie drugiego elementu
	add	eax, 7	; dodanie trzeciego elementu
	add	eax, 9	; dodanie czwartego elementu
	add	eax, 11	; dodanie piatego elementu

; obliczenie z uzyciem petli rozkazowej
	mov	eax, 0	; poczatkowa wartosc sumy
	mov	ebx, 3	; pierwszy element ciagu
	mov	ecx, 5	; liczba obiegow petli
ptl:	add	eax, ebx	; dodanie kolejnego elementu
	add	ebx, 2	; obliczenie nastepnego elementu
	sub	ecx, 1	; zmniejszenie licznika obiegow petli
	jnz	ptl	; skok, gdy licznik obiegow rozny od 0

	mov	ecx, 88; liczba znakow wyswietlanego tekstu
; wywo³anie funkcji ”write” z biblioteki jêzyka C  
	
	push  ecx      ; liczba znaków wyœwietlanego tekstu  
	push  dword PTR OFFSET tekst ; po³o¿enie obszaru         ; ze znakami  

	push  dword PTR 1   ; uchwyt urz¹dzenia wyjœciowego  
	call  __write  ; wyœwietlenie znaków        ; (dwa znaki podkreœlenia _ )  
	
	add  esp, 12        ; usuniêcie parametrów ze stosu  
	
; zakoñczenie wykonywania programu  

	push  dword PTR 0     ; kod powrotu programu   
	call  _ExitProcess@4   

_main ENDP 
END 