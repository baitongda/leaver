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
#include "null.h"

zend_class_entry *leaver_appender_null_ce;

// public \Leaver\Appender\NullAppender::onAppend(int level, string $message, Throwable $throwable = NULL) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_null_onAppend_arginfo, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_null, onAppend)
{
    // Do nothing.
}

zend_function_entry leaver_appender_null_methods[] = {
    PHP_ME(leaver_appender_null, onAppend, leaver_appender_null_onAppend_arginfo, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(appender_null)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Appender\\NullAppender", leaver_appender_null_methods);
    leaver_appender_null_ce = zend_register_internal_class_ex(&ce, leaver_appender_ce);

    return SUCCESS;
}