/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifndef PHP_LEAVER_H
#define PHP_LEAVER_H

#ifdef ZTS
#include <TSRM.h>
#endif

#define LEAVER_NAME             "Leaver"
#define LEAVER_DESCRIPTION      "A fast logger for PHP"
#define LEAVER_VERSION          "0.1.0"
#define LEAVER_VERSION_INT      100
#define LEAVER_AUTHOR           "You Ming"
#define LEAVER_HOMEPAGE         "http://leaver.funcuter.org"

#define LEAVER_API          ZEND_API

#define LEAVER_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(leaver, v)

#define LEAVER_CREATE_FUNCTION(module)      PHP_MINIT_FUNCTION(leaver_##module)
#define LEAVER_START_FUNCTION(module)       PHP_RINIT_FUNCTION(leaver_##module)
#define LEAVER_STOP_FUNCTION(module)        PHP_RSHUTDOWN_FUNCTION(leaver_##module)
#define LEAVER_DESTROY_FUNCTION(module)     PHP_MSHUTDOWN_FUNCTION(leaver_##module)

#define LEAVER_CREATE(module)               PHP_MINIT(leaver_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define LEAVER_START(module)                PHP_RINIT(leaver_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define LEAVER_STOP(module)                 PHP_RSHUTDOWN(leaver_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define LEAVER_DESTROY(module)              PHP_MSHUTDOWN(leaver_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

ZEND_BEGIN_MODULE_GLOBALS(leaver)
    struct timeval request_time;
ZEND_END_MODULE_GLOBALS(leaver)

extern ZEND_DECLARE_MODULE_GLOBALS(leaver);

PHP_GINIT_FUNCTION(leaver);
PHP_MINFO_FUNCTION(leaver);
PHP_RINIT_FUNCTION(leaver);
PHP_RSHUTDOWN_FUNCTION(leaver);
PHP_MSHUTDOWN_FUNCTION(leaver);
PHP_GSHUTDOWN_FUNCTION(leaver);
PHP_MINFO_FUNCTION(leaver);

extern zend_module_entry leaver_module_entry;
#define phpext_leaver_ptr &leaver_module_entry

#define LEAVER_LEVEL_ALL                "ALL"
#define LEAVER_LEVEL_DEBUG              "DEBUG"
#define LEAVER_LEVEL_INFO               "INFO"
#define LEAVER_LEVEL_NOTICE             "NOTICE"
#define LEAVER_LEVEL_WARNING            "WARNING"
#define LEAVER_LEVEL_ERROR              "ERROR"
#define LEAVER_LEVEL_CRITICAL           "CRITICAL"
#define LEAVER_LEVEL_ALERT              "ALERT"
#define LEAVER_LEVEL_EMERGENCY          "EMERGENCY"
#define LEAVER_LEVEL_OFF                "OFF"

#define LEAVER_LEVEL_ALL_INT            255
#define LEAVER_LEVEL_DEBUG_INT          128
#define LEAVER_LEVEL_INFO_INT           64
#define LEAVER_LEVEL_NOTICE_INT         32
#define LEAVER_LEVEL_WARNING_INT        16
#define LEAVER_LEVEL_ERROR_INT          8
#define LEAVER_LEVEL_CRITICAL_INT       4
#define LEAVER_LEVEL_ALERT_INT          2
#define LEAVER_LEVEL_EMERGENCY_INT      1
#define LEAVER_LEVEL_OFF_INT            0

#define LEAVER_DEFAULT_APPENDER_FORMAT              "%Y/%M/%D %H:%I:%S.%s [%p] %l: %m"
#define LEAVER_DEFAULT_APPENDER_EXCEPTION_FORMAT    "%e : %m%n%f Line %l%n%t"

// Call method.
extern int leaver_call_method(zend_class_entry *obj_ce, zval *obj, const char *func_name, size_t func_name_len, uint32_t params_count, zval* params, zval *retval, zend_function **func);

#define leaver_call_method_with_params(obj, func_name, params_count, params, retval, func) \
    leaver_call_method(Z_OBJCE_P(obj), obj, func_name, sizeof(func_name) - 1, params_count, params, retval, func)

#define leaver_call_method_without_params(obj, func_name, retval, func) \
    leaver_call_method(Z_OBJCE_P(obj), obj, func_name, sizeof(func_name) - 1, 0, NULL, retval, func)

#define leaver_call_method_with_scope(obj_ce, obj, func_name, params_count, params, retval, func) \
    leaver_call_method(obj_ce, obj, func_name, sizeof(func_name) - 1, params_count, params, retval, func)

#endif