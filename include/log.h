/**
  * @file log.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Provide logging function that will be called from all module in the program
  */
#ifndef __HFM_LOG_H__
#define __HFM_LOG_H__


/**
  * @brief Log levels definition
  */
typedef enum {
    LV_FATAL,
    LV_ERROR,
    LV_WARN,
    LV_INFO,
    LV_DEBUG
} level_t;

/**
  * @brief Define logger output types
  *
  * Currently support two types of output: log to console and log to textfile
  */
typedef enum {
    LOG_CONSOLE,
    LOG_TEXTFILE
} logger_t;

/**
  * @brief Init logger
  * @param level Log level
  * @param logger  Logger output type
  *
  * If the logger_t is TEXTFILE, add another param for filename
  */
void log_init(level_t level, logger_t logger, ...);


/**
  * @brief Close logger, just need when logger is textfile type
  */
void log_close(void);

/**
  * @brief Function for writing log
  * @param level Log level
  * @param message String message or string message format
  */
void writelog(level_t level, const char *message, ...);

#endif
