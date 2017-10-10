#ifndef __FCLEANER_FILEUTIL_H__
#define __FCLEANER_FILEUTIL_H__

/**
  * @brief Check two file dupplicated or not
  * @param file1 File 1
  * @param file2 File 2
  * @return 1 if dupplicate, or else return 0
  */
int util_checkdup(const char *file1, const char *file2);

/**
  * @brief Create symbolic Link file
  * @param filename Filename
  * @return 1 if create symbolick link success
  */
int util_create_symbol_link(const char *filename);

#endif
