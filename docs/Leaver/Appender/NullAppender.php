<?php

namespace Leaver\Appender;

use Leaver\Appender;
use Throwable;

class NullAppender extends Appender
{
    /**
     * Appends a log to this appender.
     *
     * @param int $level
     * @param string $message
     * @param Throwable $throwable
     */
    public function onAppend(int level, string $message, Throwable $throwable = null) : void {}
}
