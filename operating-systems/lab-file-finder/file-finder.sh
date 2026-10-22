# Author           : s208249@student.pg.edu.pl
# Created On       : 27.03.2026
# Last Modified On : 30.03.2026
# Version          : version
#
# Description PL      : Celem zadania jest napisanie skrypt w języku powłoki Bash, który za pomocą interaktywnego interfejsu tekstowego pozwoli na wyszukiwanie plików w systemie Linux.
# Description EN      : The goal of this task is to write a script in the Bash shell language that, using an interactive text interface, allows searching for files in a Linux system.
# Licensed under GPL (see /usr/share/common-licenses/GPL for more details
# or contact # the Free Software Foundation for a copy)
#
# Generative AI statement (keep ONE line below, delete the others):
# * I used GenAI tools for support only (e.g., explanations, debugging, small snippets).

#!/bin/bash -f

NAZWA_PLIKU=""
KATALOG=""
ROZMIAR=""
CZAS=""
ZAWARTOSC_PLIKU=""

while true; do
	clear
	echo "1. Nazwa pliku: $NAZWA_PLIKU"
	echo "2. Katalog: $KATALOG"
	echo "3. Rozmiar: $ROZMIAR"
	echo "4. Czas: $CZAS"
	echo "5. Zawartosc pliku: $ZAWARTOSC_PLIKU"
	echo "6. Szukaj"
	echo "7. Koniec"
	echo "Wybierz opcje:"
	read OPERATOR

	case "$OPERATOR" in
		"1")
			echo "Podaj nazwe pliku: "
			read NAZWA_PLIKU
			;;
		"2")
			echo "Podaj sciezke do katalogu: "
			read KATALOG
			;;
		"3")
			echo "Podaj rozmiar: "
			read ROZMIAR
			;;
		"4")
			echo "Podaj czas w dniach: "
			read CZAS
			;;
		"5")
			echo "Podaj szukany tekst w pliku: "
			read ZAWARTOSC_PLIKU
			;;
		"6")
			echo "WYNIK WYSZUKIWANIA"
			SZUKAJ_W="${KATALOG:-.}"

			CMD="find $SZUKAJ_W"

			[[ -n "$ZAWARTOSC_PLIKU" ]] && CMD="$CMD -type f"
			[[ -n "$NAZWA_PLIKU" ]] && CMD="$CMD -name '$NAZWA_PLIKU'"
			[[ -n "$ROZMIAR" ]] && CMD="$CMD -size $ROZMIAR"
			[[ -n "$CZAS" ]] && CMD="$CMD -mtime $CZAS"

			if [[ -n "$ZAWARTOSC_PLIKU" ]];then
			CMD="$CMD | xargs grep -l '$ZAWARTOSC_PLIKU'"
			fi

			eval $CMD

			echo ""
			echo "Naciśnij Enter, aby kontynuować..."
			read
			;;
		"7")
			exit 0
			;;
		*)
			echo "Nie poprawna opcja"
			sleep 1
			;;
	esac
done
