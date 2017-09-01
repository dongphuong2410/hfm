#ifndef __FNMATCH_H__
#define __FNMATCH_H__

typedef struct _nodes nodelist_t;
/**
  * @brief Matching a filename against a pattern
  * @param pattern pattern
  * @param expression Expresion to match
  * @return Return 0 if match, return 1 if not match, return -1 if error happens
  *
  * Currently, function just support maximum pattern len 256
  * and maximum directory level is 13
  */
int fn_match(nodelist_t *pattern, nodelist_t *expression);

/**
  * @brief Translate a string pattern into nodelist type
  * @param patttern String
  * @param is_pattern Is this pattern or expression
  * @return nodelist_t pointer
  */

nodelist_t *fn_translate(const char *str, uint8_t is_pattern);


#endif
