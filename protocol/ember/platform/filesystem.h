#pragma once

#include "ember/core.h"

/**
 * @brief File access flags.
 */
typedef enum emplat_file_flags {
    EMBER_FILE_FLAGS_READ  = 1 << 0, /**< Open file for reading */
    EMBER_FILE_FLAGS_WRITE = 1 << 1, /**< Open file for writing */
} emplat_file_flags;

/**
 * @brief Platform-specific file handle.
 */
typedef void* emplat_file;

/**
 * @brief Info on file, whetever it exists or not.
 */
typedef struct emplat_file_info {
    /** @brief Whetever the file on disk exists. If false the rest of the structure is uninitalized. */
    b8 exists;

    /** @brief Size of the file on disk (in bytes). */
    u64 size;

    /** @brief The time the file was created, zero if unsupported by driver. */
    u64 created_time;

    /** @brief The last time the file was accessed or changed. */
    u64 modified_time;

    /** @brief The last time the file was accessed. */
    u64 accessed_time;

    /** @brief Indicaties whetever there is a directory at the given filepath. */
    b8 is_directory;
} emplat_file_info;

/**
 * @brief Retrieves useful metadata on a file or directory on disk
 *
 * @param filepath Filepath on disk.
 * @param out_info Pointer to recieving info structure.
 */
void emplat_file_get_info(const char* filepath, emplat_file_info* out_info);

/**
 * @brief Opens a file with the given access flags.
 *
 * @param filepath Filepath on disk.
 * @param flags Access mode flags (read/write).
 * @param out_file Receives the opened file handle.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_file_open(const char* filepath, emplat_file_flags flags, emplat_file* out_file);

/**
 * @brief Closes an open file handle.
 *
 * @param file File handle to close.
 */
void emplat_file_close(emplat_file* file);

/**
 * @brief Gets the size of a file in bytes.
 *
 * @param file File handle.
 * @return Size of the file in bytes.
 */
u64 emplat_file_size(emplat_file* file);

/**
 * @brief Reads data from a file.
 *
 * @param file File handle.
 * @param size Number of bytes to read.
 * @param out_data Output buffer to store read data.
 * @param out_data_size Actual number of bytes read.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 * 
 * @note If @ref out_data_size does not equal requested size but succeded, 
 *       assume you reached the end of the file.
 */
em_result emplat_file_read(emplat_file* file, u64 size, void* out_data, u64* out_data_size);

/**
 * @brief Writes data to a file.
 *
 * @param file File handle.
 * @param size Number of bytes to write.
 * @param data Input buffer containing data to write.
 * @param out_data_size Actual number of bytes written.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_file_write(emplat_file* file, u64 size, const void* data, u64* out_data_size);

/**
 * @brief Locks a file from use by other threads.
 * 
 * @param file File handle.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_file_lock(emplat_file* file);

/**
 * @brief Unlocks a file for use by other threads.
 * 
 * @param file File handle.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_file_unlock(emplat_file* file);

/**
 * @brief Immediatly writes all data to a filepath through atomic operations.
 * 
 * This is very useful for crash reports or hot-reloading as it
 * ensures no other process is writing to the file and prevents corrupted file writes.
 * 
 * @param filepath Filepath on disk.
 * @param data Source data.
 * @param size Source data size (in bytes).
 * @note Use the function sparingly as it may impact performance.
 */
em_result emplat_file_write_safe(const char* filepath, const void* data, u64 size);

/**
 * @brief File system event types.
 */
typedef enum emplat_filewatch_event_type {
    EMBER_FILEWATCH_CREATED,     /**< When a file is newly created */
    EMBER_FILEWATCH_MODIFIED,    /**< When a existing files contents is modified */ 
    EMBER_FILEWATCH_DELETED,     /**< When a file is deleted from the filesystem, also possibly moved to trash folder. */
    EMBER_FILEWATCH_RENAMED_OLD, /**< Old name of a file that has been renamed */
    EMBER_FILEWATCH_RENAMED_NEW, /**< New name of a file that has been renamed*/
} emplat_filewatch_event_type;

/**
 * @brief File system event description.
 */
typedef struct emplat_filewatch_event {
    /** Path of the file affected by the event */
    const char* filepath;

    /** Type of file system event */
    emplat_filewatch_event_type type;
} emplat_filewatch_event;

/**
 * @brief Platform-specific file watcher handle.
 */
typedef void* emplat_filewatcher;

/**
 * @brief Callback invoked when a file system event occurs.
 *
 * @param filewatcher File watcher instance generating the event.
 * @param user_data User-defined pointer provided at creation time.
 * @param event File system event data.
 */
typedef void (*PFN_on_filewatch)(emplat_filewatcher* filewatcher, void* user_data, emplat_filewatch_event event);

/**
 * @brief Creates a file watcher instance.
 *
 * @param callback Function called when file events occur.
 * @param out_filewatcher Receives created watcher instance.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_filewatcher_create(PFN_on_filewatch callback, emplat_filewatcher* out_filewatcher);

/**
 * @brief Destroys a file watcher instance.
 *
 * @param filewatcher Watcher to destroy.
 */
void emplat_filewatcher_destroy(emplat_filewatcher* filewatcher);

/**
 * @brief Adds a file or directory to be watched.
 *
 * @param filewatcher Watcher instance.
 * @param filepath Filepath on disk.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_filewatcher_add(emplat_filewatcher* filewatcher, const char* filepath);

/**
 * @brief Adds a wildcard pattern to watch.
 *
 * @param filewatcher Watcher instance.
 * @param pattern Pattern string (e.g. "*.png").
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_filewatcher_add_pattern(emplat_filewatcher* filewatcher, const char* pattern);

/**
 * @brief Polls pending file system events.
 *
 * @param filewatcher Watcher instance.
 * @param event_count Maximum number of events to retrieve.
 * @param out_events Output array for events.
 * @return Ember result code; returns `EMBER_RESULT_OK` if succeeds.
 */
em_result emplat_filewatcher_poll(emplat_filewatcher* filewatcher, u32 event_count, emplat_filewatch_event* out_events);