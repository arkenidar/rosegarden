#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

// Debug configuration settings
// You can modify these values to control debugging output

// Main debug toggle - set to 0 to disable all debugging
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 1
#endif

// Detailed audio debugging - shows every audio buffer operation
#ifndef DEBUG_AUDIO_DETAILED
#define DEBUG_AUDIO_DETAILED 0
#endif

// Performance monitoring - tracks FPS, audio timing, etc.
#ifndef DEBUG_PERFORMANCE
#define DEBUG_PERFORMANCE 1
#endif

// Memory debugging - tracks allocations (future feature)
#ifndef DEBUG_MEMORY
#define DEBUG_MEMORY 0
#endif

// Event debugging - logs all SDL events
#ifndef DEBUG_EVENTS
#define DEBUG_EVENTS 0
#endif

// Color definitions for console output (if supported)
#ifdef __linux__
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#else
#define COLOR_RESET ""
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_YELLOW ""
#define COLOR_BLUE ""
#define COLOR_MAGENTA ""
#define COLOR_CYAN ""
#endif

// Debug levels
#define DEBUG_LEVEL_ERROR 0
#define DEBUG_LEVEL_WARNING 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_VERBOSE 3

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

#endif // DEBUG_CONFIG_H
