#include<iostream>
using namespace std;

int main() {
        int n;
        cin >> n;
        int tab[100];
        for (int i = 0; i < n; i++) {
                cin >> tab[i];
        }

        for (int i = 0; i < n; i++) {
                for (int j = 0; j < i; j++) {
                        if (tab[i] == tab[j]) {
                                cout << tab[i] << " " << endl;;
                                break;
                        }
                }
        }

}