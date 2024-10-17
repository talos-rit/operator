# Log Module Architecture

## Config
### Magic Value

- LOG_THRESHOLD_CONSOLE_THIS: Placed at the top of a source file, it defines the lowest priority log level printed to the console from that specific source file.
- LOG_THRESHOLD_FILE_THIS: Placed at the top of a source file, it defines the lowest priority log level printed to the log file from that specific source file.

- LOG_USE_LOCATION (boolean): Toggle's printing of where the print occured in the source code
- LOG_USE_COLOR (boolean): Toggle's printing an entry's level in color in the console. Colors are defined by _LOG_level_color.
- LOG_USE_STDERR (boolean): Toggle's printing Errors/Fatal errors into stderr instead of stdout
- LOG_FILE_PATH (string): The path to the log file created during initialization. A value of NULL will not open any file. If the file does not exist, it will be created. Otherwise, the file will be overwritten. Files have 0666 permissions (universal r/w). 
- LOG_MAX_BUFFER (int): The number of buffer's statically allocated to the Log module
- LOG_BUFFER_LEN (int): The max length of a log entry
- LOG_HEADER_LEN (int): The max length of the log header

### Macros

- LOG_LEVEL_ENTRY: This macro is redefined throughout log.h to construct various structures/enums/macros/aliases of defined log levels. Wrapping the name of the log level at the top of the file in this macro allows it to be maintained in all relevant objects.

- LOG_LEVEL_ENTRIES: list of LOG_LEVEL_ENTRY macros, ordered from most sever to least severe. Used to construct a list of LOG_print aliases, severity levels, etc. (Verbose is special in that any level less severe than verbose is treated as verbose. E.G. A level of verbose + 2 will be printed as "VERBOSE:2").

## Structures

- LOG_Buffer: Stores the relevant information needed to print into the log
- LOG_Config: Stores information needed at initialization 
- Log: Holds all the relevant statically allocated structures needed for the operation of the Log module. A single instance of this is stored as a global variable.

## Threading

### Flow
Note: The global instance of the Log struct will be referred to as "the_log".

1. LOG_init is called
    - the LOG_Config struct in the_log is populated
    - the thread locks are initialized
    - the statically allocated buffers in the_log are initalized and added to the the_log's queue of free buffers
    - the log file is opened for writing (created if it didn't exist, overwritted if it did).
    - the_log is set to disabled

2. LOG_start is called
    - the_log is set to enabled, and the thread is started
    - Messages can start to be queued

3. LOG_run operates a loop in it's thread
    - Every loop, while there are buffers in the_log's write queue, those writes will be processed by calling LOG_thread_poll for each buffer in the queue.
    - LOG_thread_poll operates as follows:
        1. A queued LOG_Buffer is popped from the_log's write queue
        2. The LOG_Buffer is passed to LOG_thread_print
            1. The header is formatted:
                1. The timestamp is put into a modified ISO6801 standard (milliseconds included).
                2. The level is padded to a standard length, with optional coloring.
                3. The location is of the LOG_print request is optionally included.
            2. The message body of the lop is appended to the header
            3. If it satisfies the console threshold, it is printed to the console (stdout/stderr depending on config)
            4. If it satisfies the log file threshold, it is printed to the log file
            
        3. The LOG_Buffer is freed (reinitialized and added back to the queue of free LOG_Buffers) 
    - If the write queue is empty, two things could happen:
        - if the_log is set to disabled (by LOG_stop), LOG_run's infinite loop is exited and the thread is terminated gracefully.
        - if the_log is still set to enabled, the thread sleeps for some amount of time before polling the write queue again

4. LOG_print is called somewhere in the process
    - Note: LOG_print has *many* macro aliases to streamline logging
    1. Relevant log header information is provided, including:
        - file name and line number
        - file's console and log-file threshold
        - log level
        - log format and arguments
    2. Checks whether or not the requirements to fulfil the request have been met (if not, it exits with a success):
        - the_log is set to enabled
        - the log meets the threshold of at least one output
    3. A timestamp is first created to accurately record when the log was requested.
    4. A freshly initialized LOG_Buffer is aquired:
    5. The above header information is copied into the aquired buffer
        - The timestamp is copied over
        - The level of the log, and thresholds are copied over
        - The fmt, arguments, and optionaly LOG_print location data are formatted into a string stored in the LOG_Buffer
    6. The buffer is pushed onto the_log's write queue:

5. LOG_stop is called
    - the_log is set to disabled, preventing new logs from queueing.
    - blocks the thread it was called from until all queued buffers are completed and LOG_run gracefully exits its loop.

6. LOG_destroy is called
    - the_log is deinitialized
    - resources are released (e.g. log file is closed)

