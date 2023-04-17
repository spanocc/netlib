#### 日志不全的两个原因
- async_logging每3秒才会唤醒一次， 最后3秒的日志可能无法打印出来, 可以通过让程序在最后sleep 3s在结束
- logfile每打印1024条日志才会flush，如果程序不是正常退出的，例如通过`CTRL+C` 强制退出，那么不会正常执行fclose的fflush, 调试时可以调低这个值，比如让他每5条就flush一次