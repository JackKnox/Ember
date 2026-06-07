#include "ember/core.h"

const char* em_result_string(em_result result,  b8 get_extended) {
    switch (result) {
    case EMBER_RESULT_OK:                 
        return !get_extended ? "EMBER_RESULT_OK" : "EMBER_RESULT_OK Operation completed successfully.";
    case EMBER_RESULT_TIMEOUT:            
        return !get_extended ? "EMBER_RESULT_TIMEOUT" : "EMBER_RESULT_TIMEOUT Operation timed out before completion.";
    case EMBER_RESULT_UNINITIALIZED:      
        return !get_extended ? "EMBER_RESULT_UNINITIALIZED" : "EMBER_RESULT_UNINITIALIZED The system, device, or resource was not initialized.";
    case EMBER_RESULT_INVALID_ENUM:       
        return !get_extended ? "EMBER_RESULT_INVALID_ENUM" : "EMBER_RESULT_INVALID_ENUM An invalid enum value was provided.";
    case EMBER_RESULT_INVALID_VALUE:      
        return !get_extended ? "EMBER_RESULT_INVALID_VALUE" : "EMBER_RESULT_INVALID_VALUE An invalid value was provided (out of expected range).";
    case EMBER_RESULT_UNSUPPORTED_FORMAT: 
        return !get_extended ? "EMBER_RESULT_UNSUPPORTED_FORMAT:" : "EMBER_RESULT_UNSUPPORTED_FORMAT The requested format or type is not supported.";
    case EMBER_RESULT_OUT_OF_MEMORY_CPU:  
        return !get_extended ? "EMBER_RESULT_OUT_OF_MEMORY_CPU" : "EMBER_RESULT_OUT_OF_MEMORY_CPU CPU memory allocation failed.";
    case EMBER_RESULT_OUT_OF_MEMORY_GPU:  
        return !get_extended ? "EMBER_RESULT_OUT_OF_MEMORY_GPU" : "EMBER_RESULT_OUT_OF_MEMORY_GPU GPU memory allocation failed.";
    case EMBER_RESULT_UNAVAILABLE_API:    
        return !get_extended ? "EMBER_RESULT_UNAVAILABLE_API" : "EMBER_RESULT_UNAVAILABLE_API The requested API is not available on this device.";
    case EMBER_RESULT_UNIMPLEMENTED:      
        return !get_extended ? "EMBER_RESULT_UNIMPLEMENTED" : "EMBER_RESULT_UNIMPLEMENTED The requested feature or function is not implemented.";
    case EMBER_RESULT_VALIDATION_FAILED:  
        return !get_extended ? "EMBER_RESULT_VALIDATION_FAILED" : "EMBER_RESULT_VALIDATION_FAILED Input or operation validation failed.";
    case EMBER_RESULT_IN_USE:             
        return !get_extended ? "EMBER_RESULT_IN_USE" : "EMBER_RESULT_IN_USE The resource is currently in use and cannot be accessed.";
    case EMBER_RESULT_PERMISSION_DENIED:
        return !get_extended ? "EMBER_RESULT_PERMISSION_DENIED" : "EMBER_RESULT_PERMISSION_DENIED The caller does not have the required permissions.";

    default:
#if !EMBER_DIST
        EM_ASSERT(FALSE && "Unknown em_result value!");
        return NULL;
#endif
    case EMBER_RESULT_UNKNOWN:
        return !get_extended ? "EMBER_RESULT_UNKNOWN" : "EMBER_RESULT_UNKNOWN An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
    }
}