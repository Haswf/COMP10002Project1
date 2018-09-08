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
#define MARKASPROCESSED_DEBUG 0
#define STROVERLAP_DEBUG 0
#define BEST_OPERATION_DEBUG 0
#define INSERT_TO_SUPERSTR_DEBUG 0

// Limitations of the fragments
#define MAX_LEN 21
#define MAX_NUM 1000

// Output-related constants
#define FIRST_25_CHARACTERS 25
#define THRESOULD_LENGTH 54
#define FIRST_TEN_FRAGMENT 10
#define LAST_ELEMENT_INDICATOR (-1)

// Status indicator used in struct fragment_t
#define PROCESSED 0
#define UNPROCESSED 1

//Operation Types
#define APPEND 0
#define INSERT 1

//JOINT POINT
#define TAIL_ONLY 0
#define HEAD_TAIL 1

// Struct fragment is used to store #of fragment, its content and if it's processed or not.
typedef struct _frg {int frg_index;  // The index of fragment
    char content[MAX_LEN];     // the content of the fragment
    int status; // status records processed or unprocessed
} fragment_t;
// Struct data_t is used to store input fragments and share them between helper functions.
typedef struct _data {char superstr[MAX_LEN*MAX_NUM]; // The superstring
    int frg_count; // #of fragments read
    fragment_t fragments[MAX_NUM];
} data_t;

typedef struct _operatoin {int type; // type of operation, insert(at the beginning) or append(to the end).
    char* dest; // destination where the src will be insert(append to)
    char* src; // source fragment.
    int src_index;
    char overlap[MAX_NUM*MAX_LEN]; // partial src which overlap with dest.
    char remainder[MAX_NUM*MAX_LEN]; // Non-overlap section of src.
} operation_t;

void print_output(int nth, int frg_no, char *superstr, int end);
void print_restricted_superstr(char *superstr, int superstrlen);
char* strtoupper(char* str, int index);
void print_output_header(int stage_no);
void append_to_superstr(char* superstr, char* dest, char* src, int src_index, char* overlap, char* remainder);
void insert_to_superstr(char* superstr, char* dest, char* src, int src_index, char* overlap, char* remainder);
void mark_as_processed(data_t *data, int frg_index);
void print_operation(operation_t *operation);
int best_operation(data_t *data, operation_t *best_p, int joint_point);

int read_str(fragment_t *fragments){
    int frag_counter = 0;
    int char_counter = 0;
    int input_length = 0;
    char cur_str[MAX_LEN];
    while (scanf("%s", cur_str)==1 && frag_counter < MAX_NUM){
        if ((input_length = (int)strlen(cur_str)) < MAX_LEN){
            // frag_counter only incrases by one when the input has been successfully read.
            // initialize_fragment_t(&fragments[frag_counter]); // Call cleanfragment to ensure it's zero-filled;
            if (strcpy(fragments[frag_counter].content, cur_str)){
                fragments[frag_counter].frg_index = frag_counter;
                fragments[frag_counter].status = UNPROCESSED;
                char_counter += input_length;
#if (DEBUG)
                printf("%d:\t %s\t %d\n",fragments[frag_counter].frg_no,fragments[frag_counter].content, fragments[frag_counter].status);
#endif
                frag_counter++;
            }
        }
    }
    print_output_header(0);
    printf("%d fragments read, %d characters in total\n", frag_counter, char_counter);
    return frag_counter;
}

char* strrcasestr(char* str, char* substr){
    char* p2s; // Pointer2string
    char *last_p = NULL;
    if ((p2s = strcasestr(str, substr))){
        for (int i = 0; ; i++) {
            if (!(p2s = strcasestr(p2s, substr))){
                break;
            }
            else{
                last_p = p2s;
                p2s = p2s+strlen(substr);
            }
        }
        p2s = last_p;
    }

    // if no substr is not found in str, return NULL
    return p2s;
}

char* last_char(char str[MAX_LEN]){
    /* return the last character in the string */
    return &str[strlen(str)-1];
}

int stroverlap(char* str, char* substr, operation_t *operation){
    /* Search for the overlap part of char* str and char* substr.
     * Copy the overlap part into char* overlap
     * Copy the non-overlap part of substr to char* remainder
     * return pointer where the largest overlap was found
     * return NULL if no overlap exists.
    */

    // Copy the entire substr to overlap
    strcpy(operation->overlap, substr);
    #if (STROVERLAP_DEBUG)
    printf("Search for %s in %s\n", operation->overlap, str);
    #endif (STROVERLAP_DEBUG)

    char* position = NULL;

    // Check if the whole fragment is presented in the superstring
    if (strrcasestr(str, substr)){
        position = strrcasestr(str, substr);
    }

        // Otherwise, check if there is any overlap between the head and the tail.
    else{

        for(int i = (int)strlen(substr)-1; i >= 0; i--){
            // if partial fragment is presented in the superstring and it is at the end of the superstring.
            if ((position = strrcasestr(str, operation->overlap)) && !strcasecmp(position, operation->overlap)){
                break;
            }
            // “Cross out” the last character in overlap.
            memset(last_char(operation->overlap),'\0', sizeof(char));
            // remainder get the "cross-outed" character.
            strcpy(operation->remainder, substr+i);
#if (STROVERLAP_DEBUG)
            printf("Overlap= %s \tRemainder:= %s\n", operation->overlap,operation->remainder);
#endif
        }
    }
    operation->src = substr;
    operation->dest = position;
    return (int)strlen(operation->overlap);
}

