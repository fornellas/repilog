repilog
=======

Reopenable Pipped Logging is a very simple program to facilitate Apache log rotation.

Motivation
----------

Apache has two ways of managing log rotation:

* Rename, graceful and gzip (http://httpd.apache.org/docs/2.2/logs.html#rotation).
* Piped logs, and let other tool handle log rotation (http://httpd.apache.org/docs/2.2/logs.html#piped).

First method is by far the most common, but it must call graceful restart (http://httpd.apache.org/docs/2.2/stopping.html#graceful). This will make Apache restart all children processes, and can be an issue on very busy sites. The second method, does not have this issue, but is as good as the external tool is.

Apache provides rotatelogs (http://httpd.apache.org/docs/2.2/programs/rotatelogs.html) tool for that. It works but has 2 disadvantages:

* Rotation period is configured at Apache's httpd.conf, thus, Apache must be disturbed in order to alter rotation periodicity.
* It always use a different suffix for each file name, which can be very tedious to the system administrator.

Apache also recommends Cronolog (http://www.cronolog.org/), but the project have not been updated in a while, and it also has the above limitations. 

The goal here, is to provide the rename, graceful and gzip rotation scheme, but instead of a graceful, only reopen the logs.

How to use it
-------------

repilog can be used in the same fashion as rotatelogs or cronolog along with Apache. Main differences are:

* It will always use the same file name for logging.
* It will close(2) and open(2) the log file upon receiving SIGUSR1.

You must add to your httpd.conf, a line similar to:

CustomLog  "|/opt/repilog/bin/repilog /var/opt/httpd22/log/access.log" default

Rotation can be done with:

* Rename log file.
* Send SIGUSR1 to repilog.
* Wait for the new file (with original name) to be created.
* gzip old log file.

The command below, can be used to find child repilog process from Apache, and send SIGUSR1 only to them:

`/usr/bin/pkill -USR1 -g "$(cat /var/opt/httpd22/run/httpd22.pid)" repilog`

