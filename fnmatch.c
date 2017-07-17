#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fnmatch.h"

#define MAX_PATH_LEN        256
#define MAX_DIR_LEVEL       13

#define MATCH 0
#define NO_MATCH 1
#define ERROR -1

#define NODE_LEN(list) (list->offsets[list->cur_node + 1] - list->offsets[list->cur_node])
#define NODE_ADDR(list) (list->str + list->offsets[list->cur_node])
#define NODE_TYPE(list) (list->nodetypes[list->cur_node])
#define NODE_RESET(list) list->cur_node = 0
#define NODE_NEXT(list) list->cur_node++        //Move to next node in the list
#define NODE_BACK(list) list->cur_node--;
#define NODE_VISITTED_ALL(list) (list->cur_node >= list->node_cnt)

typedef enum {
    NODE_INVALID,       //Invalid file/directory name or empty
    NODE_CHARACTER,     //Normal file/directory name, example root, home, ...
    NODE_STAR,          //Character *, for example /home/meo/*/
    NODE_DOUBLE_STAR,   //Double start **, represent directory recursive, for example /home/meo/**/file.txt
    NODE_MIXED_STAR     //Filename or directory name with wildcards, for example *.txt, test00*.txt ...
} node_t;

struct _nodes {
    uint8_t node_cnt;                       //a pattern is splitted into nodes by '/' or '\'
    uint8_t cur_node;                       //current node index
    uint8_t offsets[MAX_DIR_LEVEL];         //this array store the offset of each node in the str
    node_t  nodetypes[MAX_DIR_LEVEL];       //type of node
    char str[MAX_PATH_LEN];                 //Raw string
};

/**
  * @brief Matching a filepath nodelist against a pattern nodelist
  * @return 1 if matched, 0 if not matched
  */
int _match(nodelist_t *pattern, nodelist_t *expression);

/**
  * @brief Matching current node of a nodelist against the current node (at pidx position)
  * @return 1 if match, 0 if not match
  */
int _match_cur_node(nodelist_t *pattern, nodelist_t *expression);

/**
  * @brief Find item type of a directory/file name
  * @param str Full filepath path
  * @param start : first index of the item
  * @param end : last index of the item
  */
node_t _find_node_type(const char *str, uint8_t start, uint8_t end);

int fn_match(nodelist_t *pattern, nodelist_t *expression)
{
    NODE_RESET(pattern);
    NODE_RESET(expression);
    int match = _match(pattern, expression);
    if (match)
        return MATCH;
    else
        return NO_MATCH;
}

nodelist_t *fn_translate(const char *str, uint8_t is_pattern)
{
    char ch;
    char *destPtr;
    const char *last_slash;
    const char *strPtr = str;
    nodelist_t *pattern = (nodelist_t *)calloc(1, sizeof(nodelist_t));

    if (*strPtr == '/' || *strPtr == '\\') {
        last_slash = strPtr;
        strPtr += 1;
    }
    else {
        last_slash = strPtr - 1;
    }
    destPtr = pattern->str;

    int cnt = 0;
    while (*strPtr) {
        ch = *strPtr;
        if (ch == '/' || ch == '\\') {      //Meet the slash
            if (strPtr - last_slash == 1) {
                goto error;
            }
            else {
                last_slash = strPtr;
            }
        }
        else if (strPtr - last_slash == 1) {   //Meet first character of one item
            pattern->offsets[pattern->node_cnt] = destPtr - pattern->str;
            *destPtr++ = ch;
            pattern->node_cnt++;
        }   //In the middle of one item
        else {
            *destPtr++ = ch;
        }
        strPtr++;
    }
    //Mark end of pattern
    pattern->offsets[pattern->node_cnt] = destPtr - pattern->str;

    if (is_pattern) {
        //Identify item types
        int i, j;
        for (i = 0; i < pattern->node_cnt; i++) {
            pattern->nodetypes[i] = _find_node_type(pattern->str, pattern->offsets[i], pattern->offsets[i + 1]);
        }
    }
    goto done;
error:
    free(pattern);
    pattern = NULL;
done:
    return pattern;
}

node_t _find_node_type(const char *str, uint8_t start, uint8_t end)
{
    int len = end - start;
    if (len == 1 && str[start] == '*') {
        return NODE_STAR;
    }
    else if (len == 2 && str[start] == '*' && str[start + 1] == '*') {
        return NODE_DOUBLE_STAR;
    }
    else {
        int i;
        for (i = start; i < end; i++) {
            if (str[i] == '*') {
                return NODE_MIXED_STAR;
            }
        }
    }
    return NODE_CHARACTER;
}

int _match(nodelist_t *pattern, nodelist_t *exp)
{
    int match = 1;
TailRecursive:
    if (NODE_VISITTED_ALL(exp) && NODE_VISITTED_ALL(pattern))
        return 1;
    if (NODE_VISITTED_ALL(exp) && !NODE_VISITTED_ALL(pattern))
        return 0;
    if (!NODE_VISITTED_ALL(exp) && NODE_VISITTED_ALL(pattern))
        return 0;
    if (NODE_TYPE(pattern) == NODE_DOUBLE_STAR) {
        NODE_NEXT(pattern);
        NODE_NEXT(exp);
        match = _match(pattern, exp);
        if (match) return match;
        NODE_BACK(pattern);
        goto TailRecursive;
    }
    else {
        if (!_match_cur_node(pattern, exp))
            return 0;
        NODE_NEXT(pattern);
        NODE_NEXT(exp);
        goto TailRecursive;
    }
}

int _match_cur_node(nodelist_t *pattern, nodelist_t *exp)
{
    node_t type = NODE_TYPE(pattern);
    if (type == NODE_CHARACTER) {
        int plen = NODE_LEN(pattern);
        int flen = NODE_LEN(exp);
        if (plen == flen) {
            int cmp = strncmp(NODE_ADDR(pattern), NODE_ADDR(exp), plen);
            if (!cmp)
                return 1;
        }
    }
    else if (type == NODE_STAR) {
        return 1;
    }
    else if (type == NODE_MIXED_STAR) {
        //TODO: For simplicity, currently just support 3  form of mixed star : abc*, *abc, a*b
        char *start = pattern->str + pattern->offsets[pattern->cur_node];
        char *end = pattern->str + (pattern->offsets[pattern->cur_node + 1] -1);
        char *pptr, *fptr;
        int idx;
        if (*start == '*') {
            fptr = exp->str + (exp->offsets[exp->cur_node + 1] - 1);
            for (pptr = end; pptr > start; pptr--, fptr--) {
                if (*pptr != *fptr)
                    return 0;
            }
        }
        else if (*end == '*') {
            fptr = exp->str + exp->offsets[exp->cur_node];
            for (pptr = start; pptr < end; pptr++, fptr++) {
                if (*pptr != *fptr)
                    return 0;
            }
        }
        else {
            pptr = pattern->str + pattern->offsets[pattern->cur_node];
            fptr = exp->str + exp->offsets[exp->cur_node];
            while (*pptr != '*') {
                if (*pptr != *fptr)
                    return 0;
                pptr++;
                fptr++;
            }
            pptr = pattern->str + (pattern->offsets[pattern->cur_node + 1] - 1);
            fptr = exp->str + (exp->offsets[exp->cur_node + 1] - 1);
            while (*pptr != '*') {
                if (*pptr != *fptr)
                    return 0;
                pptr--;
                fptr--;
            }
        }
        return 1;
    }
    return 0;
}
