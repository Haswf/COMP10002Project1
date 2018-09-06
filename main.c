#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define INITIAL_FRAGMENT_INDEX 0
#define INITIAL_CHARACTER_INDEX 0

// DEBUGGING PANEL
#define DEBUG 0
#define OVERLAPDEBUG 0

#define MAX_LEN 21
#define MAX_NUM 1000

// Position
#define HEAD 0
#define TAIL 1
#define BOTH 2


// Output-related constants
#define FIRST_25_CHARACTERS 25
#define THRESHOULD_LENGTH 54
#define FIRST_TEN_FRAGMENT 10
#define LAST_ELEMENT_INDICATOR (-1)


//initialize the superstring with the biggest possible size (i.e. MAX_NUM * MAX_LEN)
typedef struct _superstr{char thestring[MAX_NUM * MAX_LEN];
    char processed[MAX_NUM][MAX_LEN];
    char unprocessed[MAX_NUM][MAX_LEN];
    int processed_count;
    int unprocessed_count;
} superstr_t;

int read_str(char strarr[MAX_NUM][MAX_LEN]);
void initialize_superstr(superstr_t* superstr_p);
char* last_char(char str[MAX_LEN]);
char* strrstr(char *str, char *substr);
char* stroverlap(char* str, char* substr, char* overlap, char* remainder);
char* strtoupper(char* str, int index);
char* place_initial_fragment(char* superstr, char* substr);
void strerase(char *str);
char* max_overlap(superstr_t *superstr_pointer, int *index, int where);
void print_restricted_superstr(char* superstr, int superstrlen);
void print_output(int nth, int frg_no, char *superstr, int end);
void print_output_header(int stage_no);
void insert_to_superstr(char* superstr, char* substr, char* overlap, char* remainder, char* insubstr);
void append_to_superstr(char* superstr, char* substr, char* overlap, char* remainder, char* insubstr);
int stage1(superstr_t superstr);
int stage2(superstr_t superstr);

//DEBUGGING UTILITIES
void print_array(char array[MAX_NUM][MAX_LEN], int count);

int read_str(char strarr[MAX_NUM][MAX_LEN]){
    int frag_counter = 0;
    int char_counter = 0;
    int input_length = 0;
    char cur_str[MAX_LEN];
    while (scanf("%s", cur_str)==1 && frag_counter < MAX_NUM){
        if ((input_length = (int)strlen(cur_str)) < MAX_LEN){
            // frag_counter only incrases by one when the input has been successfully read.
            if (strcpy(strarr[frag_counter++], cur_str)){
                char_counter += input_length;
            }
        }
    }
    print_output_header(0);
    printf("%d fragments read, %d characters in total\n", frag_counter, char_counter);
    return frag_counter;
}

void initialize_superstr(superstr_t* superstr_p){
    /* Initialize any superstr_t structure passed by the pointer
     * Basically, it zero-filled everything.
    */
    // '\0'-fill array of strings processed.
    memset((superstr_p->processed), '\0', sizeof(superstr_p->processed));
    //'\0'-fill array of strings unprocessed.
    memset((superstr_p->unprocessed), '\0', sizeof(superstr_p->unprocessed));
    //'\0'-fill thestring.
    memset((superstr_p->thestring), '\0', sizeof(superstr_p->thestring));
    //0-fill all counters.
    superstr_p->processed_count = 0;
    superstr_p->unprocessed_count = 0;
}

char* last_char(char str[MAX_LEN]){
    /* return the last character in the string */
    return &str[strlen(str)-1];
}

char* strrstr(char *str, char *substr){
    /* Return the pointer point to the last occurrence of substr in str */

    char *p = str;
    char *last_p = str;
    for (int i = 0; ; i++) {
        if (!(p = strcasestr(p, substr))){
            break;
        }
        last_p = p;
        // search from the next position
        p = p+strlen(substr);

    }
    return last_p;
}

char* stroverlap(char* str, char* substr, char* overlap, char* remainder){
    /* Search for the overlap part of char* str and char* substr.
     * Copy the overlap part into char* overlap
     * Copy the non-overlap part of substr to char* remainder
     * return pointer where the largest overlap was found
     * return NULL if no overlap exists.
    */

    // Copy the entire substr to overlap
    strcpy(overlap, substr);
    char* position = NULL;

    // Check if the whole fragment is presented in the superstring
    if (strcasestr(str, substr)){
        position = strcasestr(str, substr);
    }

        // Otherwise, check if there is any overlap between the head and the tail.
    else{

        for(int i = (int)strlen(substr)-1; i >= 0; i--){
            // if partial fragment is presented in the superstring
            // and it is at the end of the superstring
            if ((position = strcasestr(str, overlap))){
                // Call strrstr to search for the last occurrence of overlap.
                position = strrstr(position, overlap);
                if (strlen(position)==strlen(overlap)){
                    break;
                }

            }
            // “Cross out” the last character in overlap.
            memset(last_char(overlap),'\0', sizeof(char));
            // remainder get the "cross-outed" character.
            strcpy(remainder, substr+i);
//            printf("%s \t%s\n", remainder, overlap);
        }
    }
    return position;
}

