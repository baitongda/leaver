<?php

namespace Leaver\Appender;

use Leaver\Appender;
use Throwable;

class FileAppender extends Appender
{
    protected $filePath;

    protected $usePattern = true;

    /**
     * Sets path to log file.
     *
     * @param string $filePath
     */
    public function setFilePath(string $filePath) : void {}

    /**
     * Sets wheater to use pattern replacing in log path.
     *
     * @param bool $usePattern
     */
    public function setUsePattern(bool $usePattern = true) : void {}

    /**
     * Appends a log to this appender.
     *
     * @param int $level
     * @param string $message
     * @param Throwable $throwable
     */
    public function onAppend(int level, string $message, Throwable $throwable = null) : void {}
}
