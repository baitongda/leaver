<?php

namespace Leaver;

use Throwable;

abstract class Appender
{
    protected $format;

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
     * Sets the format of this appender.
     *
     * @param string $format
     */
    public function setFormat(string $format) : void {}

    /**
     * Sets acceptable levels of this appender.
     *
     * @param int $level
     * @param bool $special
     */
    public function setLevel(int $level, bool $special = false) : void {}

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
