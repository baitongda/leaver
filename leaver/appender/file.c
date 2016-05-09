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
#include <ext/standard/file.h>

#include "../../php_leaver.h"
#include "../appender.h"
#include "file.h"

void leaver_appender_file_make_log_dir(char *log_file)
{
    char *log_dir;

    log_dir = estrdup(log_file);
    zend_dirname(log_dir, strlen(log_dir));

    if (access(log_dir, F_OK)) {
        php_stream_context *context;

        context = php_stream_context_from_zval(NULL, 0);
        php_stream_mkdir(log_dir, 0777, PHP_STREAM_MKDIR_RECURSIVE, context);
    }

    efree(log_dir);
}

int leaver_appender_file_write_log(char *log_file, char *log, size_t log_len)
{
    php_stream *stream;

    if (access(log_file, F_OK)) {
        leaver_appender_file_make_log_dir(log_file);
    }

    stream = php_stream_open_wrapper(log_file, "a", IGNORE_URL, NULL);
    if (!stream) {
        return FAILURE;
    }

    php_stream_write(stream, log, log_len);
    php_stream_write(stream, "\n", 1);
    php_stream_free(stream, PHP_STREAM_FREE_CLOSE | PHP_STREAM_FREE_RELEASE_STREAM);

    return SUCCESS;
}

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
    zval *z_format, *z_file_path, *z_log_exception;
    zend_string *format, *file_path;
    zend_bool log_exception;
    char *log;
    size_t log_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "lS|z", &level, &message, &z_exception)) {
        return;
    }

    z_format = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("format"), 1, NULL);
    z_log_exception = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("logException"), 1, NULL);
    z_file_path = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("filePath"), 1, NULL);

    format = zval_get_string(z_format);
    log_exception = Z_TYPE_P(z_log_exception) == IS_TRUE ? (char) 1 : (char) 0;
    file_path = zval_get_string(z_file_path);

    log_len = leaver_appender_format_log(&log, format, level, message);

    if (log_exception && z_exception && Z_TYPE_P(z_exception) == IS_OBJECT) {
        // Add exception log if exist.
        zval *z_exception_format;
        zend_string *exception_format;
        char *exception_log, *log_tmp;
        size_t exception_log_len;

        z_exception_format = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("exceptionFormat"), 1, NULL);
        exception_format = zval_get_string(z_exception_format);

        exception_log_len = leaver_appender_format_exception_log(&exception_log, exception_format, z_exception);

        log_tmp = emalloc(log_len + exception_log_len + 2);

        strncpy(log_tmp, log, log_len);
        log_tmp[log_len] = '\n';
        strncpy(log_tmp + log_len + 1, exception_log, exception_log_len);
        log_tmp[log_len + exception_log_len + 1] = '\0';

        zend_string_release(exception_format);
        efree(exception_log);
        efree(log);

        log_len = log_len + exception_log_len + 1;
        log = log_tmp;
    }

    leaver_appender_file_write_log(file_path->val, log, log_len);

    zend_string_release(format);
    zend_string_release(file_path);
    efree(log);
}

zend_function_entry leaver_appender_file_methods[] = {
    PHP_ME(leaver_appender_file, setFilePath, leaver_appender_file_setFilePath_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender_file, onAppend, leaver_appender_file_onAppend_arginfo, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(appender_file)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Appender\\FileAppender", leaver_appender_file_methods);
    leaver_appender_file_ce = zend_register_internal_class_ex(&ce, leaver_appender_ce);

    zend_declare_property_null(leaver_appender_file_ce, ZEND_STRL("filePath"), ZEND_ACC_PROTECTED);

    return SUCCESS;
}