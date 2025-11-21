#include <iostream>
#include <fstream>
#include <string>
#include <utility>

using namespace std;

int main() {
    ifstream reader("message.txt");
    int counts[5][26] = {};
    string current;
    char last = ' ';
    int row = 0;
    while (getline(reader, current)) {
        for (int i = 0; i < 5; i++) {
            int idx = current[i] - 'a';
            counts[i][idx]++;
        }
        // if (current[0] != last) {
        //     last = current[0];
        //     cout  << " " << row * 6;
        // }

        // row++;
    }
    reader.close();

    // for (int i = 0; i < 5 ; i++) {
    //     cout << i << endl;
    //     for (int j = 0; j < 26; j++) {
    //         cout << ((char)('a' + j)) << " " << counts[i][j] << " | " ;
    //     }
    //     cout << endl << endl;
    // }

    pair<string,int> bestSlotted[5] = {};
    pair<string,int> bestGeneral[5] = {};

    ifstream reader2("message.txt");
    while (getline(reader2, current)) {
        int slotScore = 0;
        int genScore = 0;
        for (int i = 0; i < 5; i++) {
            slotScore += counts[i][current[i] - 'a'];
            for (int j = 0; j < 5; j++) {
                genScore += counts[j][current[i] - 'a'];
            }
        }

        for (int i = 0; i < 5; i++) {
            if (slotScore > bestSlotted[i].second) {
                for (int j = 4; j > i; j--) {
                    bestSlotted[j] = bestSlotted[j - 1];
                }
                bestSlotted[i] = {current, slotScore};
                break;
            }
        }

        
        for (int i = 0; i < 5; i++) {
            if (genScore > bestGeneral[i].second) {
                for (int j = 4; j > i; j--) {
                    bestGeneral[j] = bestGeneral[j - 1];
                }
                bestGeneral[i] = {current, genScore};
                break;
            }
        }
    }
    reader2.close();

    cout << "General    |   Slot" << endl;
    for (int i = 0 ; i < 5; i++) {
        cout << bestGeneral[i].first << " " << bestGeneral[i].second << "   ";
        cout << bestSlotted[i].first << " " << bestSlotted[i].second << endl;
    }
    return 0;
}