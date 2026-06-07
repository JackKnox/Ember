#pragma once

#include "ember/core.h"

/**
 * @brief Type of popup dialog to display.
 *
 * Determines the icon and visual style used by dialog functions.
 */
typedef enum emplat_popup_type {
    EMBER_POPUP_TYPE_INFO,      /**< Informational message. */
    EMBER_POPUP_TYPE_WARNING,   /**< Warning message. */
    EMBER_POPUP_TYPE_ERROR,     /**< Error message. */
    EMBER_POPUP_TYPE_QUESTION,  /**< Question requiring user input. */
} emplat_popup_type;

/**
 * @brief Displays a notification dialog.
 *
 * Shows a modal dialog containing a message and an OK button.
 *
 * @param title Title displayed in the dialog window.
 * @param message Message displayed to the user.
 * @param type Visual style and icon of the dialog.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_notify(
    const char* title,
    const char* message,
    emplat_popup_type type);

/**
 * @brief Displays an option selection dialog.
 *
 * Presents a list of user-defined options and waits for the user
 * to select one.
 *
 * @param title Title displayed in the dialog window.
 * @param message Message displayed to the user.
 * @param options Array of option strings.
 * @param option_count Number of entries in @p options.
 * @param type Visual style and icon of the dialog.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_option(
    const char* title,
    const char* message,
    const char** options,
    u32 option_count,
    emplat_popup_type type);

/**
 * @brief Displays a text input dialog.
 *
 * Prompts the user to enter a string.
 *
 * @param title Title displayed in the dialog window.
 * @param message Message displayed to the user.
 * @param out_text Buffer that receives the entered text.
 * @param max_size Maximum number of characters that can be written
 *                 to @p out_text, including the null terminator.
 * @param hide_text If true, entered characters are hidden (e.g. password input).
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_input(
    const char* title,
    const char* message,
    char** out_text,
    u32 max_size,
    b8 hide_text);

/**
 * @brief Displays a save file dialog.
 *
 * Allows the user to choose a file path for saving.
 *
 * @param title Title displayed in the dialog window.
 * @param patterns Array of file filter patterns.
 * @param pattern_count Number of entries in @p patterns.
 * @param out_text Buffer that receives the selected file path.
 * @param max_size Maximum number of characters that can be written
 *                 to @p out_text, including the null terminator.
 * @param default_path Initial directory or file path shown when opening the dialog.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_save_file(
    const char* title,
    const char* patterns,
    u32 pattern_count,
    char** out_text,
    u32 max_size,
    const char* default_path);

/**
 * @brief Displays an open file dialog.
 *
 * Allows the user to select one or more files.
 *
 * @param title Title displayed in the dialog window.
 * @param pattern_count Number of entries in @p patterns.
 * @param patterns Array of file filter patterns.
 * @param out_text Buffer that receives the selected file path(s).
 * @param max_size Maximum number of characters that can be written
 *                 to @p out_text, including the null terminator.
 * @param default_path Initial directory shown when opening the dialog.
 * @param multiple_files If true, multiple file selection is allowed.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_open_file(
    const char* title,
    u32 pattern_count,
    const char* patterns,
    char** out_text,
    u32 max_size,
    const char* default_path,
    b8 multiple_files);

/**
 * @brief Displays a folder selection dialog.
 *
 * Allows the user to choose a directory.
 *
 * @param title Title displayed in the dialog window.
 * @param out_text Buffer that receives the selected folder path.
 * @param max_size Maximum number of characters that can be written
 *                 to @p out_text, including the null terminator.
 * @param default_path Initial directory shown when opening the dialog.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_open_folder(
    const char* title,
    char** out_text,
    u32 max_size,
    const char* default_path);

/**
 * @brief Displays a colour picker dialog.
 *
 * Allows the user to select a colour value.
 *
 * @param title Title displayed in the dialog window.
 * @param default_colour Initial colour value.
 * @param out_colour Receives the selected colour value.
 *
 * @return Result code indicating success or failure.
 */
em_result emplat_dialog_colour(
    const char* title,
    u32 default_colour,
    u32* out_colour);