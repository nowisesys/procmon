# Process monitor for GNU/Linux (procmon)

This application scans the table of running processes, killing those that has 
exceeds a given CPU-time limit or has gone for lunch. Filtering of processes is 
optionally done on command name, matching absolute or fuzzy.

The procmon application is etiher runned periodical in single-shot mode from 
command line or crontab, or runned as procmond forked into the background 
as a daemon process.

### Requirements
The procps package should be installed (with development headers and libs) before 
trying to compile this application source code.

### Daemon mode
The process can be controlled by sending signals when running as daemon. Sending 
SIGKILL or SIGTERM will ask the daemon to exit. Sending SIGHUP will force the 
process to immediate begin a scanning of running processes.

### Options
These are some of the options supported by procmon (dump from version 0.8.4):

```bash
Usage: procmon [options...]
Options:
  -c,--command=name: Name of command to monitor.
  -n,--limit=sec:    Max execution time limit (3600 sec).
  -b,--daemon:       Fork to background running as daemon.
  -x,--script=path:  Execute script when signal process.
  -s,--signal=num:   Send signal to processes (15).
  -i,--interval=sec: Poll interval (60 sec).
  -f,--foreground:   Don't detach from controlling terminal.
  -z,--fuzzy:        Enable fuzzy match of command name.
     ...
  -m,--dry-run:      Don't kill processes, only monitor and report.
     ...
```

The process can drop privileges (permanent or temporary) between scannings. 
These options are not showed above in the options excerpt, but should be familiar 
to everyone.

