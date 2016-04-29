<?php

namespace Leaver;

class Logger
{
    // Log levels
    const ALL       = 255;
    const DEBUG     = 128;
    const INFO      = 64;
    const NOTICE    = 32;
    const WARNING   = 16
    const ERROR     = 8;
    const CRITICAL  = 4;
    const ALERT     = 2;
    const EMERGENCY = 1;
    const OFF       = 0;

    /**
     * Adds an appender to this logger.
     *
     * @param Appender $appender
     */
    public function addAppender(Appender $appender) : void {}

    /**
     * Detailed debug information.
     *
     * @param string $message
     * @param array $context
     */
    public function debug(string $message, array $context = null) : void {}

    /**
     * Interesting events.
     *
     * @param string $message
     * @param array $context
     */
    public function info(string $message, array $context = null) : void {}

    /**
     * Normal but significant events.
     *
     * @param string $message
     * @param array $context
     */
    public function notice(string $message, array $context = null) : void {}

    /**
     * Exceptional occurrences that are not errors.
     *
     * @param string $message
     * @param array $context
     */
    public function warning(string $message, array $context = null) : void {}

    /**
     * Runtime errors that do not require immediate action but should typically
     * be logged and monitored.
     *
     * @param string $message
     * @param array $context
     */
    public function error(string $message, array $context = null) : void {}

    /**
     * Critical conditions.
     *
     * @param string $message
     * @param array $context
     */
    public function critical(string $message, array $context = null) : void {}

    /**
     * Action must be taken immediately.
     *
     * @param string $message
     * @param array $context
     */
    public function alert(string $message, array $context = null) : void {}

    /**
     * System is unusable.
     *
     * @param string $message
     * @param array $context
     */
    public function emergency(string $message, array $context = null) : void {}

    /**
     * Logs with an arbitrary level.
     *
     * @param int $level
     * @param string $message
     * @param array $context
     */
    public function log(int $level, string $message, array $context = null) : void {}
}
