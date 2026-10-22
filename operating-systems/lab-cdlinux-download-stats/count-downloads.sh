# Author           : Adrian Trojak
# Created On       : 16.03
# Last Modified On : 23.03
# Version          : version
#
# Description      :
#PL
#Na podstawie danych zapisanych w plikach logów z serwerów ftp (cdlinux.ftp.log) i http (cdlinux.www.log), podać liczbę pobrań dla każdej z wersji dystrybucji #cdlinux.pl. Użytkowników pobierających daną wersję dystrybucji wielokrotnie spod jednego adresu IP należy liczyć tylko raz.
#EN
# Based on the data stored in the log files of the ftp (cdlinux.ftp.log) and http (cdlinux.www.log) servers, give the number of downloads for each version of #the cdlinux.pl distribution. Users who download a given version of the distribution multiple times from a single IP address should be counted only once.

# Licensed under GPL (see /usr/share/common-licenses/GPL for more details
# or contact # the Free Software Foundation for a copy)
#
# Generative AI statement (keep ONE line below, delete the others):
# * I used GenAI tools for support only (e.g., explanations, debugging, small snippets).

#!/bin/bash
echo "wynik z pliku www"
awk '{print $1, $7}' cdlinux.www.log | awk -F'/' '{print $1, $NF}' | grep -E ' cdlinux-[a-zA-Z0-9.-]+\.iso$' | sort -u | awk '{print $2}' | sort | uniq -c
echo "wynik sumaryczny"
(
  grep "OK DOWNLOAD" cdlinux.ftp.log | awk -F'"' '{print $2, $4}' | awk -F'/' '{print $1, $NF}' | grep -E ' cdlinux-[a-zA-Z0-9.-]+\.iso$' | sort -u | awk '{print $2}'

  awk '{print $1, $7}' cdlinux.www.log | awk -F'/' '{print $1, $NF}' | grep -E ' cdlinux-[a-zA-Z0-9.-]+\.iso$' | sort -u | awk '{print $2}'
) | sort | uniq -c
