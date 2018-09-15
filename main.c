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

/* includes strings.h to utilize function strcasecmp */
#include <strings.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

#define INITIAL_FRAGMENT_INDEX 0 /* Index used to obtain first fragment*/
#define INITIAL_CHARACTER_INDEX 0 /* Index used to obtain first character */

/* Limitations of the fragments */
#define MAX_LEN 20 /* No string fragment will be longer than 20 characters */
#define MAX_NUM 1000 /* At most 1000 fragments supplied */

/* Output-related constants */
#define FIRST_25_CHARACTERS 25
#define THRESHOLD_LENGTH 54
#define FIRST_TEN_FRAGMENT 10
#define LAST_ELEMENT_INDICATOR (-1)

/* Status indicator in struct fragment_t */
#define PROCESSED 0 /* 0 stands for processed */
#define UNPROCESSED 1 /* 1 stands for unprocessed */

/* Operation Types */
#define APPEND 0 /* 0 stands for appending */
#define INSERT 1 /* 1 stands for insertion */

/* Joint point */
#define END_ONLY 0 /* only check the end of the superstr */
#define HEAD_END 1 /* check both the head and end of the superstr */

/* Structure fragment is used to store fragments read and relevant
 * information. */
typedef struct {int frg_index;  /* index of the fragment */
    char content[MAX_LEN+1];    /* content of the fragment */
    int status; /*  processed or unprocessed */
} fragment_t;

/* Structure data_t is used to store input fragments */
typedef struct {char superstr[(MAX_LEN+1)*MAX_NUM]; /* the superstring */
    int frg_count; /* #of fragments read in total */
    fragment_t fragments[MAX_NUM]; /* fragments */
} data_t;

/* Structure operation_t stores instructions on how to modify the superstring */
typedef struct {int type; /* type of operation */
    char* dest; /* destination where the src will be insert(append) to */
    char* src; /* source fragment */
    int src_index; /* source fragment index */
    char overlap[MAX_NUM*(MAX_LEN+1)]; /* partial src which overlaps with dest*/
    char remainder[MAX_NUM*(MAX_LEN+1)]; /* Non-overlap part of src */
} operation_t;

/* function prototypes */
int mygetchar();
int read_str(fragment_t *fragments_p);
char* strrcasestr(char* haystack, char* needle);
char* last_char(char* str);
int stroverlap(char* str, char* substr, int frg_index, operation_t *operation);
char* place_initial_fragment(char* superstr, char* substr);
char* strtoupper(char* str, int index);
void print_output(int nth, int frg_index, char *superstr, int end);
void print_restricted_superstr(char *superstr, int superstrlen);
void print_output_header(int stage_no);
int modify_superstr(data_t*, operation_t*);
void append_to_superstr(char* superstr, char* dest,
                        char* src, char* overlap, char* remainder);
void insert_to_superstr(char* superstr, char* dest, char* src, char* overlap);
int find_best_operation(data_t *data_p, operation_t *best_p, int joint_point);
void mark_as_processed(data_t *data_p, int frg_index);
int find_unprocessed_fragment(data_t *data_p, operation_t *best_p);
int stage1(data_t data);
int stage2(data_t data);
int stage3(data_t data);

int mygetchar(){
    /* The following function was written by Alistair Moffat obtained from
     * https://people.eng.unimelb.edu.au/ammoffat/teaching/10002/ass1/
     * All credit goes to him.
     */
    int c;
    while ((c=getchar())=='\r'){
    }
    return c;
}

int read_str(fragment_t *fragment_p){
    /* read_str read fragments one by one from input and
     * save them in an array of fragment_t.
     * returns number of fragment read*/

    int char_count = 0; /* number of characters read in total */
    int char_index = 0; /* index of character in current fragment */
    int frag_index = 0; /* number of fragment read in total*/
    char ch;
    while (((ch = (char)mygetchar())!= EOF) && frag_index <= MAX_NUM)
    {
        /* if current character read is not the end of a fragment */
        if (ch!='\n' && isalnum(ch) && (char_index <= (MAX_LEN+1))){
            fragment_p[frag_index].content[char_index++] = ch;
        }
        /* if current character read is the end of a fragment
         * pack it and move to the next fragment_t */
        else if (ch == '\n'){
            fragment_p[frag_index].frg_index = frag_index;
            /* Place NULL terminator to end the sting */
            fragment_p[frag_index].content[char_index] = '\0';
            /* Update status as unprocessed */
            fragment_p[frag_index].status = UNPROCESSED;
            frag_index++;
            char_count += char_index;
            /* reset char_index to 0 to read next fragment */
            char_index = 0;
        }
    }
    print_output_header(0);
    printf("%d fragments read, %d characters in total\n",
           frag_index, char_count);
    return frag_index;
}

