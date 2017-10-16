#ifndef __FCLEANER_FILEWATCH_H__
#define __FCLEANER_FILEWATCH_H__

typedef struct _filewatch_t filewatch_t;

/**
  * @brief Init filewatch
  * @param path Path to log file
  * @return filewatch_t instance, return NULL if error
  */
filewatch_t *filewatch_init(const char *path);

/**
  * @brief Set callback function when read a line from log file
  * @param fw Filewatch pointer
  * @param on_recv Callback function pointer
  */
void filewatch_set_cb(filewatch_t *fw, void (*on_recv)(const char *line));

/**
  * @brief Start the filewatch
  * @param fw Filewatch pointer
  */
void filewatch_start(filewatch_t *fw);

/**
  * @brief Stop the filewatch
  * @param fw Filewatch pointer
  */
void filewatch_stop(filewatch_t *fw);

/**
  * @brief Destroy filewatch, release resources
  * @param fw Filewatch pointer
  */
void filewatch_destroy(filewatch_t *fw);

#endif
