/* Solution to comp10002 Assignment 1, 2018 semester 2.

   Authorship Declaration:

   I certify that the program contained in this submission is completely my
   own individual work, except where explicitly noted by comments that
   provide details otherwise.  I understand that work that has been
   developed by another student, or by me in collaboration with other
   students, or by non-students as a result of request, solicitation, or
   payment, may not be submitted for assessment in this subject.  I further
   understand that submitting for assessment work developed by or in
   collaboration with other students or non-students constitutes Academic
   Misconduct, and may be penalized by mark deductions, or by other
   penalties determined via the University of Melbourne Academic Honesty
   Policy, as described at https://academicintegrity.unimelb.edu.au.

   I further certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor
   will I do so until after the marks are released. I understand that
   providing my work to other students, regardless of my intention or any
   undertakings made to me by that other student, is also Academic
   Misconduct.

   Signed by: Shuyang Fan 988301
   Dated:     September 10th, 2018

*/

#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
//Include strings.h to use strcasecmp.
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>

#define INITIAL_FRAGMENT_INDEX 0
#define INITIAL_CHARACTER_INDEX 0

// DEBUGGING PANEL
#define READSTR_DEBUG 0
#define STROVERLAP_DEBUG 0
#define BEST_OPERATION_DEBUG 0
#define  MODIFY_DEBUG 0

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

// Structure fragment is used to store #of fragment, its content and if it's processed or not.
typedef struct {int frg_index;  // The index of fragment
    char content[MAX_LEN];     // the content of the fragment
    int status; // status records processed or unprocessed
} fragment_t;
// Structure data_t is used to store input fragments and share them between helper functions.
typedef struct {char superstr[MAX_LEN*MAX_NUM]; // The superstring
    int frg_count; // #of fragments read
    fragment_t fragments[MAX_NUM];
} data_t;

typedef struct {int type; // type of operation, insert(at the beginning) or append(to the end).
    char* dest; // destination where the src will be insert(append to)
    char* src; // source fragment.
    int src_index;
    char overlap[MAX_NUM*MAX_LEN]; // partial src which overlap with dest.
    char remainder[MAX_NUM*MAX_LEN]; // Non-overlap section of src.
} operation_t;

/* function prototypes */
int read_str(fragment_t *fragments_p);
char* strrcasestr(char* str, char* substr);
char* last_char(char* str);
int stroverlap(char* str, char* substr, operation_t *operation);
char* place_initial_fragment(char* superstr, char* substr);
char* strtoupper(char* str, int index);
void print_output(int nth, int frg_index, char *superstr, int end);
void print_restricted_superstr(char *superstr, int superstrlen);
void print_output_header(int stage_no);
void print_operation(operation_t *operation_p);
int modify_superstr(data_t*, operation_t*);
void append_to_superstr(char* superstr, char* dest, char* src, char* overlap, char* remainder);
void insert_to_superstr(char* superstr, char* dest, char* src, char* overlap);
int best_operation(data_t *data_p, operation_t *best_p, int joint_point);
int stage1(data_t data);
int stage2(data_t data);
int stage3(data_t data);
void mark_as_processed(data_t *data_p, int frg_index);