char* strrcasestr(char* haystack, char* needle){
    /* The strrcasestr() locates a substring in a string,
     * returns pointer in string to last place where substring occurs,
     * returns NULL if substring does not occur in string,
     * doing case-insensitive tests */

    char* p2s; /* pointer to last occurrence in superstring */
    char *last_p = NULL;

    if ((p2s = strcasestr(haystack, needle))){
        int i = 0;
        for (i = 0; ; i++){
            /* if substring does not occur in superstring
             * Confirm the previous location is the last occurrence
             * break */
            if (!(p2s = strcasestr(p2s, needle))){
                break;
            }

            /* if substring occurs in the superstring, search
             * the rest of the superstring to locate the last
             * occurrence */
            else{
                last_p = p2s;
                p2s += strlen(needle);
            }
        }
        p2s = last_p;
    }
    return p2s;
}

char* last_char(char* str){
    /* return the last character in the string */
    return &str[strlen(str)-1];
}

int stroverlap(char* str, char* substr, int frg_index, operation_t *operation){
    /* Search for the overlap part of str and char* substr.
     * Store how to merge them with minimum length in operation.
     * return length of overlap
    */

    /* Copy the entire substring to overlap */
    strcpy(operation->overlap, substr);
    char* position = NULL;

    /* Check if the whole fragment is presented in the superstring */
    if (strrcasestr(str, substr)){
        position = strrcasestr(str, substr);
    }

        /* Otherwise, search for overlap between the head and the tail */
    else{
        int i;
        for(i = (int)strlen(substr)-1; i >= 0; i--){
            /* if partial fragment is presented in the superstring and
                it is at the end of the superstring. */
            if ((position = strrcasestr(str, operation->overlap)) &&
                    !strcasecmp(position, operation->overlap)){
                break;
            }
            /* “cross out” the last character in overlap */
            memset(last_char(operation->overlap),'\0', sizeof(char));
            /* remainder get the "cross-outed" character */
            strcpy(operation->remainder, substr+i);
        }
    }
    operation->src = substr;
    operation->dest = position;
    operation->src_index = frg_index;
    return (int)strlen(operation->overlap);
}

char* place_initial_fragment(char* superstr, char* substr){
    /* Capitalize the first character of the first fragment */
    strtoupper(substr, INITIAL_CHARACTER_INDEX);
    /* Use the first fragment to initialize a superstring */
    strcpy(superstr, substr);
    print_output(INITIAL_FRAGMENT_INDEX, INITIAL_FRAGMENT_INDEX, superstr, 0);
    return superstr;
}

char* strtoupper(char* str, int index){
    /* Capitalise the index-th character in the string
     * return the modified string. */
    char upper_character = (char)toupper(str[index]);
    /* Capitalise the character */
    memset(&str[index], upper_character, sizeof(char));
    return str;
}

void print_output(int nth, int frg_index, char *superstr, int end){
    int len = (int)strlen(superstr);

    if (nth<=FIRST_TEN_FRAGMENT || !(nth%5) || end){
        /* if it is indicated that this is the last fragment processed */
        if (end){
            printf("---\n");
            frg_index = LAST_ELEMENT_INDICATOR;
        }
        printf("%2d: frg=%2d, slen=%3d  ", nth, frg_index, len);
        /* Call print_restricted_superstr to print formatted output */
        print_restricted_superstr(superstr, len);
    }
}

