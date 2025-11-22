#include "wordle_funcs.hpp"

using namespace std;



bool letters_allowed(const struct wordle_state_t *state, string candidate) {
  for (int i = 0; i < 5; i ++) {
    char letter_index = candidate[i] - 'a';
    allow_loop++;
    if ((state->letter_flags[i] & (1 << letter_index)) == 0) {
      return false;
    }
  }
  return true;
}

bool letters_required(const struct wordle_state_t *state, string candidate) {
  for (int i = 0; i < 5 ; i ++) {
    char desired_count = state->letters[i].count;
    if (desired_count == 0) {
      break;
    }
    req_loop++;
    // count up instances of each letter
    char letter_index = state->letters[i].letter_index;
    char count = 0;
    for (int j = 0; j < 5 ; j ++) {
        if (letter_index == (candidate[j] - 'a')) {
	        count ++;
        }
    }
    if (desired_count & EXACTLY) {
        desired_count &= ~EXACTLY;
        if (desired_count != count) {
	        return false;
        }
    } else {
        if (count < desired_count) {
	        return false;
        }
    }
  }
  
  return true;
}

string find_matching_word(const struct wordle_state_t *state, const vector<string>& words, int partitions[24]) {
    int nextPartition = 0;
    for (int i = 0; i < (int)words.size();) {
        string candidate = words[i];
        
        // char letter_index = candidate[0] - 'a';
        // if ((state->letter_flags[0] & (1 << letter_index)) == 0) {
        //     i = partitions[nextPartition];
        //     nextPartition++;
        //     continue;
        // }

        if (letters_allowed(state, candidate) && letters_required(state, candidate)) {
          return candidate;
        }

        i++;
        match_loop++;
        // if (words[i][0] != words[i-1][0]) {
        //     nextPartition++;
        // }
    }
    return "";
}

void init_state(wordle_state_t *state) {
    for (int i = 0; i < 5; i ++) {
        state->letter_flags[i] = 0x03ffffff;
        state->letters[i].count = 0;
        state->letters[i].letter_index = 0;
    }
}

char count_instances_of_letter(wordle_feedback_t *feedback, int i, char letter_index, unsigned *already_visited) {
  char count = 0;
  
  for (int j = i; j < 5 ; j ++) {
    char j_letter_index = feedback->word[j] - 'a';
    if (j_letter_index == letter_index) {
        char j_fback = feedback->feedback[j];
        if (j_fback == NOMATCH) {
	        count |= EXACTLY;
        } else {
	        count ++;
        }
        *already_visited |= (1 << j);
    }      
  }
  return count;
}

void build_state(struct wordle_feedback_t *feedback, struct wordle_state_t *state) {
    if (feedback == NULL) {
        init_state(state);
        return;
    }

    build_state(feedback->prev, state);

    unsigned int already_visited = 0;
    unsigned int next_letter = 0;

    for (int i = 0 ; i < 5 ; i ++) {
        char letter_index = feedback->word[i] - 'a';
        char fback = feedback->feedback[i];

        if (fback == MATCH) {
            state->letter_flags[i] = 1 << letter_index;
        } else {
            state->letter_flags[i] &= ~(1 << letter_index);
        }

        if (!(already_visited & (1 << i))) {
            char count = count_instances_of_letter(feedback, i, letter_index, &already_visited);
            if ((count & ~EXACTLY) == 0) {
                for (int k = 0 ; k < 5 ; k ++) {
                    state->letter_flags[k] &= ~(1 << letter_index);
                }
            } else {
                state->letters[next_letter].letter_index = letter_index;
                state->letters[next_letter].count = count;
                next_letter ++;
            }      
        }    
    }
}

void compute_feedback(string word, string input, struct wordle_feedback_t *feedback) {
  for (int i = 0; i < 5; i++) {
    feedback->word[i] = input[i];
    feedback->feedback[i] = 0;
  }

  // do a pass finding all positional matches
  for (int i = 0 ; i < 5 ; i ++) {
    if (input[i] == word[i]) {
      feedback->feedback[i] = MATCH;
    }
  }

  // do a second pass finding any wrong location matches
  for (int i = 0 ; i < 5 ; i ++) {
    if (feedback->feedback[i] == MATCH) {
      continue;
    }
    
    // count how many of these are in the actual word that weren't matched
    char letter = input[i];
    int wanted_count = 0;
    for (int j = 0 ; j < 5 ; j ++) {
      if (letter == word[j] && word[j] != input[j]) {
	      wanted_count ++;
      }
    }

    // count how many of these are up to this point
    int count = 1;
    for (int j = 0 ; j < i ; j ++) {
      if (letter == input[j]) {
	      count ++;
      }
    }
    
    if (count <= wanted_count) {
      feedback->feedback[i] = WRONG_LOCATION;
    }
  }
}

void reset_counts() {
    match_loop = 0;
    req_loop = 0;
    allow_loop = 0;
}

bool is_solved(wordle_feedback_t feedback) {
  for (int i = 0; i < 5; i++) {
    if (feedback.feedback[i] != MATCH) return false;
  }
  return true;
}