int read_str(fragment_t *fragments_p){
    int frag_counter = 0;
    int char_counter = 0;
    int input_length = 0;
    char cur_str[MAX_LEN];
    while (scanf("%s", cur_str)==1 && frag_counter < MAX_NUM){
        if ((input_length = (int)strlen(cur_str)) < MAX_LEN){
            // frag_counter only incrases by one when the input has been successfully read.
            // initialize_fragment_t(&fragments[frag_counter]);
            if (strncpy(fragments_p[frag_counter].content, cur_str, strlen(cur_str))){
                fragments_p[frag_counter].frg_index = frag_counter;
                fragments_p[frag_counter].status = UNPROCESSED;
                char_counter += input_length;
#if (READSTR_DEBUG)
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
    // if no substr was found in str, return NULL
    return p2s;
}

char* last_char(char* str){
    /* return the last character in the string */
    return &str[strlen(str)-1];
}

int stroverlap(char* str, char* substr, operation_t *operation){
    /* Search for the overlap part of str and char* substr.
     * Store how to merge them with minimum length in operation.
     * return length of overlap
    */

    // Copy the entire substr to overlap
    strcpy(operation->overlap, substr);
    #if (STROVERLAP_DEBUG)
    printf("Search for %s in %s\n", operation->overlap, str);
    #endif

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
     * return the modified string. */
    char upper_character = (char)toupper(str[index]);
    memset(&str[index], upper_character, sizeof(char)); // Capitalised the character
    return str;
}

void print_output(int nth, int frg_index, char *superstr, int end){
    int len = (int)strlen(superstr);
    // Print output
    if (nth<=FIRST_TEN_FRAGMENT || !(nth%5) || end){
        if (end){   // if it is indicated that this is the last fragment processed
            printf("---\n");
            frg_index = LAST_ELEMENT_INDICATOR;
        }
        printf("%2d: frg=%2d, slen=%3d  ", nth, frg_index, len);
        print_restricted_superstr(superstr, len); // Call print_restricted_superstr to print formatted output.
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

void print_operation(operation_t *operation_p){
    printf("type= %d\n", operation_p->type);
    printf("dest= %s\n", operation_p->dest);
    printf("src= %s\n", operation_p->src);
    printf("src_index= %d\n", operation_p->src_index);
    printf("overlap= %s\n", operation_p->overlap);
    printf("remainder= %s\n\n", operation_p->remainder);
}

int modify_superstr(data_t *data_p, operation_t *operation_p){
#if (MODIFY_DEBUG)
    printf("Superstr before oprt= %s\n", data->superstr);
#endif
    if (operation_p->type == APPEND){
        append_to_superstr(data_p->superstr, operation_p->dest, operation_p->src, operation_p->overlap, operation_p->remainder);
    }
    else if (operation_p->type == INSERT){
        operation_p->src = data_p->fragments[operation_p->src_index].content;
        insert_to_superstr(data_p->superstr, operation_p->dest, operation_p->src, operation_p->overlap);
    }
    mark_as_processed(data_p, operation_p->src_index);
#if (MODIFY_DEBUG)
    printf("----------OPERATION--------\n");
    print_operation(operation);
    printf("Superstr after oprt= %s\n", data->superstr);
#endif
    return 0;
}

void append_to_superstr(char* superstr, char* dest, char* src, char* overlap, char* remainder){
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

    // otherwise, partial overlap exists
    else {
        // capitalize the first character of the overlap.
        strtoupper(dest, INITIAL_CHARACTER_INDEX);
        // Append the remainder to the superstring.
        strncat(superstr, remainder, strlen(remainder));
    }
}

void insert_to_superstr(char* superstr, char* dest, char* src, char* overlap){
    // capitalize the first character of the overlap.
    strtoupper(overlap, INITIAL_CHARACTER_INDEX);
    memset(dest, '\0', sizeof(char));
    char merge[strlen(src)+strlen(superstr)+1];
    strcpy(merge, strtoupper(src, INITIAL_CHARACTER_INDEX));
    strcat(merge, superstr);
    strncpy(superstr, merge, strlen(merge));
}

int best_operation(data_t *data_p, operation_t *best_p, int joint_point){
    /* best operation takes structure data_t pointer, operation_t pointer and an int joint_point
     * stands for type of operation expected.
     * Then, it found the fragment gives the most overlap and store instructions on how to modify the
     * the superstring in best_p.
     */

    // The max length of overlap found in unprocessed fragments.
    int max_overlap = 0;

    // Iterate through unprocessed fragment
    for (int i= 1; i < data_p->frg_count; i++){
        int insert_len = 0;
        int append_len = 0;
        if (data_p->fragments[i].status){
            //Initialize an operation.
            operation_t append_operation, insert_operation;
            append_len = stroverlap(data_p->superstr, data_p->fragments[i].content, &append_operation);

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
                insert_len = stroverlap(data_p->fragments[i].content, data_p->superstr, &insert_operation);
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
    // zero-overlap obtained for all fragments
    if (!max_overlap){
        int j;
        for (j = 1; (j < data_p->frg_count); j++){
            // Place the first unprocessed fragment at the end of the superstring.
            if (data_p->fragments[j].status){
                break;
            }
        }
        // Generate operation.
        best_p->dest = data_p->superstr;
        best_p->src = data_p->fragments[j].content;
        best_p->type = APPEND;
        best_p->src_index = data_p->fragments[j].frg_index;
        memset(best_p->overlap, '\0', sizeof(char));
        strcpy(best_p->remainder, data_p->fragments[j].content);
    }
#if (BEST_OPERATION_DEBUG)
    printf("--------------------------BEST-----------------------\n");
    print_operation(best_p);
#endif
    return 0;
}

int stage1(data_t data){
    print_output_header(1);

    // Place the first fragment to initialize a superstring.
    place_initial_fragment(data.superstr, data.fragments[INITIAL_FRAGMENT_INDEX].content);

    // Iterate and append other fragments.
    for (int i = 1; i < data.frg_count; i++){
        operation_t operation;
        stroverlap(data.superstr, data.fragments[i].content, &operation);
        operation.src_index = i;
        // Set operation type to APPEND for stage 1.
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
        // Call best_operation to choose the fragment that gives the most overlap.
        best_operation(p2d, &best, HEAD_TAIL);
        // Modify the superstring according to the operation.
        modify_superstr(p2d, &best);
        print_output(i, best.src_index, data.superstr, 0);
        if (i==data.frg_count-1){
            print_output(i, best.src_index, data.superstr, 1);
        }
    }
    return 0;
}

void mark_as_processed(data_t *data_p, int frg_index){
     /* This function takes a pointer to structure data, and a fragment index
      * marks the fragment specified by the index number as processed(~0)
     */
    // If the fragment specified have not been processed
    if (data_p->fragments[frg_index].status){
        data_p->fragments[frg_index].status = PROCESSED;
    }
    // If the fragment specified have been processed
    else if (!data_p->fragments[frg_index].status){
        printf("ERROR: Fragment %s (%d) have already been marked as processed!\n", data_p->fragments[frg_index].content, frg_index);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    /* Driver function for stage 1, 2 and 3. */
    data_t data;
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