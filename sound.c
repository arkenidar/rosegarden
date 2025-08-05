#include <SDL3/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define FREQUENCY_A4 440.0 // Frequency for A4 note (440 Hz)

// Window dimensions
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

// Debug configuration
#define DEBUG_ENABLED 1
#define DEBUG_AUDIO_DETAILED 0 // Set to 1 for verbose audio debugging
#define DEBUG_PERFORMANCE 1    // Set to 1 for performance monitoring

// Debug macros
#if DEBUG_ENABLED
#define DEBUG_LOG(fmt, ...)                                         \
    do                                                              \
    {                                                               \
        time_t now = time(NULL);                                    \
        struct tm *tm = localtime(&now);                            \
        printf("[%02d:%02d:%02d] DEBUG: " fmt "\n",                 \
               tm->tm_hour, tm->tm_min, tm->tm_sec, ##__VA_ARGS__); \
    } while (0)

#define DEBUG_ERROR(fmt, ...)                                        \
    do                                                               \
    {                                                                \
        time_t now = time(NULL);                                     \
        struct tm *tm = localtime(&now);                             \
        fprintf(stderr, "[%02d:%02d:%02d] ERROR: " fmt "\n",         \
                tm->tm_hour, tm->tm_min, tm->tm_sec, ##__VA_ARGS__); \
    } while (0)
#else
#define DEBUG_LOG(fmt, ...)
#define DEBUG_ERROR(fmt, ...)
#endif

// Performance monitoring
typedef struct
{
    Uint64 start_time;
    Uint64 frame_count;
    double total_audio_gen_time;
    double total_render_time;
    int audio_buffer_underruns;
    int audio_samples_generated;
} DebugStats;

SDL_AudioDeviceID audio_device;
SDL_AudioStream *audio_stream;
bool note_on = false;
double phase = 0.0;
double frequency = FREQUENCY_A4;
const char *note_name = "A"; // Current note name

// Debug and monitoring globals
DebugStats debug_stats = {0};
Uint64 last_stats_print = 0;

// Audio device information
typedef struct
{
    SDL_AudioSpec spec;
    char device_name[256];
    bool is_capture;
    int device_id;
} AudioDeviceInfo;

AudioDeviceInfo audio_info = {0};

// Function prototypes for debugging
void print_audio_device_info(void);
void print_performance_stats(void);
void validate_audio_stream(void);
bool check_sdl_error(const char *operation);

// SDL error checking utility
bool check_sdl_error(const char *operation)
{
    const char *error = SDL_GetError();
    if (error && *error)
    {
        DEBUG_ERROR("%s failed: %s", operation, error);
        return true;
    }
    return false;
}

// Print detailed audio device information
void print_audio_device_info(void)
{
    DEBUG_LOG("=== Audio Device Information ===");
    DEBUG_LOG("Device ID: %u", audio_device);
    DEBUG_LOG("Sample Rate: %d Hz", audio_info.spec.freq);
    DEBUG_LOG("Format: %s", SDL_GetAudioFormatName(audio_info.spec.format));
    DEBUG_LOG("Channels: %d", audio_info.spec.channels);

    // Get audio driver info
    const char *driver = SDL_GetCurrentAudioDriver();
    if (driver)
    {
        DEBUG_LOG("Audio Driver: %s", driver);
    }

    // Check audio stream status
    if (audio_stream)
    {
        int queued = SDL_GetAudioStreamQueued(audio_stream);
        int available = SDL_GetAudioStreamAvailable(audio_stream);
        DEBUG_LOG("Stream - Queued: %d bytes, Available: %d bytes", queued, available);
    }
}

// Validate audio stream state
void validate_audio_stream(void)
{
    if (!audio_stream)
    {
        DEBUG_ERROR("Audio stream is NULL!");
        return;
    }

    int queued = SDL_GetAudioStreamQueued(audio_stream);
    if (queued < 0)
    {
        DEBUG_ERROR("Invalid queued audio data: %d", queued);
        check_sdl_error("SDL_GetAudioStreamQueued");
    }

    if (queued == 0 && note_on)
    {
        debug_stats.audio_buffer_underruns++;
        DEBUG_LOG("Audio buffer underrun detected (count: %d)", debug_stats.audio_buffer_underruns);
    }
}

// Print performance statistics
void print_performance_stats(void)
{
    Uint64 current_time = SDL_GetTicks();

    if (current_time - last_stats_print >= 5000)
    { // Every 5 seconds
        double elapsed_seconds = (current_time - debug_stats.start_time) / 1000.0;
        double fps = debug_stats.frame_count / elapsed_seconds;

        DEBUG_LOG("=== Performance Statistics ===");
        DEBUG_LOG("Runtime: %.1f seconds", elapsed_seconds);
        DEBUG_LOG("Frames rendered: %" SDL_PRIu64, debug_stats.frame_count);
        DEBUG_LOG("Average FPS: %.1f", fps);
        DEBUG_LOG("Audio samples generated: %d", debug_stats.audio_samples_generated);
        DEBUG_LOG("Audio buffer underruns: %d", debug_stats.audio_buffer_underruns);

        if (debug_stats.frame_count > 0)
        {
            DEBUG_LOG("Avg audio gen time: %.2f ms/frame",
                      debug_stats.total_audio_gen_time / debug_stats.frame_count);
            DEBUG_LOG("Avg render time: %.2f ms/frame",
                      debug_stats.total_render_time / debug_stats.frame_count);
        }

        last_stats_print = current_time;
    }
}

void render_text(SDL_Renderer *renderer, const char *text, int x, int y)
{
    // Placeholder rendering as a rectangle since SDL has no text rendering by default
    // Suppress unused parameter warning
    (void)text;
    SDL_FRect rect = {x, y, 100, 50};
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

// Function to generate and queue audio samples
void generate_audio_samples(int num_samples)
{
    Uint64 start_time = SDL_GetPerformanceCounter();

#if DEBUG_AUDIO_DETAILED
    DEBUG_LOG("Generating %d audio samples, frequency: %.2f Hz, note: %s",
              num_samples, frequency, note_on ? note_name : "OFF");
#endif

    Sint16 *buffer = malloc(num_samples * sizeof(Sint16));
    if (!buffer)
    {
        DEBUG_ERROR("Failed to allocate audio buffer for %d samples", num_samples);
        return;
    }

    double phase_increment = 2.0 * M_PI * frequency / SAMPLE_RATE;

    for (int i = 0; i < num_samples; i++)
    {
        if (note_on)
        {
            buffer[i] = (Sint16)(AMPLITUDE * sin(phase));
            phase += phase_increment;
            if (phase >= 2.0 * M_PI)
            {
                phase -= 2.0 * M_PI;
            }
        }
        else
        {
            buffer[i] = 0; // Silence when the note is not active
        }
    }

    // Queue audio data
    int result = SDL_PutAudioStreamData(audio_stream, buffer, num_samples * sizeof(Sint16));
    if (result < 0)
    {
        DEBUG_ERROR("Failed to queue audio data: %s", SDL_GetError());
    }
    else
    {
        debug_stats.audio_samples_generated += num_samples;
#if DEBUG_AUDIO_DETAILED
        DEBUG_LOG("Successfully queued %d samples", num_samples);
#endif
    }

    free(buffer);

    // Performance tracking
    Uint64 end_time = SDL_GetPerformanceCounter();
    double elapsed_ms = (double)(end_time - start_time) / SDL_GetPerformanceFrequency() * 1000.0;
    debug_stats.total_audio_gen_time += elapsed_ms;

#if DEBUG_AUDIO_DETAILED
    DEBUG_LOG("Audio generation took %.3f ms", elapsed_ms);
#endif
}

// Cleanup function to ensure all resources are freed
void cleanup(SDL_Window *window, SDL_Renderer *renderer)
{
    DEBUG_LOG("Starting cleanup...");

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        DEBUG_LOG("Renderer destroyed");
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        DEBUG_LOG("Window destroyed");
    }
    if (audio_stream)
    {
        SDL_DestroyAudioStream(audio_stream);
        DEBUG_LOG("Audio stream destroyed");
    }
    if (audio_device)
    {
        SDL_CloseAudioDevice(audio_device);
        DEBUG_LOG("Audio device closed");
    }

    // Print final statistics
    DEBUG_LOG("=== Final Statistics ===");
    Uint64 total_time = SDL_GetTicks() - debug_stats.start_time;
    DEBUG_LOG("Total runtime: %" SDL_PRIu64 " ms", total_time);
    DEBUG_LOG("Total frames: %" SDL_PRIu64, debug_stats.frame_count);
    DEBUG_LOG("Total audio samples: %d", debug_stats.audio_samples_generated);
    DEBUG_LOG("Total audio underruns: %d", debug_stats.audio_buffer_underruns);

    SDL_Quit();
    DEBUG_LOG("SDL quit complete");
}

int main(int argc, char *argv[])
{
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;
    
    DEBUG_LOG("Starting SDL3 Audio Synthesizer with automatic debugging");
    DEBUG_LOG("Debug mode: %s", DEBUG_ENABLED ? "ENABLED" : "DISABLED");
    DEBUG_LOG("Audio debugging: %s", DEBUG_AUDIO_DETAILED ? "VERBOSE" : "BASIC");
    DEBUG_LOG("Performance monitoring: %s", DEBUG_PERFORMANCE ? "ENABLED" : "DISABLED");

    // Initialize debug stats
    debug_stats.start_time = SDL_GetTicks();
    last_stats_print = debug_stats.start_time;

    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
    {
        DEBUG_ERROR("Error initializing SDL: %s", SDL_GetError());
        return -1;
    }
    DEBUG_LOG("SDL initialized successfully");

    // Audio specification setup
    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = SDL_AUDIO_S16;
    desired_spec.channels = 1;

    DEBUG_LOG("Requesting audio spec: %d Hz, %s, %d channels",
              desired_spec.freq, SDL_GetAudioFormatName(desired_spec.format), desired_spec.channels);

    // Open audio device (SDL3 way)
    audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!audio_device)
    {
        DEBUG_ERROR("Error opening audio device: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Store audio info for debugging
    audio_info.device_id = audio_device;
    audio_info.spec = desired_spec;
    audio_info.is_capture = false;

    DEBUG_LOG("Audio device opened successfully with ID: %u", audio_device);
    print_audio_device_info();

    // Create audio stream
    audio_stream = SDL_CreateAudioStream(&desired_spec, &desired_spec);
    if (!audio_stream)
    {
        DEBUG_ERROR("Error creating audio stream: %s", SDL_GetError());
        SDL_CloseAudioDevice(audio_device);
        SDL_Quit();
        return -1;
    }
    DEBUG_LOG("Audio stream created successfully");

    // Bind the stream to the device
    if (!SDL_BindAudioStream(audio_device, audio_stream))
    {
        DEBUG_ERROR("Error binding audio stream: %s", SDL_GetError());
        SDL_CloseAudioDevice(audio_device);
        SDL_Quit();
        return -1;
    }
    DEBUG_LOG("Audio stream bound to device");

    // Start audio playback
    if (!SDL_ResumeAudioDevice(audio_device))
    {
        DEBUG_ERROR("Error resuming audio device: %s", SDL_GetError());
    }
    DEBUG_LOG("Audio device resumed");

    // Initial audio stream validation
    validate_audio_stream();

    // Create SDL window (SDL3 way)
    SDL_Window *window = SDL_CreateWindow("(a-s-d-f keys) SDL Sinusoidal Synthesizer",
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          0);

    if (!window)
    {
        DEBUG_ERROR("Error creating window: %s", SDL_GetError());
        cleanup(window, NULL);
        return -1;
    }
    DEBUG_LOG("Window created successfully (%dx%d)", WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        DEBUG_ERROR("Error creating renderer: %s", SDL_GetError());
        cleanup(window, renderer);
        return -1;
    }
    DEBUG_LOG("Renderer created successfully");

    bool running = true;
    SDL_Event event;

    DEBUG_LOG("Entering main loop - Press A/S/D/F for notes, ESC to quit");
    DEBUG_LOG("Controls: A=440Hz, S=466Hz, D=493Hz, F=523Hz");

    while (running)
    {
        debug_stats.frame_count++;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                DEBUG_LOG("Quit event received");
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                switch (event.key.key)
                {
                case SDLK_A:
                    frequency = FREQUENCY_A4;
                    note_name = "A";
                    note_on = true;
                    DEBUG_LOG("Key pressed: A (%.2f Hz)", frequency);
                    break;
                case SDLK_S:
                    frequency = FREQUENCY_A4 * pow(2, 1.0 / 12);
                    note_name = "A#";
                    note_on = true;
                    DEBUG_LOG("Key pressed: S/A# (%.2f Hz)", frequency);
                    break;
                case SDLK_D:
                    frequency = FREQUENCY_A4 * pow(2, 2.0 / 12);
                    note_name = "B";
                    note_on = true;
                    DEBUG_LOG("Key pressed: D/B (%.2f Hz)", frequency);
                    break;
                case SDLK_F:
                    frequency = FREQUENCY_A4 * pow(2, 3.0 / 12);
                    note_name = "C";
                    note_on = true;
                    DEBUG_LOG("Key pressed: F/C (%.2f Hz)", frequency);
                    break;
                case SDLK_ESCAPE:
                    DEBUG_LOG("Escape key pressed - exiting");
                    running = false;
                    break;
                    // Add more notes as needed
                }
            }
            else if (event.type == SDL_EVENT_KEY_UP)
            {
                if (note_on)
                {
                    DEBUG_LOG("Key released - stopping note %s", note_name);
                }
                note_on = false; // Stop the note when the key is released
            }
        }

        // Generate audio samples continuously
        // Check if the stream needs more data
        int queued = SDL_GetAudioStreamQueued(audio_stream);
        if (queued < 8192) // Keep at least 8KB of audio queued
        {
            generate_audio_samples(2048);
        }

        // Validate audio stream periodically
        if (debug_stats.frame_count % 60 == 0)
        { // Every ~1 second at 60 FPS
            validate_audio_stream();
        }

        // Rendering
        Uint64 render_start = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        if (note_on)
        {
            render_text(renderer, note_name, 150, 120); // Show the note name
        }

        SDL_RenderPresent(renderer);

        // Performance tracking
        Uint64 render_end = SDL_GetPerformanceCounter();
        double render_time = (double)(render_end - render_start) / SDL_GetPerformanceFrequency() * 1000.0;
        debug_stats.total_render_time += render_time;

// Print performance stats periodically
#if DEBUG_PERFORMANCE
        if (debug_stats.frame_count % 300 == 0)
        { // Every ~5 seconds at 60 FPS
            print_performance_stats();
        }
#endif
    }

    // Cleanup and exit
    cleanup(window, renderer);
    return 0;
}
