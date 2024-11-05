#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define FREQUENCY_A4 440.0 // Frequency for A4 note (440 Hz)

// Window dimensions
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

SDL_AudioDeviceID audio_device;
bool note_on = false;
double phase = 0.0;
double frequency = FREQUENCY_A4;
const char *note_name = "A"; // Current note name

// Audio callback function for generating a sinusoidal wave
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2; // Length in samples (Sint16 is 2 bytes)
    double phase_increment = 2.0 * M_PI * frequency / SAMPLE_RATE;

    for (int i = 0; i < length; i++)
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
}

void render_text(SDL_Renderer *renderer, const char *text, int x, int y)
{
    // Placeholder rendering as a rectangle since SDL has no text rendering by default
    SDL_Rect rect = {x, y, 100, 50};
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

// Cleanup function to ensure all resources are freed
void cleanup(SDL_Window *window, SDL_Renderer *renderer)
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    if (audio_device)
        SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return -1;
    }

    // Audio specification setup
    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 1;
    desired_spec.samples = 4096;
    desired_spec.callback = audio_callback;

    // Open audio device
    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, NULL, 0);
    if (!audio_device)
    {
        SDL_Log("Error opening audio device: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Start audio playback
    SDL_PauseAudioDevice(audio_device, 0);

    // Create SDL window
    SDL_Window *window = SDL_CreateWindow("(a-s-d-f keys) SDL Sinusoidal Synthesizer",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window)
    {
        SDL_Log("Error creating window: %s", SDL_GetError());
        cleanup(window, NULL);
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        cleanup(window, renderer);
        return -1;
    }

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_a:
                    frequency = FREQUENCY_A4;
                    note_name = "A";
                    note_on = true;
                    break;
                case SDLK_s:
                    frequency = FREQUENCY_A4 * pow(2, 1.0 / 12);
                    note_name = "A#";
                    note_on = true;
                    break;
                case SDLK_d:
                    frequency = FREQUENCY_A4 * pow(2, 2.0 / 12);
                    note_name = "B";
                    note_on = true;
                    break;
                case SDLK_f:
                    frequency = FREQUENCY_A4 * pow(2, 3.0 / 12);
                    note_name = "C";
                    note_on = true;
                    break;
                    // Add more notes as needed
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                note_on = false; // Stop the note when the key is released
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        if (note_on)
        {
            render_text(renderer, note_name, 150, 120); // Show the note name
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup and exit
    cleanup(window, renderer);
    return 0;
}