char* place_initial_fragment(char* superstr, char* substr){
    // Capitalize the first character of the first fragment.
    strtoupper(substr, INITIAL_CHARACTER_INDEX);
    // Use the first fragment to initialize a superstring.
    strcpy(superstr, substr);
    print_output(INITIAL_FRAGMENT_INDEX, INITIAL_FRAGMENT_INDEX, superstr, 0);
    return superstr;
}

char* strtoupper(char* str, int index){
    /* Capitalise the index-th character in the string
     * return the modified string
    */
    char upper_character = (char)toupper(str[index]);
    memset(&str[index], upper_character, sizeof(char));
    return str;
}

void print_output(int nth, int frg_index, char *superstr, int end){
    int len = (int)strlen(superstr);
    // Print output
    if (nth<=THRESOULD_LENGTH || !(nth%5) || end){
        if (end){
            printf("---\n");
            frg_index = LAST_ELEMENT_INDICATOR;
        }
        printf("%2d: frg=%2d, slen=%3d  ", nth, frg_index, len);
        print_restricted_superstr(superstr, len);
    }
}

void print_restricted_superstr(char *superstr, int superstrlen){
    // if the superstring has fewer characters than 54, print it directly.
    if (superstrlen<=THRESOULD_LENGTH){
        printf("%s\n", superstr);
    }

        // if the superstring has more characters than 54.
    else if (superstrlen>THRESOULD_LENGTH){
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

void print_operation(operation_t *operation){
    printf("type= %d\n", operation->type);
    printf("dest= %s\n", operation->dest);
    printf("src= %s\n", operation->src);
    printf("src_index= %d\n", operation->src_index);
    printf("overlap= %s\n", operation->overlap);
    printf("remainder= %s\n\n", operation->remainder);
}

int modify_superstr(data_t *data, operation_t *operation){
    if (operation->type == APPEND){
        append_to_superstr(data->superstr, operation->dest, operation->src, operation->src_index, operation->overlap, operation->remainder);
    }
    else if (operation->type == INSERT){
        operation->src = data->fragments[operation->src_index].content;
        insert_to_superstr(data->superstr, operation->dest, operation->src, operation->src_index, operation->overlap, operation->remainder);
    }
    mark_as_processed(data, operation->src_index);
    return 0;
}

void append_to_superstr(char* superstr, char* dest, char* src, int src_index, char* overlap, char* remainder){
    // the number of characters overlapping.
    int overlap_len = (int)strlen(overlap);

    // if the fragment appears wholly in the superstring.
    if (overlap_len==strlen(src)){
        // capitalize the first character of the fragment in the superstring.
        strtoupper(dest, INITIAL_CHARACTER_INDEX);
    }

        // No overlap exists.
    else if (!overlap_len){
        // capitalize the first character of the fragment.
        strtoupper(src, INITIAL_CHARACTER_INDEX);
        strncat(superstr, src, strlen(src));
    }

        // otherwise, overlap exists
    else {
        // capitalize the first character of the overlap.
        strtoupper(dest, INITIAL_CHARACTER_INDEX);
        // Append the remainder to the superstring.
        strncat(superstr, remainder, strlen(remainder));
    }
}

void insert_to_superstr(char* superstr, char* dest, char* src, int src_index, char* overlap, char* remainder){
    // capitalize the first character of the overlap.
    strtoupper(overlap, INITIAL_CHARACTER_INDEX);
    memset(dest, '\0', sizeof(char));

    char merge[strlen(src)+strlen(superstr)+1];
    strcpy(merge, strtoupper(src, INITIAL_CHARACTER_INDEX));
    strcat(merge, superstr);
    strncpy(superstr, merge, strlen(merge));
}

int best_operation(data_t *data, operation_t *best_p, int joint_point){

    // The max length of overlap found in unprocessed fragments.
    int max_overlap = 0;
    const int frag_counts = data->frg_count;
    // Iterate through unprocessed fragment
    for (int i= 1; i < frag_counts; i++){
        int insert_len = 0;
        int append_len = 0;
        if (data->fragments[i].status){
            //Initialize an operation.
            operation_t append_operation, insert_operation;
            append_len = stroverlap(data->superstr, data->fragments[i].content, &append_operation);

#if (BEST_OPERATION_DEBUG)
            printf("i = %d\n", i);
            printf("APPEND: \tLEN= %d\n",append_len);
            print_operation(&append_operation);
#endif
            if (append_len > max_overlap){
                max_overlap = append_len;
                *best_p = append_operation;
                best_p->src_index = i;
                best_p->type = APPEND;
            }

            if (joint_point == HEAD_TAIL){
                insert_len = stroverlap(data->fragments[i].content, data->superstr, &insert_operation);
                if (insert_len > max_overlap){
                    max_overlap = insert_len;
                    *best_p = insert_operation;
                    best_p->src_index = i;
                    best_p->type = INSERT;
                }
            }
#if (BEST_OPERATION_DEBUG)
            printf("INSERT: \tLEN= %d\n",insert_len);
            print_operation(&insert_operation);
#endif
        }
    }
    if (max_overlap==0){
        int j;
        for (j = 1; (j < frag_counts); j++){
            if (data->fragments[j].status){
                break;
            }
            else {
                continue;
            }
        }
#if (BEST_OPERATION_DEBUG)
        printf("WARNING: No overlap found");
        printf("%d %s\n", j, data->fragments[j].content);
#endif

        best_p->dest = data->superstr;
        best_p->src = data->fragments[j].content;
        best_p->type = APPEND;
        best_p->src_index = data->fragments[j].frg_index;
        memset(best_p->overlap, '\0', sizeof(char));
        strcpy(best_p->remainder, data->fragments[j].content);
    }
#if (BEST_OPERATION_DEBUG)
    printf("--------------------------BEST-----------------------\n");
    print_operation(best_p);
#endif
    return 0;
}

int stage1(data_t data){
    print_output_header(1);

    // Declare a pointer point to local superstring.
    data_t* p2d = &data;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(data.superstr, data.fragments[INITIAL_FRAGMENT_INDEX].content);

    // Iterate and append other fragments.

    int const fragment_count = data.frg_count;
    for (int i = 1; i < fragment_count; i++){
        operation_t operation;
        stroverlap(data.superstr, data.fragments[i].content, &operation);
        operation.src_index = i;
        operation.type = APPEND;

        modify_superstr(&data, &operation);
        print_output(i, i, data.superstr, 0);
        if (i==data.frg_count-1){
            print_output(i, i, data.superstr, 1);
        }
    }
    return 0;
}

int stage2(data_t data){
    print_output_header(2);

    // Declare a pointer point to local superstring.
    data_t* p2d = &data;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(data.superstr, data.fragments[INITIAL_FRAGMENT_INDEX].content);
    operation_t best;
    // initialize_operation_t(&best);
    for (int i = 1; i < data.frg_count; i++){
        best_operation(p2d, &best, TAIL_ONLY);
        modify_superstr(p2d, &best);
        print_output(i, best.src_index, data.superstr, 0);
        if (i==data.frg_count-1){
            print_output(i, best.src_index, data.superstr, 1);
        }
    }
    return 0;
}

int stage3(data_t data){
    print_output_header(3);

    // Declare a pointer point to local superstring.
    data_t* p2d = &data;

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(data.superstr, data.fragments[INITIAL_FRAGMENT_INDEX].content);
    operation_t best;

    for (int i = 1; i < data.frg_count; i++){
        best_operation(p2d, &best, HEAD_TAIL);
        modify_superstr(p2d, &best);
        print_output(i, best.src_index, data.superstr, 0);
        if (i==data.frg_count-1){
            print_output(i, best.src_index, data.superstr, 1);
        }
    }
    return 0;
}

void mark_as_processed(data_t *data, int frg_index){
    int retval;
#if (MARKASPROCESSED_DEBUG)
    printf("Status of fragment %s (%d) before exec = %d\n", data->fragments[frg_index].content, frg_index, data->fragments[frg_index].status);
#endif
    // If the fragment specificed have been processed
    if (data->fragments[frg_index].status){
        data->fragments[frg_index].status = PROCESSED;
    }
    else if (!data->fragments[frg_index].status){
        printf("ERROR: Fragment %s (%d) have already been marked as processed!\n", data->fragments[frg_index].content, frg_index);
        exit(EXIT_FAILURE);
    }
#if (MARKASPROCESSED_DEBUG)
    printf("Status of fragment %s (%d) after exec = %d\n", data->fragments[frg_index].content, frg_index, data->fragments[frg_index].status);
#endif
}

int main(int argc, char* argv[]){
    data_t data;
    data_t *p2d = &data; // Create pointer p2d(pointer2date).
    // initialize_data_t(p2d);

    // Read fragments and save return value as frg_count.
    data.frg_count = read_str(data.fragments);

    // Run stage1
    stage1(data);

    // Run stage2
    stage2(data);

    // Run stage3
    stage3(data);

    return 0;
}