char* strtoupper(char* str, int index){
    /* Capitalise the index-th character in the string
     * return the modified string
    */
    char upper_character = (char)toupper(str[index]);
    memset(&str[index], upper_character, sizeof(char));
    return str;
}

char* place_initial_fragment(char* superstr, char* substr){
    // Capitalize the first character of the first fragment.
    strtoupper(substr, INITIAL_CHARACTER_INDEX);
    // Use the first fragment to initialize a superstring.
    strcpy(superstr, substr);
    print_output(INITIAL_FRAGMENT_INDEX, INITIAL_FRAGMENT_INDEX, superstr, 0);
    return superstr;
}


char* modify_superstr(char* superstr, char* substr, int position){
    //0-filled overlap and remainder
    char overlap[MAX_LEN] = {'\0'};
    char remainder[MAX_LEN] = {'\0'};
    if (position == TAIL){
        char* insubstr = stroverlap(superstr, substr, overlap, remainder);
        append_to_superstr(superstr, substr, overlap, remainder, insubstr);
    }
    else if (position == HEAD){
        char* insubstr = stroverlap(substr, superstr, overlap, remainder);
        insert_to_superstr(superstr, substr, overlap, remainder, insubstr);
    }
    return superstr;
}



void append_to_superstr(char* superstr, char* substr, char* overlap, char* remainder, char* insubstr){

    // the number of characters overlapping.
    int overlap_len = (int)strlen(overlap);

    // if the fragment appears wholly in the superstring.
    if (overlap_len==strlen(substr)){
        // capitalize the first character of the fragment in the superstring.
        strtoupper(insubstr, INITIAL_CHARACTER_INDEX);
    }

        // No overlap exists.
    else if (!overlap_len){
        // capitalize the first character of the fragment.
        strtoupper(substr, INITIAL_CHARACTER_INDEX);
        strcat(superstr, substr);
    }

        // otherwise, overlap exists
    else {
        // capitalize the first character of the overlap.
        strtoupper(insubstr, INITIAL_CHARACTER_INDEX);
        // Append the remainder to the superstring.
        strcat(superstr, remainder);
    }
}

void insert_to_superstr(char* superstr, char* substr, char* overlap, char* remainder, char* insubstr){

    // otherwise, overlap exists at the head of the superstring.
    // capitalize the first character of the overlap.
    strtoupper(overlap, INITIAL_CHARACTER_INDEX);
    // Insert the superstr to the end of the remainder.
    char merge[strlen(substr)+strlen(superstr)+1];
    memset(insubstr, '\0',sizeof(char));
    strcpy(merge, strtoupper(substr, INITIAL_CHARACTER_INDEX));
    strcat(merge, overlap);
    strcat(merge, remainder);
    memcpy(superstr, merge, (strlen(merge)+1)*sizeof(char));
}

void strerase(char *str){
    memset(str, '\0', sizeof(char));
}

char* max_overlap(superstr_t *superstr_pointer, int *index, int where){

    int const frag_count = superstr_pointer->unprocessed_count;
    // The max length of overlap found in unprocessed fragments.
    int max_overlap = 0;
    char* max_location = {'\0'};

    // Iterate through unprocessed fragment
    for (int i= 1; i < frag_count && superstr_pointer->unprocessed[i]; i++){
        int cur_overlap = 0;
        //Initialize overlap and remainder for each fragment to be tested.
        char tail_overlap[MAX_LEN] = {'\0'};
        char tail_remainder[MAX_LEN] = {'\0'};
        int tail_overlap_len = 0;

        stroverlap(superstr_pointer->thestring, superstr_pointer->unprocessed[i], tail_overlap, tail_remainder);

        // if overlap exist at tail
        if (strlen(tail_overlap)){
            // if the overlap is longer than max_overlap
            if((tail_overlap_len = strlen(tail_overlap))>max_overlap){
                max_overlap = tail_overlap_len;
                max_location = superstr_pointer->unprocessed[i];
                *index = i;
            }
        }
        if (where == BOTH){
            char head_overlap[MAX_LEN] = {'\0'};
            char head_remainder[MAX_LEN] = {'\0'};
            int head_overlap_len = 0;
            stroverlap(superstr_pointer->unprocessed[i], superstr_pointer->thestring, head_overlap, head_remainder);
            // if overlap exist at head
            if (strlen(head_overlap)){
                if((head_overlap_len = strlen(head_overlap))>max_overlap){
                    max_overlap = head_overlap_len;
                    max_location = superstr_pointer->thestring;
                    *index = i;
                }
            }
        }

    }
    // If all remaining fragments have overlap 0, which means strlen(NULL) doesn't work properly, return the first
    // unprocessed
    if (!max_location){
        for (int j = 1; j < frag_count; j++){
            if(strlen(superstr_pointer->unprocessed[j])){
                max_location = superstr_pointer->unprocessed[j];
                *index = j;
                break;
            }
        }
    }
    return max_location;
}

