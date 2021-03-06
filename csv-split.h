#ifndef __CSV_SPLIT_H
#define __CSV_SPLIT_H

#include "csv-buf.h"
#include "queue.h"
#include <getopt.h>
#include "csv.h"

/**
 * Version number
 */
#define CSV_SPLIT_VERSION "0.1.0"

/**
 * Default IO thread count and sane min/max values
 */
#define IO_THREADS_DEFAULT 1
#define IO_THREADS_MIN     1
#define IO_THREADS_MAX     10

/**
 * How much of a backlog to allow in our IO queue
 */
#define BG_QUEUE_MAX 20

/** 
 * The CSV realloc size, we're being aggressive here
 */
#define CSV_BLK_SIZE 1024

/**
 * Initial size of our passthrough buffer, be aggressive
 */
#define BUFFER_SIZE 1024*1000*10

/** 
 * How much data to read at a time
 */
#define READ_BUF_SIZE 32768

/**
 * Environment variable for payload file
 */
#define ENV_PAYLOAD_VAR  "CSV_PAYLOAD_FILE"
#define ENV_ROWCOUNT_VAR "CSV_ROWCOUNT"

// Context we'll need for our split operation
struct csv_context {
    // Our input file and output path
    char in_file[255], out_path[255];

    // The prefix to use when we split
    const char *in_prefix;

	// Trigger command to run when a chunk is done
    char trigger_cmd[255];

    // Should we read from stdin?
    int from_stdin;

    // Which part are we on
    unsigned int on_file;

    // The number of rows, and our current column
    unsigned long row, col;

    // The maximum number of rows per file
    unsigned long max_rows; 

	/**
     * Mark our overflow position here, which lets us specify that
     * we've gone past our maximum row count but may need to in order
     * to keep 'group together' column data together
     */
    size_t opos;

    /**
     * "group together" column, meaning that we will never split rows
     * with the same value for this column apart.  We assume the
     * rows are sorted by this column
     */
    int gcol;

    // Gzip compression level (zero for none)
    int gzip;

    // Simple flag to let us know if we should put a comma
    unsigned int put_comma;

    // The last group column we encountered, so we can detect when it changes
    cbuf gcol_buf;

    // A buffer we're using and re-using to write the CSV output data
    cbuf csv_buf;

    // Our blocking, thread-safe, IO queue
    fqueue io_queue;

    // The number of threads we're using, and storage for them
    unsigned int thread_count;
    pthread_t *io_threads;
    
    // Our CSV parser
    struct csv_parser parser;
};

/**
 * An item with enough information for our IO consumers to write to disk
 */
struct q_flush_item { 
    // The filename where we'll write data
    char out_file[255];

    // Our trigger command
    const char *trigger_cmd;

    // Our row count
    unsigned long row_count;

    // The data we'll be writing
    char *str;

    // The data length
    size_t len;

    // gzip compression level (zero for none)
    int gzip;
};

static const struct option g_long_opts[] = {
    { "group-col", required_argument, NULL, 'g' },
    { "num-rows", required_argument, NULL, 'n' },
    { "io-threads", optional_argument, NULL, 'i'},
    { "stdin", no_argument, NULL, 0 },
    { "trigger", required_argument, NULL, 't'},
    { "version", no_argument, NULL, 'v'},
    { "gzip", optional_argument, NULL, 'z'},
    { 0, 0, 0, 0}
};

#endif
