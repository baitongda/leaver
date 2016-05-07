/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifndef LEAVER_APPENDER_FILE_H
#define LEAVER_APPENDER_FILE_H

extern int leaver_appender_file_write_log(char *log_file, char *log, size_t log_len);

extern zend_class_entry *leaver_appender_file_ce;

LEAVER_CREATE_FUNCTION(appender_file);

#endif