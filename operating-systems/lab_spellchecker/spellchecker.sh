# Description PL      : Skrypt w Bashu sprawdzający pisownię tekstu na podstawie słownika (words.txt). Nieznane słowa są wyróżniane w wyniku kolorem 
# Description EN     : A Bash script that checks the spelling of a text against a dictionary (words.txt). Unknown words are highlighted in red in the output.
czerwonym.
#!/bin/bash

DICT="./words.txt"

RED_BOLD='\033[1;31m'
RESET='\033[0m'

echo "Podaj tekst do sprawdzenia: "

FULL_OUTPUT=""

while read -r line; do
	output_line=""

	for word in $line; do
		clean_word="${word//[^a-zA-Z"'"]/}"

		if [[ -z "$clean_word" ]]; then
			output_line+="$word "
		elif grep -q -i "^${clean_word}$" "$DICT"; then
			output_line+="$word "
		else
			output_line+="${RED_BOLD}${word}${RESET} "
		fi
	done

	FULL_OUTPUT+="$output_line\n"
done

echo -e "\nWynik sprawdzania:"
echo -e "$FULL_OUTPUT"

