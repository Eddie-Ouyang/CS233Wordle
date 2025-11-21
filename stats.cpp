#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include "include\wordle_funcs.cpp"

using namespace std;

static int counts[5][26] = {};
static int partitions[24];

void write_stats() {
    vector<pair<char, int>> slot_counts[6];
    auto sorting = [](pair<char,int> a, pair<char,int> b) { return a.second > b.second; };

    for (int i = 0; i < 26; i++) {
        slot_counts[0].push_back({(char)('a' + i), counts[0][i] + counts[1][i] + counts[2][i] + counts[3][i] + counts[4][i]});
        for (int j = 1; j < 6; j++) {
            slot_counts[j].push_back({(char)('a' + i), counts[j-1][i]});
        }
    }

    for (int i = 0; i < 6; i++) {
        sort(slot_counts[i].begin(), slot_counts[i].end(), sorting);
    }

    ofstream writer("letter_counts.csv");
    writer << "Total, count, 1, count, 2, count, 3, count, 4, count, 5, count" << endl;

    for (int i = 0; i < 26; i++) {
        for (int j = 0; j < 6; j++) {
            if (j > 0) writer << ", ";
            writer << slot_counts[j][i].first << ", " << slot_counts[j][i].second;
        }
        writer << endl;
    }

    writer.close();
}


void simulate_all (const vector<string>& words) {
    vector<pair<string, vector<int>>> results;

    for (const string& curr : words) {
        reset_counts();
        int fail = 0;
        int attempts = 0;
        for (const string& other : words) {
            if (other == curr) continue;
            wordle_feedback_t feedback[6];
            wordle_state_t state;
            for (int i = 1; i < 6; i++) {
                feedback[i].prev = &feedback[i-1];
            }
            
            compute_feedback(other, curr, &feedback[0]);
            int guess = 1;
            for (; guess < 6; guess++) {
                build_state(&feedback[guess - 1], &state);
                string next_guess = find_matching_word(&state, words, partitions);
                if (next_guess.length() == 0) {
                    fail++;
                    break;
                }
                compute_feedback(other, next_guess, &feedback[guess]);
                if (is_solved(feedback[guess])) {
                    attempts += guess;
                    break;
                }
            }
            if (guess == 6) {
                fail++;
                attempts += 6;
            }
        }

        results.push_back({curr, {attempts, fail, match_loop, req_loop, allow_loop}});
    }

    sort(results.begin(), results.end(), [](auto a, auto b) {
        if (a.second[1] == b.second[1]) {
            return a.second[0] < b.second[0];
        }
        return a.second[1] < b.second[1];
    });

    
    ofstream writer("word_stats.csv");
    writer << "word, attempts, failed, match loop, required loop, allowed loop" << endl;
    for (auto i : results) {
        writer << i.first << ", ";
        for (int j = 0; j < 5; j++) {
            if (j > 0) writer << ", ";
            writer << i.second[j];
        }
        writer << endl;
    }
    writer.close();
}

int main() {
    ifstream reader("message.txt");
    string current;
    vector<string> words;

    char last = '-';
    int row = 0;
    int partition = 0;
    while (getline(reader, current)) {
        for (int i = 0; i < 5; i++) {
            int idx = current[i] - 'a';
            counts[i][idx]++;
        }
        words.push_back(current);
        if (current[0] != last) {
            last = current[0];
            partitions[partition] = row;
            partition++;
        }
        row++;
    }
    reader.close();

    vector<string> alpha[26];
    for (auto i : words) {
        alpha[i[0] - 'a'].push_back(i);
    }

    vector<pair<char, int>> letter_counts;
    for (int i = 0; i < 26; i++) {
        letter_counts.push_back({(char)('a' + i), counts[0][i] + counts[1][i] + counts[2][i] + counts[3][i] + counts[4][i]});
    }
    sort(letter_counts.begin(), letter_counts.end(), [](auto a, auto b) {
        return a.second > b.second;
    });

    row = 0;
    partition = 0;
    vector<string> words_sorted;
    for (auto i : letter_counts) {
        for (auto w : alpha[i.first - 'a']) {
            words_sorted.push_back(w);
            row++;
        }
        partitions[partition] = row;
        partition++;
    }

    simulate_all(words_sorted);
}