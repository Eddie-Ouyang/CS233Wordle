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


void simulate_all (const vector<string>& words, const vector<pair<string, vector<string>>>& second_guesses = {}) {
    vector<pair<string, vector<int>>> results;

    // for (const string& curr : words) {
    string curr = "slate";
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
            build_state(&feedback[0], &state);
            int index = feedback[0].feedback[0] + 3 * feedback[0].feedback[1] + 9 * feedback[0].feedback[2] + 27 * feedback[0].feedback[3] + 81 * feedback[0].feedback[4];
            compute_feedback(other, second_guesses[index].first, &feedback[1]);
            if (is_solved(feedback[1])) {
                attempts += 1;
                continue;
            }

            int guess = 2;
            for (; guess < 6; guess++) {
                build_state(&feedback[guess - 1], &state);
                string next_guess = find_matching_word(&state, second_guesses[index].second, partitions);
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
        cout << attempts << " " <<  fail << " " << match_loop << " " << req_loop << " " << allow_loop << endl;
    // }

    sort(results.begin(), results.end(), [](auto a, auto b) {
        if (a.second[1] == b.second[1]) {
            return a.second[0] < b.second[0];
        }
        return a.second[1] < b.second[1];
    });


    
    // ofstream writer("word_stats.csv");
    // writer << "word, attempts, failed, match loop, required loop, allowed loop" << endl;
    // for (auto i : results) {
    //     writer << i.first << ", ";
    //     for (int j = 0; j < 5; j++) {
    //         if (j > 0) writer << ", ";
    //         writer << i.second[j];
    //     }
    //     writer << endl;
    // }
    // writer.close();
}

pair<string, vector<string>> find_suitable_word (const wordle_state_t& state, const vector<string>& words, string curr) {
    int best = 0;
    string best_word = "N/A";
    int local_counts[5][26] = {};
    vector<string> local_words;

    for (string word : words) {
        if (word == curr) continue;
        if (letters_allowed(&state, word) && letters_required(&state, word)) {
            local_counts[0][word[0] - 'a']++;
            local_counts[1][word[1] - 'a']++;
            local_counts[2][word[2] - 'a']++;
            local_counts[3][word[3] - 'a']++;
            local_counts[4][word[4] - 'a']++;
            local_words.push_back(word);
        }
    }

    
    for (string word : local_words) {
        int score = local_counts[0][word[0] - 'a'] +
                    local_counts[1][word[1] - 'a'] +
                    local_counts[2][word[2] - 'a'] +
                    local_counts[3][word[3] - 'a'] +
                    local_counts[4][word[4] - 'a'];
        if (score > best) {
            best_word = word;
            best = score;
        }
    }
    return {best_word, local_words};
}

int count_remaining (const wordle_state_t& state, const vector<string>& words, string curr) {
    int count = 0;
    for (string word : words) {
        if (word == curr) continue;
        if (letters_allowed(&state, word) && letters_required(&state, word)) {
            count++;
        }
    }
    return count;
}

vector<pair<string, vector<string>>> generate_responses(wordle_feedback_t* prev, string word, const vector<string>& words) {
    wordle_feedback_t feedback;
    wordle_state_t state;
    feedback.word[0] = word[0];
    feedback.word[1] = word[1];
    feedback.word[2] = word[2];
    feedback.word[3] = word[3];
    feedback.word[4] = word[4];
    feedback.prev = prev;
    vector<pair<string, vector<string>>> results;
    
    for (int i = 0; i < 243; i++) {
        feedback.feedback[0] = i % 3;
        feedback.feedback[1] = (i / 3) % 3;
        feedback.feedback[2] = (i / 9) % 3;
        feedback.feedback[3] = (i / 27) % 3;
        feedback.feedback[4] = (i / 81) % 3;
        build_state(&feedback, &state);
        auto best = find_suitable_word(state, words, word);
        results.push_back(best);
    }

    return results;
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
    for (int i = 0; i < 26; i++ ) {
        sort(alpha[i].begin(), alpha[i].end(), [](auto a, auto b) {
            int scoreA = counts[0][a[0] - 'a'] + counts[1][a[1] - 'a'] + counts[2][a[2] - 'a'] + counts[3][a[3] - 'a'] + counts[4][a[4] - 'a'];
            int scoreB = counts[0][b[0] - 'a'] + counts[1][b[1] - 'a'] + counts[2][b[2] - 'a'] + counts[3][b[3] - 'a'] + counts[4][b[4] - 'a'];
            return scoreA > scoreB;
        });
    }

    vector<pair<char, int>> letter_counts;
    for (int i = 0; i < 26; i++) {
        letter_counts.push_back({(char)('a' + i), counts[0][i] + counts[1][i] + counts[2][i] + counts[3][i] + counts[4][i]});
    }
    sort(letter_counts.begin(), letter_counts.end(), [alpha](auto a, auto b) {
        return alpha[a.first - 'a'].size() > alpha[b.first - 'a'].size();
        // int scoreA = counts[0][a.first - 'a'] + counts[1][a.first - 'a'] + counts[2][a.first - 'a'] + counts[3][a.first - 'a'] + counts[4][a.first - 'a'];
        // int scoreB = counts[0][b.first - 'a'] + counts[1][b.first - 'a'] + counts[2][b.first - 'a'] + counts[3][b.first - 'a'] + counts[4][b.first - 'a'];
        // return scoreA > scoreB;
    });

    row = 0;
    partition = 0;
    vector<string> words_sorted;
    for (auto i : letter_counts) {
        for (auto w : alpha[i.first - 'a']) {
            words_sorted.push_back(w);
            // cout << w << " ";
            row++;
        }
        partitions[partition] = row;
        partition++;
    }

    auto responses = generate_responses(NULL, "slate", words);
    // map<string, int> checking;
    for (int i = 0; i < (int)responses.size(); i++) {
        if (responses[i].first == "N/A") continue;
        cout << (i/89) % 3 << " | " << (i/27) % 3 << " | " << (i/9) % 3 << " | " << (i/3) % 3 << " | " << (i) % 3 << " | ";
        cout << responses[i].first << " " << responses[i].second.size() << endl;
        // for (auto j : responses[i].second) {
        //     checking[j] = 1;
        // }
    }
    // cout << checking.size();
    // simulate_all(words_sorted, responses);
}