#pragma once

#include <string>
#include <vector>

const char NOMATCH = 0;
const char WRONG_LOCATION = 1;
const char MATCH = 2; 
const char EXACTLY = 0x40;
static int match_loop;
static int req_loop;
static int allow_loop;

using namespace std;

struct wordle_feedback_t {
  char word[5];
  char feedback[5];
  struct wordle_feedback_t *prev = NULL;
};

struct letter_info_t {
  char letter_index = 0;
  char count = 0;
};

struct wordle_state_t {
  unsigned int letter_flags[5] = {};
  struct letter_info_t letters[5] = {};
};


bool letters_allowed(const struct wordle_state_t *state, string candidate);
bool letters_required(const struct wordle_state_t *state, string candidate);
string find_matching_word(const struct wordle_state_t *state, const vector<string>& words, int partitions[24]);
void init_state(wordle_state_t *state);
char count_instances_of_letter(wordle_feedback_t *feedback, int i, char letter_index, unsigned *already_visited);
void build_state(struct wordle_feedback_t *feedback, struct wordle_state_t *state);
void compute_feedback(string word, string input, struct wordle_feedback_t *feedback);
void reset_counts();
bool is_solved(wordle_feedback_t feedback);