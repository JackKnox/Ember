#pragma once

#include "ember/core.h"

/**
 * @brief Result codes returned by Ember functions.
 * 
 * Indicates the outcome of an operation. Functions typically return
 * `EMBER_RESULT_OK` on success or another value indicating the type
 * of failure or status.
 */
typedef enum em_result {
    EMBER_RESULT_OK = 0,             /**< Operation completed successfully */
    EMBER_RESULT_TIMEOUT,            /**< Operation timed out before completion */
    EMBER_RESULT_UNINITIALIZED,      /**< The system, device, or resource was not initialized */
    EMBER_RESULT_INVALID_ENUM,       /**< An invalid enum value was provided */
    EMBER_RESULT_INVALID_VALUE,      /**< An invalid value was provided (out of expected range) */
    EMBER_RESULT_UNSUPPORTED_FORMAT, /**< The requested format or type is not supported */
    EMBER_RESULT_OUT_OF_MEMORY_CPU,  /**< CPU memory allocation failed */
    EMBER_RESULT_OUT_OF_MEMORY_GPU,  /**< GPU memory allocation failed */
    EMBER_RESULT_UNAVAILABLE_API,    /**< The requested API is not available on this device */
    EMBER_RESULT_UNIMPLEMENTED,      /**< The requested feature or function is not implemented */
    EMBER_RESULT_VALIDATION_FAILED,  /**< Input or operation validation failed */
    EMBER_RESULT_IN_USE,             /**< The resource is currently in use and cannot be accessed */
    EMBER_RESULT_UNKNOWN             /**< An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred */
} em_result;

const char* em_result_string(em_result result, b8 get_extended);