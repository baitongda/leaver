/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <php.h>

#include "../../php_leaver.h"
#include "../appender.h"
#include "file.h"

extern void leaver_appender_file_write_log(char *log_file, char *log, size_t log_len);
extern char *leaver_appender_file_format_path(char *log_file, size_t log_file_len);
extern int leaver_appender_file_check_dir(char *log_dir);

zend_class_entry *leaver_appender_file_ce;

// public \Leaver\Appender\FileAppender::setFilePath(string $filePath) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_file_setFilePath_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, filePath, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_file, setFilePath)
{
    zval *this = getThis();
    zend_string *filePath;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filePath)) {
        return;
    }

    zend_update_property_str(leaver_appender_file_ce, this, ZEND_STRL("filePath"), filePath);
}

// public \Leaver\Appender\FileAppender::setUsePattern(bool $usePattern = true) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_file_setUsePattern_arginfo, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, usePattern, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_file, setUsePattern)
{
    zval *this = getThis();
    zend_bool use_pattern;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &use_pattern)) {
        return;
    }

    zend_update_property_bool(leaver_appender_file_ce, this, ZEND_STRL("usePattern"), use_pattern);
}

// public \Leaver\Appender\FileAppender::onAppend(int level, string $message, Throwable $throwable = NULL) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_file_onAppend_arginfo, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_file, onAppend)
{
    zval *this = getThis();
    zend_long level;
    zend_string *message;
    zval *z_exception = NULL;
    zval *z_format, *z_file_path, *z_use_pattern;
    zend_string *format, *file_path;
    zend_bool use_pattern;
    char *log, *log_file;
    size_t log_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "lS|z", &level, &message, &z_exception)) {
        return;
    }

    z_format = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("format"), 1, NULL);
    z_file_path = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("filePath"), 1, NULL);
    z_use_pattern = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("usePattern"), 1, NULL);

    format = zval_get_string(z_format);
    file_path = zval_get_string(z_file_path);
    use_pattern = Z_TYPE_P(z_use_pattern) == IS_TRUE ? (char) 1 : (char) 0;

    if (use_pattern) {
        log_file = leaver_appender_file_format_path(file_path->val, file_path->len);
    } else {
        log_file = estrdup(file_path->val);
    }

    log_len = leaver_appender_common_format_log(&log, format, level, message);

    leaver_appender_file_write_log(log_file, log, log_len);

    zend_string_release(format);
    zend_string_release(file_path);
    efree(log);
    efree(log_file);
}

zend_function_entry leaver_appender_file_methods[] = {
    PHP_ME(leaver_appender_file, setFilePath, leaver_appender_file_setFilePath_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender_file, setUsePattern, leaver_appender_file_setUsePattern_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender_file, onAppend, leaver_appender_file_onAppend_arginfo, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(appender_file)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Appender\\FileAppender", leaver_appender_file_methods);
    leaver_appender_file_ce = zend_register_internal_class_ex(&ce, leaver_appender_ce);

    zend_declare_property_null(leaver_appender_file_ce, ZEND_STRL("filePath"), ZEND_ACC_PROTECTED);
    zend_declare_property_bool(leaver_appender_file_ce, ZEND_STRL("usePattern"), 1, ZEND_ACC_PROTECTED);

    return SUCCESS;
}

void leaver_appender_file_write_log(char *log_file, char *log, size_t log_len)
{
    char *log_dir;
    php_stream *stream;

    log_dir = estrdup(log_file);
    zend_dirname(log_dir, strlen(log_dir));

    if (FAILURE == leaver_appender_file_check_dir(log_dir)) {
        efree(log_dir);
        return;
    }

    efree(log_dir);

    stream = php_stream_open_wrapper(log_file, "a", IGNORE_URL, NULL);
    if (!stream) {
        return;
    }

    php_stream_write(stream, log, log_len);
    php_stream_write(stream, "\n", 1);
    php_stream_close(stream);
    php_stream_free(stream, PHP_STREAM_FREE_RELEASE_STREAM);
}

// Usable tokens:
//  %% for '%'; %Y year; %YYY year; %M month; %D day; %H hour; %I minute;
char *leaver_appender_file_format_path(char *log_file, size_t log_file_len)
{
    char *p, *buf, *b;
    size_t buf_len, add_len;
    char token;

    time_t timestamp;
    struct tm * timeinfo;

    timestamp = time(NULL);
    timeinfo = localtime(&timestamp);

    p = log_file;
    buf = emalloc(log_file_len + 1);
    b = buf;
    buf_len = 0;

    while ('\0' != *p) {
        if (!token && '%' == *p) {
            token = 1;
            p++;
            continue;
        }

        if (token) {
            token = 0;

            switch (*p) {
                case '%':
                    *b = '%';
                    add_len = 1;
                    break;

                case 'Y':
                    add_len = snprintf(b, 3, "%2d", (timeinfo->tm_year + 1900) % 100);
                    break;

                case 'M':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mon + 1);
                    break;

                case 'D':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mday);
                    break;

                case 'H':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_hour);
                    break;

                case 'I':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_min);
                    break;

                default:
                    break;
            }

                buf_len += add_len;
            b += add_len;
            p++;
            continue;
        }

        *b = *p;
        buf_len++;
        b++;
        p++;
    }

    buf[buf_len] = '\0';

    return buf;
}

int leaver_appender_file_check_dir(char *log_dir)
{
    if (access(log_dir, 0)) {
        mode_t mask = umask(0);

        if (!php_stream_mkdir(log_dir, 0777, PHP_STREAM_MKDIR_RECURSIVE, NULL)) {
            umask(mask);
            return FAILURE;
        }

        umask(mask);
    }

    return SUCCESS;
}