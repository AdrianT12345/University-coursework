#include<iostream>
using namespace std;

int szukaj(int tab[], int szukana, int index, int size) {
    if (index >= size) {
        return -1;
    }
    if (tab[index] == szukana) {
        return index;
    }

    return szukaj(tab, szukana, index + 1, size);
}

int main() {
    int size;
    cin >> size;
    int tab[100];
    int n;
    for (int i = 0; i < size; i++) {
        int x;
        cin >> x;
        tab[i] = x;
    }
    cin >> n;

    int wynik = szukaj(tab, n, 0, size);

    cout << wynik;

    return 0;
}

 