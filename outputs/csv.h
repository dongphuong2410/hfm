/**
  * @file csv.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Log content to csv file
  */
#ifndef __HFM_CSV_H
#define __HFM_CSV_H

/**
  * @brief Init csv output module
  *
  * @param filepath Csv file pathc
  * @param output Output_t handler
  */
void out_csv_init(output_t *output, const char *filepath);

/**
  * @brief Write using csv output
  *
  * @param info Output info
  */
void out_csv_write(output_t *output, output_info_t *info);

/**
  * @brief Close and release file handler
  *
  */
void out_csv_close(output_t *output);
#endif
