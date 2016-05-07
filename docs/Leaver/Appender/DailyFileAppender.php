<?php

namespace Leaver\Appender;

use Leaver\Appender;
use Throwable;

class DailyFileAppender extends Appender
{
    /**
     * The path of log path.
     *
     * @var string
     */
    protected $filePath;

    /**
     * Sets path to log file.
     *
     * @param string $filePath
     */
    public function setFilePath(string $filePath) : void {}

    /**
     * Appends a log to this appender.
     *
     * @param int $level
     * @param string $message
     * @param Throwable $throwable
     */
    public function onAppend(int level, string $message, Throwable $throwable = null) : void {}
}