void print_restricted_superstr(char *superstr, int superstrlen){
    /* The print_restricted_superstr(char *superstr, int superstrlen) prints
     * output if the len is within 54. Otherwise, it prints the first 25
     * characters and the last 25 characters.
     */

    /* if the superstring has fewer than 54 characters, print it directly */
    if (superstrlen<=THRESHOLD_LENGTH){
        printf("%s\n", superstr);
    }

    /* if the superstring has more than 54 characters */
    else if (superstrlen>THRESHOLD_LENGTH){
        /* Declare and make a copy of the superstring to avoid any
         * modification */
        char copy[superstrlen+1];
        strcpy(copy, superstr);
        /* Place a NULL terminator to "cut" first 25 characters */
        memset(&copy[FIRST_25_CHARACTERS], '\0', sizeof(char));

        /* Print the first and last 25 characters */
        printf("%s .. %s\n", copy, &copy[superstrlen-FIRST_25_CHARACTERS]);
    }
}

void print_output_header(int stage_no){
    /* The print_output_header(int stage_no) prints stage header. */
    printf("\nStage %d Output\n--------------\n", stage_no);
}

int modify_superstr(data_t *data_p, operation_t *operation_p){
    /* if operation_type is append, unpack operation_t, call
     * append_to_superstr and pass relevant parameters */
    if (operation_p->type == APPEND){
        append_to_superstr(data_p->superstr, operation_p->dest,
                           operation_p->src, operation_p->overlap,
                           operation_p->remainder);
    }
    /* if operation_type is insert, unpack operation_t, call
     * append_to_superstr and pass relevant parameters */
    else if (operation_p->type == INSERT){
        operation_p->src = data_p->fragments[operation_p->src_index].content;
        insert_to_superstr(data_p->superstr, operation_p->dest,
                           operation_p->src, operation_p->overlap);
    }
    /* Mark the fragment involved in this operation as processed */
    mark_as_processed(data_p, operation_p->src_index);

    return 0;
}

void append_to_superstr(char* superstr, char* dest, char* src,
                        char* overlap, char* remainder){
    /* the number of characters overlapping */
    int overlap_len = (int)strlen(overlap);

    /* if the fragment appears wholly in the superstring */
    if (overlap_len==strlen(src)){
        /* capitalize the first character of the fragment in the superstring */
        strtoupper(dest, INITIAL_CHARACTER_INDEX);
    }

    /* if no overlap exists */
    else if (!overlap_len){
        /* capitalize the first character of the fragment */
        strtoupper(src, INITIAL_CHARACTER_INDEX);
        strncat(superstr, src, strlen(src));
    }

    /* otherwise, partial overlap exists */
    else {
        /* capitalize the first character of the overlap */
        strtoupper(dest, INITIAL_CHARACTER_INDEX);
        /* Append the remainder to the superstring */
        strncat(superstr, remainder, strlen(remainder));
    }
}

void insert_to_superstr(char* superstr, char* dest, char* src, char* overlap){
    /* capitalize the first character of the overlap */
    strtoupper(overlap, INITIAL_CHARACTER_INDEX);
    memset(dest, '\0', sizeof(char));

    /* Combine parts to string merge */
    char merge[strlen(src)+strlen(superstr)+1];
    strcpy(merge, strtoupper(src, INITIAL_CHARACTER_INDEX));
    strcat(merge, superstr);

    /* Copy merge to superstr */
    strncpy(superstr, merge, strlen(merge));
}

int find_best_operation(data_t *data_p, operation_t *best_p, int joint_point){
    /* find_best_operation takes structure data_t pointer,
     * operation_t pointer and an int joint_point stands for type of
     * operation expected. Then, it found the fragment gives the most overlap
     * and store instructions on how to modify the the superstring in best_p.
     */

    /* the max length of overlap found in unprocessed fragments */
    int max_overlap = 0;
    int i;

    /* iterate through unprocessed fragments */
    for (i= 1; i < data_p->frg_count; i++){
        int insert_len = 0;
        int append_len = 0;
        if (data_p->fragments[i].status){
            operation_t append_operation, insert_operation;
            /* Compute length of overlap with the end of superstring */
            append_len = stroverlap(data_p->superstr, data_p->fragments[i]
                    .content, i, &append_operation);

            if (append_len > max_overlap){
                max_overlap = append_len;
                *best_p = append_operation;
                best_p->type = APPEND;
            }

            if (joint_point == HEAD_END){
                /* Compute length of overlap with the head of superstring */
                insert_len = stroverlap(data_p->fragments[i].content,
                        data_p->superstr, i, &insert_operation);
                /* if it is better than append operation, set it as the best
                 * operation*/
                if (insert_len > max_overlap){
                    max_overlap = insert_len;
                    *best_p = insert_operation;
                    best_p->type = INSERT;
                }
            }
        }
    }
    /* if no overlap are found for the fragment in the head and in the end */
    if (!max_overlap){
        /* Find the first unprocessed fragment */
        find_unprocessed_fragment(data_p, best_p);
    }
    return 0;
}

