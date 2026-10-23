read_numbers() {
	echo "Działanie A $1 B"
	echo "Podaj liczbę A"
	read A
	echo "Podaj liczbę B"
	read B
	if [[ "$1" != "/" ]]; then
		echo -n "Wynik działania: $A $1 $B = "
	fi
}

while [[ "$OPERATOR" != "q" ]]; do 
	echo "Podaj operator działania lub 'q' żeby zakończyć działanie skryptu"
	read OPERATOR

	if [[ "$OPERATOR" == "+" ]]; then
		read_numbers "+"
		echo $(($A+$B))
	elif [[ "$OPERATOR" == "-" ]]; then
		read_numbers "-"
		echo $(($A-$B))
	elif [[ "$OPERATOR" == "*" ]]; then
		read_numbers "*"
		echo $(($A*$B))
	elif [[ "$OPERATOR" == "/" ]]; then
		read_numbers "/"
		if [[ "$B" -eq 0 ]]; then
			echo "Dzielenie przez zero!"
		else
			echo -n "Wynik działania: $A / $B = "
			echo -n $(($A/$B))
			echo ", reszta $(($A%$B))"
		fi
	fi
	echo
done
