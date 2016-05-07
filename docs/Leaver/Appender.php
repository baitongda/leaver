<?php

namespace Leaver;

use Throwable;

abstract class Appender
{
    /**
     * The name of this appender.
     *
     * @var string
     */
    protected $name;

    /**
     * All levels to be logged.
     *
     * @var int
     */
    protected $acceptLevels;

    /**
     * The format for log.
     *
     * @var bool
     */
    protected $format;

    /**
     * The format for exception log.
     *
     * @var bool
     */
    protected $exceptionFormat;

    /**
     * Whether to append exception logs.
     *
     * @var bool
     */
    protected $logException;

    /**
     * Gets the name of this appender.
     *
     * @return string
     */
    public function getName() : mixed {}

    /**
     * Sets the name of this appender.
     *
     * @param string $name
     */
    public function setName(string $name) : void {}

    /**
     * Sets acceptable levels of this appender.
     *
     * @param int $level
     * @param bool $special
     */
    public function setLevel(int $level, bool $special = false) : void {}

    /**
     * Sets the format of this appender.
     *
     * @param string $format
     */
    public function setFormat(string $format) : void {}

    /**
     * Sets the format for exceptions of this appender.
     *
     * @param string $exceptionFormat
     */
    public function setExceptionFormat(string $exceptionFormat) : void {}

    /**
     * Sets whether to append exception logs.
     *
     * @param bool $logException
     */
    public function setLogException(bool $logException) : void {}

    /**
     * Appends a log to this appender.
     *
     * @param int $level
     * @param string $message
     * @param Throwable $throwable
     */
    public function append(int level, string $message, Throwable $throwable = null) : void {}

    /**
     * Appends a log to this appender.
     *
     * @param int $level
     * @param string $message
     * @param Throwable $throwable
     */
    abstract public function onAppend(int level, string $message, Throwable $throwable = null) : void {}
}
