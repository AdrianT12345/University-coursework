#include <iostream>
using namespace std;

int main() {
    int n;
    cin >> n;
    int size;
    size = n * 4;
    int x;
    int y;
    cin >> x;
    cin >> y;
    //gora rakiety
    for (int o = 0; o < y; o++) {
        for (int j = 0; j < 2 * n - n - 1 / 2; j++) {
            for (int t = 0; t < x; t++) {
                for (int i = 0; i < size / 2 - 1 - j; i++)
                    cout << "*";
                cout << "/";
                for (int i = 0; i < 2 * j; i++)
                    cout << "*";
                cout << "\\";
                for (int i = 0; i < size / 2 - 1 - j; i++)
                    cout << "*";
            }
            cout << endl;

        }
        //sufit kwadrat
        for (int t = 0; t < x; t++) {
            for (int j = 0; j < n; j++) {
                cout << "*";
            }
            for (int j = 0; j < (2 * n); j++) {
                cout << "#";
            }
            for (int j = 0; j < n; j++) {
                cout << "*";
            }
        }
        cout << endl;

        // kwadrat
        for (int k = 0; k < n * 2 - 2; k++) {
            for (int t = 0; t < x; t++) {
                for (int l = 0; l < n; l++) {
                    cout << "*";
                }
                cout << "|";
                for (int l = 0; l < n * 4 - n * 2 - 2; l++) {
                    cout << "*";
                }
                cout << "|";
                for (int l = 0; l < n; l++) {
                    cout << "*";
                }
            }
            cout << endl;
        }
        //podloga kwadrat
        for (int t = 0; t < x; t++) {
            for (int j = 0; j < n; j++) {
                cout << "*";
            }
            for (int j = 0; j < (2 * n); j++) {
                cout << "#";
            }
            for (int j = 0; j < n; j++) {
                cout << "*";
            }
        }
        cout << endl;
        // dol rakiety
        for (int j = 0; j < 2 * n - n - 1 / 2; j++) {
            for (int t = 0; t < x; t++) {
                for (int i = 0; i < size / 2 - 1 - j - n; i++)
                    cout << "*";
                cout << "/";
                for (int i = 0; i < j * 2 + n * 2; i++)
                    cout << "*";
                cout << "\\";
                for (int i = 0; i < size / 2 - 1 - j - n; i++)
                    cout << "*";
            }
            cout << endl;
        }
    }
    return 0;
}