int stage1(superstr_t superstr){
    print_output_header(1);

    // Declare a pointer point to local superstring.
    superstr_t *superstr_pointer = &superstr;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(superstr.thestring, superstr.unprocessed[INITIAL_FRAGMENT_INDEX]);

    // Iterate and append other fragments.
    int const frag_count = superstr_pointer->unprocessed_count;
    int i;
    for (i = 1; i < frag_count; i++){
        modify_superstr(superstr_pointer->thestring, superstr_pointer->unprocessed[i], TAIL);
        print_output(i, i, superstr_pointer->thestring, 0);
        if (i==frag_count-1){
            print_output(i, i, superstr_pointer->thestring, 1);
        }
    }
    return 0;
}

int stage3(superstr_t superstr){
    print_output_header(3);
    // Declare a pointer point to local superstr.
    superstr_t *superstr_pointer = &superstr;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(superstr.thestring, superstr.unprocessed[INITIAL_FRAGMENT_INDEX]);

    int index = 0;
    char* max = {'\0'};
    for (int i = 1; i < superstr.unprocessed_count; i++){
        max = max_overlap(superstr_pointer, &index, BOTH);
        int position = 0;
        if (!strcasecmp(max, superstr.thestring)){
            position = HEAD;
        }
        else if (strcasecmp(max, superstr.thestring)){
            position = TAIL;
        }
        modify_superstr(superstr_pointer->thestring, superstr.unprocessed[index], TAIL);
        // Remove the fragment which has been appended from the array
        // To mark it as processed
        print_output(i, index, superstr_pointer->thestring, 0);
        strerase(superstr.unprocessed[index]);
        if (i==superstr.unprocessed_count-1){
            print_output(i, i, superstr_pointer->thestring, 1);
        }
    }
    return 0;
}

int stage2(superstr_t superstr){
    print_output_header(2);
    // Declare a pointer point to local superstr.
    superstr_t *superstr_pointer = &superstr;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(superstr.thestring, superstr.unprocessed[INITIAL_FRAGMENT_INDEX]);
    int index = 0;
    char* max = {'\0'};
    for (int i = 1; i < superstr.unprocessed_count; i++){
        max = max_overlap(superstr_pointer, &index, TAIL);
        modify_superstr(superstr_pointer->thestring, superstr.unprocessed[index], TAIL);
        // Remove the fragment which has been appended from the array
        // To mark it as processed
        strerase(max);
        print_output(i, index, superstr_pointer->thestring, 0);
        if (i==superstr.unprocessed_count-1){
            print_output(i, i, superstr_pointer->thestring, 1);
        }
    }
}

void print_array(char array[MAX_NUM][MAX_LEN], int count){
    /* It takes an array and number of element in the array
     * Print all non-empty strings in the given array
     */
    for(size_t i = 0; i < count && array[i]; i++)
    {
        printf("Array[%d]: %s\n", i, array[i]);
    }
}

void print_output(int nth, int frg_no, char *superstr, int end){
    int len = (int)strlen(superstr);
    // Print output
    if (nth<=FIRST_TEN_FRAGMENT || !(nth%5) || end){
        if (end){
            printf("---\n");
            frg_no = LAST_ELEMENT_INDICATOR;
        }
        printf("%2d: frg=%2d, slen=%3d  ", nth, frg_no, len);
        print_restricted_superstr(superstr, len);
    }
}

void print_restricted_superstr(char* superstr, int superstrlen){
    // if the superstring has fewer characters than 54, print it directly.
    if (superstrlen<=THRESHOULD_LENGTH){
        printf("%s\n", superstr);
    }

        // if the superstring has more characters than 54.
    else if (superstrlen>THRESHOULD_LENGTH){
        // Declare and make a copy of the superstring to avoid any modification;
        char copy[superstrlen+1];
        strcpy(copy, superstr);
        memset(&copy[FIRST_25_CHARACTERS], '\0', sizeof(char));

        // Print the fist and last 25 characters.
        printf("%s .. %s\n", copy, &copy[superstrlen-FIRST_25_CHARACTERS]);
    }
}

void print_output_header(int stage_no){
    printf("\nStage %d Output\n--------------\n", stage_no);
}

int main(int agrc, char* agrv[]){
    superstr_t main_superstr;
    superstr_t *main_superstr_p = &main_superstr;
    initialize_superstr(main_superstr_p);

    // Read fragments to unprocessed in superstr and store return value as unprocessed_count.
    main_superstr.unprocessed_count = read_str(main_superstr.unprocessed);

    // Run stage 1;
    stage1(main_superstr);

    // Run stage 2;
    stage2(main_superstr);
    return 0;
}