int find_unprocessed_fragment(data_t *data_p, operation_t *best_p){
    /* Find the first unprocessed fragment, then generate corresponding
     * operation*/
    int j;
    for (j = 1; (j < data_p->frg_count); j++){
        /* Place the first unprocessed fragment at the end of the superstring */
        if (data_p->fragments[j].status){
            break;
        }
    }
    /*  Generate operation */
    best_p->dest = data_p->superstr;
    best_p->src = data_p->fragments[j].content;
    best_p->type = APPEND;
    best_p->src_index = data_p->fragments[j].frg_index;
    memset(best_p->overlap, '\0', sizeof(char));
    strcpy(best_p->remainder, data_p->fragments[j].content);
    return 0;
}

int stage1(data_t data){
    print_output_header(1);

    /* Declare a pointer point to local copy of data */
    data_t* p2d = &data;

    /* Place the first fragment to initialize a superstring */
    place_initial_fragment(data.superstr,
                           data.fragments[INITIAL_FRAGMENT_INDEX].content);

    int i;
    /* Iterate and append other fragments. */
    for (i = 1; i < data.frg_count; i++){
        operation_t operation;
        stroverlap(data.superstr, data.fragments[i].content, i, &operation);
        operation.src_index = i;
        /* Set operation type to APPEND for stage 1 */
        operation.type = APPEND;
        /* Call modify_superstr */
        modify_superstr(p2d, &operation);
        print_output(i, i, data.superstr, 0);

        /* inform print_out if the last fragment is reached*/
        if (i==data.frg_count-1){
            print_output(i, i, data.superstr, 1);
        }
    }
    return 0;
}

int stage2(data_t data){
    print_output_header(2);

    /* Declare a pointer point to local copy of data */
    data_t* p2d = &data;

    /* Place the first fragment to initialize a superstring */
    place_initial_fragment(data.superstr,
                           data.fragments[INITIAL_FRAGMENT_INDEX].content);
    operation_t best;
    int i;
    /* Iterate and append fragments */
    for (i = 1; i < data.frg_count; i++){
        find_best_operation(p2d, &best, END_ONLY);

        /* Modify the superstring according to the operation
         * Then, print output */
        modify_superstr(p2d, &best);
        print_output(i, best.src_index, data.superstr, 0);

        /* Inform print_out if the last fragment is reached*/
        if (i==data.frg_count-1){
            print_output(i, best.src_index, data.superstr, 1);
        }
    }
    return 0;
}

int stage3(data_t data){
    print_output_header(3);

    /* Declare a pointer point to local superstring */
    data_t* p2d = &data;

    /* Place the first fragment to initialize a superstring */
    place_initial_fragment(data.superstr,
                           data.fragments[INITIAL_FRAGMENT_INDEX].content);
    operation_t best;

    int i;
    for (i = 1; i < data.frg_count; i++){
        /* Call find_best_operation to choose the
         * fragment the gives the most overlap */
        find_best_operation(p2d, &best, HEAD_END);

        /* Modify the superstring according to the operation
         * Then, print output */
        modify_superstr(p2d, &best);
        print_output(i, best.src_index, data.superstr, 0);

        /* Inform print_out if the last fragment is reached*/
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
    /* Check that the fragment specified have not been processed */
    assert(data_p->fragments[frg_index].status);
    data_p->fragments[frg_index].status = PROCESSED;
}

int main(int argc, char* argv[]){
    /* Driver function for stage 1, 2 and 3. */
    data_t data;

    /* Read fragments and save return value as frg_count */
    data.frg_count = read_str(data.fragments);

    /* Run stage1 */
    stage1(data);

    /* Run stage2 */
    stage2(data);

    /* Run stage3 */
    stage3(data);

    return 0;
}
/* Algorithms are fun */
