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
  * @param filename Csv file pathc
  */
void out_csv_init(const char *filename);

/**
  * @brief Write using csv output
  *
  * @param info Output info
  */
void out_csv_write(output_info_t *info);

#endif
