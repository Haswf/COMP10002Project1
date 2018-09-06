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

// Overlap indicator
#define NO_OVERLAP 0
#define OVERLAP 1
#define WITHIN 2

// Output-related constants
#define FIRST_25_CHARACTERS 25
#define THRESHOULD_LENGTH 54
#define FIRST_TEN_FRAGMENT 10
#define LAST_ELEMENT_INDICATOR -1

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
int append_to_superstr(char* superstr, char* substr);
int strerase(char *str);
char* max_overlap(superstr_t *superstr_pointer, int* index);
void print_restricted_superstr(char* superstr, int superstrlen);
void print_output(int nth, int frg_no, char *superstr, int end);
void print_output_header(int stage_no);
int stage0(superstr_t superstr);
int stage1(superstr_t superstr);

//DEBUGGING UTILITIES
void print_array(char array[MAX_NUM][MAX_LEN], int count);

int read_str(char strarr[MAX_NUM][MAX_LEN]){
    int frag_counter = 0;
    int char_counter = 0;
    int input_length = 0;
    char cur_str[MAX_LEN];
    while (scanf("%s", cur_str)==1 && frag_counter < MAX_NUM){
        if ((input_length = strlen(cur_str)) < MAX_LEN){
            // frag_counter only incrases by one when the input has been successfully read.
            if (strcpy(strarr[frag_counter], cur_str)){
                char_counter += input_length;
                #if (DEBUG)
                printf("%d: %s\n", frag_counter, strarr[frag_counter]);
                #endif
                frag_counter++;
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
    char *last_p;
    for (int i = 0; ; ++i) {
        if (!(p = strcasestr(p, substr))){
            break;
        }
        last_p = p;
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

    for(int i = strlen(substr)-1; i >= 0; i--){
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
#if (OVERLAPDEBUG)
        printf("overlap= %s, remainder= %s\n", overlap, remainder);
#endif
        memset(last_char(overlap),'\0', sizeof(char));
        // remainder get the "cross-outed" character.
        strcpy(remainder, substr+i);
        }
    }
    return position;
}

char* strtoupper(char* str, int index){
    /* Capitalise the index-th character in the string
     * return the modified string
    */
    char upper_character = toupper(str[index]);
    memset(str, upper_character, sizeof(char));
    return str;
}

char* place_initial_fragment(char* superstr, char* substr){
    // If the argument substr is empty, return;
    if (!strlen(substr)){
        return superstr;
    }
    // Capitalize the first character of the first fragment.
    strtoupper(substr, INITIAL_CHARACTER_INDEX);
    // Use the first fragment to initialize a superstring.
    strcpy(superstr, substr);
//    printf("%2d: frg= %2d, slen= %2d  ", INITIAL_FRAGMENT_INDEX, INITIAL_FRAGMENT_INDEX, (int)strlen(superstr));
//    print_output(superstr, INITIAL_FRAGMENT_INDEX);
    print_output(INITIAL_FRAGMENT_INDEX, INITIAL_FRAGMENT_INDEX, superstr, 0);
    return superstr;
}

int append_to_superstr(char* superstr, char* substr){
    /* Append substr to superstr with longest overlap possible
     * return the overlap indicator, see the head of this code
     */

    int overlap_indicator;

    //0-filled overlap and remainder
    char overlap[MAX_LEN] = {'\0'};
    char remainder[MAX_LEN] = {'\0'};

    // char* position point to where the overlap exists in superstring.
    char* position;
    position = stroverlap(superstr, substr, &overlap, &remainder);

    // the number of characters overlapping.
    int overlap_len = strlen(overlap);

    // if the fragment appears wholly in the superstring.
    if (overlap_len==strlen(substr)){
        // capitalize the first character of the fragment in the superstring.
        strtoupper(position, INITIAL_CHARACTER_INDEX);
        overlap_indicator = WITHIN;

    }

    // No overlap exists.
    else if (!overlap_len){
        // capitalize the first character of the fragment.
        strtoupper(substr, INITIAL_CHARACTER_INDEX);
        strcat(superstr, substr);
        overlap_indicator = NO_OVERLAP;
    }

    // otherwise, overlap exists
    else {
        // capitalize the first character of the overlap.
        strtoupper(position, INITIAL_CHARACTER_INDEX);
        // Append the remainder to the superstring.
        strcat(superstr, remainder);
        overlap_indicator = OVERLAP;
    }
    return overlap_indicator;
}

int strerase(char *str){
    memset(str, '\0', sizeof(char));
    if (!strlen(str)){
        return 0;
    }
    else{
        return 1;
    }
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
        append_to_superstr(superstr_pointer->thestring, superstr_pointer->unprocessed[i]);
        print_output(i, i, superstr_pointer->thestring, 0);
        if (i==frag_count-1){
            print_output(i, i, superstr_pointer->thestring, 1);
        }
    }
    return i;
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
        max = max_overlap(superstr_pointer, &index);
        append_to_superstr(superstr_pointer->thestring, max);
        // Remove the fragment which has been appended from the array
        // To mark it as processed
        strerase(max);
        print_output(i, index, superstr_pointer->thestring, 0);
        if (i==superstr.unprocessed_count-1){
            print_output(i, i, superstr_pointer->thestring, 1);
        }
    }
}


char* max_overlap(superstr_t *superstr_pointer, int *index){

    int const frag_count = superstr_pointer->unprocessed_count;
    // The max length of overlap found in unprocessed fragments.
    int max_overlap = 0;
    char* max_location = {'\0'};

    // Iterate through unprocessed fragment
    for (int i= 1; i < frag_count && superstr_pointer->unprocessed[i]; i++){
        size_t cur_overlap = 0;
        //Initialize overlap and remainder for each fragment to be tested.
        char overlap[MAX_LEN] = {'\0'};
        char remainder[MAX_LEN] = {'\0'};

        stroverlap(superstr_pointer->thestring, superstr_pointer->unprocessed[i], &overlap, &remainder);

        // if overlap exist...
        if (strlen(overlap)){
            cur_overlap = strlen(overlap);
            if(cur_overlap>max_overlap){
                max_overlap = cur_overlap;
                max_location = superstr_pointer->unprocessed[i];
                *index = i;
            }
        }
#if (DEBUG)
        printf("%d: Str= %s, cur= %d, max= %d, location= %s\n", i,
               superstr_pointer->unprocessed[i], cur_overlap, max_overlap, max_location);
#endif
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

void print_array(char array[MAX_NUM][MAX_LEN], int count){
    /* It takes an array and number of element in the array
     * Print all non-empty strings in the given array
     */
    for(size_t i = 0; i < count && array[i]; i++)
    {
        printf("%s\n", array[i]);
    }
}

void print_output(int nth, int frg_no, char *superstr, int end){
    int len = strlen(superstr);
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
    else if (superstr>THRESHOULD_LENGTH){
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