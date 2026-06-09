#include "pch.h"
#include "Device.hpp"
#include "Pixmap.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderState.hpp"
#include "Input.hpp"
#define MSF_GIF_IMPL
#include "msf_gif.h"
// Platform OpenGL/GLES headers — resolved centrally
#include "Opengl.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFontAwesome.h"

// C functions for path handling
#include <errno.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

// #include <algorithm>
// #include <cstdlib>
// #include <cstdio>
// #include <ctime>
// #include <filesystem>
// #include <vector>

struct Device::GifRecordingState
{
    FILE *file = nullptr;
    MsfGifState gif = {};
    int width = 0;
    int height = 0;
    int fps = 12;
    int frameDelayCenti = 8;
    double captureInterval = 1.0 / 12.0;
    double accumulator = 0.0;
    int framesWritten = 0;
    char path[512] = {0};
    uint8_t *pixels = nullptr;
    size_t pixels_size = 0;

    void cleanup()
    {
        if (pixels)
        {
            std::free(pixels);
            pixels = nullptr;
        }
        pixels_size = 0;
    }

    bool allocate_pixels(int w, int h)
    {
        size_t needed = static_cast<size_t>(w) * static_cast<size_t>(h) * 4u;
        if (needed != pixels_size)
        {
            cleanup();
            pixels = static_cast<uint8_t *>(std::malloc(needed));
            if (!pixels)
                return false;
            pixels_size = needed;
        }
        return true;
    }
};
 

double GetTime() { return static_cast<double>(SDL_GetTicks()) / 1000; }

//*************************************************************************************************
// Device
//*************************************************************************************************

Device &Device::Instance()
{
    static Device instance;
    return instance;
}
Device *Device::InstancePtr() { return &Instance(); }

Device::Device() : m_width(0), m_height(0)
{
    LogInfo("[DEVICE] Initialized.");

    m_shouldclose = false;
    m_window = NULL;
    m_context = NULL;
    m_current = 0;
    m_previous = 0;
    m_update = 0;
    m_draw = 0;
    m_frame = 0;
    m_target = 0;
    m_vsyncEnabled = false;
    m_ready = false;
    m_is_resize = false;
}

Device::~Device()
{
    LogInfo("[DEVICE] Destroyed.");
    Close();
}

bool Device::Create(int width, int height, const char *title, bool vzync, u16 monitorIndex)
{

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        LogError( "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }
    m_current = 0;
    m_previous = 0;
    m_update = 0;
    m_draw = 0;
    m_frame = 0;

    // Do not combine software frame limiter with VSync.
    // Running both can cause pacing oscillation (e.g. bouncing 30/60 FPS).
    SetTargetFPS(vzync ? 0 : 1000);
    m_closekey = 256;
    m_width = width;
    m_height = height;

    m_current = GetTime();
    m_draw = m_current - m_previous;
    m_previous = m_current;
    m_frame = m_update + m_draw;

    // // Atributos de contexto antes de criar a janela
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    const int majorVersion = 3;
    const int minorVersion = 3; // pede 3.2 para ter debug core (altera para 3.1 para nao ter)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Formato de framebuffer
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // MSAA
    int sampleCount = 1; // mete >0 para ativar
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, sampleCount);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, sampleCount > 0 ? 1 : 0);

    int numDisplays = SDL_GetNumVideoDisplays();
    LogInfo("[Device] Num Displays: %d", numDisplays);
    for (int i = 0; i < numDisplays; i++)
    {
        const char *displayName = SDL_GetDisplayName(i);
        LogInfo("[Device] Display: %d - %s", i, displayName);
    }

    if (monitorIndex > numDisplays)
    {
        monitorIndex = 0;
    }

    // Criação
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED_DISPLAY(monitorIndex),
        SDL_WINDOWPOS_CENTERED_DISPLAY(monitorIndex),
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        LogError( "[Device] Window! %s", SDL_GetError());
        return false;
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        LogError( "[Device] Context! %s", SDL_GetError());
        return false;
    }

    // VSync
    SDL_GL_SetSwapInterval(vzync ? 1 : 0);
    m_vsyncEnabled = vzync;

    LogInfo("Load opengl extensions.");

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        LogError( "[Device] Failed to load GLES with glad");
        return false;
    }
    if (!(GLAD_GL_ES_VERSION_3_1))
    {
        LogError( "OpenGL ES 3.1 is required");
        return false;
    }
#endif

    LogInfo("[DEVICE] Vendor  : %s", (const char *)glGetString(GL_VENDOR));
    LogInfo("[DEVICE] Renderer: %s", (const char *)glGetString(GL_RENDERER));
    LogInfo("[DEVICE] Version : %s", (const char *)glGetString(GL_VERSION));
    LogInfo("[DEVICE] GLSL ES : %s",
            (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLfloat maxAniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);

    LogInfo("[DEVICE] Anisotropy: %f", maxAniso);

    RenderState::Instance().Initialize();
    TextureManager::Instance().Initialize();
    ShaderManager::Instance().Initialize();

    m_ready = true;

    return true;
}

void Device::Wait(float ms) { SDL_Delay(ms); }

int Device::GetFPS(void)
{
#define FPS_CAPTURE_FRAMES_COUNT 30   // 30 captures
#define FPS_AVERAGE_TIME_SECONDS 0.5f // 500 millisecondes
#define FPS_STEP (FPS_AVERAGE_TIME_SECONDS / FPS_CAPTURE_FRAMES_COUNT)

    static int index = 0;
    static float history[FPS_CAPTURE_FRAMES_COUNT] = {0};
    static float average = 0, last = 0;
    float fpsFrame = GetFrameTime();

    if (fpsFrame == 0)
        return 0;

    if ((GetTime() - last) > FPS_STEP)
    {
        last = (float)GetTime();
        index = (index + 1) % FPS_CAPTURE_FRAMES_COUNT;
        average -= history[index];
        history[index] = fpsFrame / FPS_CAPTURE_FRAMES_COUNT;
        average += history[index];
    }

    return (int)roundf(1.0f / average);
}

void Device::SetTargetFPS(int fps)
{
    if (fps < 1)
        m_target = 0.0;
    else
        m_target = 1.0 / (double)fps;
}

float Device::GetFrameTime(void) { return (float)m_frame; }

double Device::GetTime(void) { return (double)SDL_GetTicks() / 1000.0; }

u32 Device::GetTicks(void) { return SDL_GetTicks(); }

bool Device::Run()
{
    if (!m_ready)
        return false;

    m_current = GetTime(); // Number of elapsed seconds since InitTimer()
    m_update = m_current - m_previous;
    m_previous = m_current;
    m_is_resize = false;

    SDL_Event event;
    Input::Update();

    while (SDL_PollEvent(&event) != 0)
    {
        bool imguiMouse = false;
        bool imguiKeyboard = false;

        if (m_imguiReady)
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            // When ImGui is capturing input, don't forward to the game Input system.
            const ImGuiIO &io = ImGui::GetIO();
            imguiMouse = io.WantCaptureMouse;
            imguiKeyboard = io.WantCaptureKeyboard;
        }

        switch (event.type)
        {
        case SDL_QUIT:
        {
            m_shouldclose = true;
            break;
        }
        case SDL_WINDOWEVENT:
        {
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
            {
                m_width = event.window.data1;
                m_height = event.window.data2;
                m_is_resize = true;
                break;
            }
            }
            break;
        }
        case SDL_KEYDOWN:
        {
            if (event.key.keysym.sym == m_closekey)
            {
                SetShouldClose(true);
                break;
            }
            if (event.key.keysym.sym == SDLK_F10)
            {
                TakeScreenshot("screenshot.png");
                break;
            }
            if (event.key.keysym.sym == SDLK_F11)
            {
                if (IsGifRecording())
                    EndGifRecording();
                else
                    BeginGifRecording();
                break;
            }

            if (!imguiKeyboard)
                Input::OnKeyDown(event.key);
            break;
        }
        case SDL_KEYUP:
        {
            if (!imguiKeyboard)
                Input::OnKeyUp(event.key);
            break;
        }
        case SDL_TEXTINPUT:
        {
            if (!imguiKeyboard)
                Input::OnTextInput(event.text);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (!imguiMouse)
                Input::OnMouseDown(event.button);
            break;
        }
        case SDL_MOUSEBUTTONUP:
        {
            if (!imguiMouse)
                Input::OnMouseUp(event.button);
            break;
        }
        case SDL_MOUSEMOTION:
        {
            if (!imguiMouse)
                Input::OnMouseMove(event.motion);
            break;
        }
        case SDL_MOUSEWHEEL:
        {
            if (!imguiMouse)
                Input::OnMouseWheel(event.wheel);
            break;
        }
        }
    }

    return !m_shouldclose;
}

int Device::PollEvents(SDL_Event *event)
{
    if (!m_ready)
        return false;
    m_current = GetTime();
    m_update = m_current - m_previous;
    m_previous = m_current;
    int ret = SDL_PollEvent(event);
    if (ret && m_imguiReady)
        ImGui_ImplSDL2_ProcessEvent(event);
    return ret;
}

void Device::Close()
{
    if (!m_ready)
        return;

    EndGifRecording();
 
    m_ready = false;

    ImGuiShutdown();

    RenderState::Instance().Shutdown();
    TextureManager::Instance().Clear();
    ShaderManager::Instance().Clear();

    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);

    m_window = NULL;
    LogInfo("[DEVICE] closed!");
    SDL_Quit();
}

void Device::Flip()
{
    
    CaptureGifFrame();
 

    SDL_GL_SwapWindow(m_window);

    m_current = GetTime();
    m_draw = m_current - m_previous;
    m_previous = m_current;
    m_frame = m_update + m_draw;

    // Wait for some milliseconds...
    if (!m_vsyncEnabled && m_target > 0.0 && m_frame < m_target)
    {
        Wait((float)(m_target - m_frame) * 1000.0f);

        m_current = GetTime();
        double waitTime = m_current - m_previous;
        m_previous = m_current;

        m_frame += waitTime; // Total frame time: update + draw + wait
    }

    m_is_resize = false;
}

bool Device::IsRunning() const
{
    return m_ready && !m_shouldclose;
}

void Device::ImGuiInit(const char *glsl_version)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImFont *baseFont = io.Fonts->Fonts.empty() ? io.Fonts->AddFontDefault() : io.Fonts->Fonts[0];
    const bool iconsLoaded = ImGuiFontAwesome::MergeSolid(io, baseFont, 13.0f);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    m_imguiReady = true;
    LogInfo("[DEVICE] ImGui initialised (%s)", glsl_version);
    LogInfo("[DEVICE] ImGui FontAwesome: %s", iconsLoaded ? "loaded" : "not loaded");
}

void Device::ImGuiBegin()
{
    if (!m_imguiReady)
        return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Device::ImGuiEnd()
{
   if (m_imguiReady)
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        RenderState::Instance().UseProgram(0);
    }

}

void Device::ImGuiShutdown()
{
    if (!m_imguiReady)
        return;
    m_imguiReady = false;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    LogInfo("[DEVICE] ImGui shut down");
}

// Helper functions for C-only path building
static void build_default_gif_path(char *buffer, size_t buffer_size)
{
    time_t now = time(nullptr);
    tm tmNow = {};
#if defined(_WIN32)
    localtime_s(&tmNow, &now);
#else
    localtime_r(&now, &tmNow);
#endif
    char stamp[32];
    strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tmNow);

    // Get current working directory
    char cwd[512] = {0};
#if defined(_WIN32)
    _getcwd(cwd, sizeof(cwd) - 1);
#else
    getcwd(cwd, sizeof(cwd) - 1);
#endif

    std::snprintf(buffer, buffer_size, "%s/zen3d_%s.gif", cwd, stamp);
}

 

static bool create_directories_recursive(const char *path)
{
    if (!path || !path[0])
        return false;

    char buffer[512];
    std::strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    for (size_t i = 0; buffer[i]; ++i)
    {
#if defined(_WIN32)
        if (buffer[i] == '/' || buffer[i] == '\\')
        {
#else
        if (buffer[i] == '/')
        {
#endif
            char tmp = buffer[i];
            buffer[i] = '\0';
            if (buffer[0] != '\0')
            {
#if defined(_WIN32)
                _mkdir(buffer);
#else
                mkdir(buffer, 0755);
#endif
            }
            buffer[i] = tmp;
        }
    }

    // Create final directory
#if defined(_WIN32)
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

static const char *extract_directory(const char *path, char *buffer, size_t buffer_size)
{
    if (!path || !path[0])
        return ".";

    std::strncpy(buffer, path, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';

    size_t len = std::strlen(buffer);
    for (size_t i = len; i > 0; --i)
    {
#if defined(_WIN32)
        if (buffer[i - 1] == '/' || buffer[i - 1] == '\\')
        {
#else
        if (buffer[i - 1] == '/')
        {
#endif
            buffer[i - 1] = '\0';
            return buffer;
        }
    }
    return ".";
}

Pixmap *Device::CaptureFramebuffer()
{
    // Obter tamanho da janela
    int w = GetWidth();
    int h = GetHeight();

    // Criar Pixmap RGBA
    Pixmap *screenshot = new Pixmap(w, h, 4);

    if (!screenshot || !screenshot->IsValid())
    {
        LogError( "[Device] Failed to create screenshot pixmap");
        return nullptr;
    }

    // Ler pixels do framebuffer
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, screenshot->pixels);

    // OpenGL lê de baixo para cima, então flip vertical
    screenshot->FlipVertical();

    LogInfo("[Device] Captured framebuffer: %dx%d", w, h);

    return screenshot;
}

bool Device::TakeScreenshot(const char *filename)
{
    Pixmap *screenshot = CaptureFramebuffer();

    if (!screenshot)
    {
        LogError( "[Device] Failed to capture framebuffer");
        return false;
    }

    char default_path[512];
    const char* final_path = filename;
    
    if (!filename || filename[0] == '\0')
    {
        time_t now = time(nullptr);
        tm tmNow = {};
#if defined(_WIN32)
        localtime_s(&tmNow, &now);
#else
        localtime_r(&now, &tmNow);
#endif
        char stamp[32];
        strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tmNow);

        char cwd[512] = {0};
#if defined(_WIN32)
        _getcwd(cwd, sizeof(cwd) - 1);
#else
        getcwd(cwd, sizeof(cwd) - 1);
#endif

        std::snprintf(default_path, sizeof(default_path), "%s/captures/screenshot_%s.png", cwd, stamp);
        final_path = default_path;

        char dir_buffer[512];
        const char *dir = extract_directory(final_path, dir_buffer, sizeof(dir_buffer));
        if (dir && dir[0] != '\0' && std::strcmp(dir, ".") != 0)
        {
            create_directories_recursive(dir);
        }
    }

    // Salvar como PNG
    bool success = screenshot->Save(final_path);

    if (success)
    {
        LogInfo("[Device] Screenshot saved: %s", final_path);
    }
    else
    {
        LogError( "[Device] Failed to save screenshot: %s", final_path);
    }

    delete screenshot;
    return success;
}

bool Device::BeginGifRecording(const char *filename, int fps)
{
    if (!m_ready || !m_window)
        return false;
    if (IsGifRecording())
        return false;

    GifRecordingState *recording = new GifRecordingState();

    // Set output path
    if (filename && filename[0] != '\0')
    {
        std::strncpy(recording->path, filename, sizeof(recording->path) - 1);
        recording->path[sizeof(recording->path) - 1] = '\0';
    }
    else
    {
        build_default_gif_path(recording->path, sizeof(recording->path));
    }

    // Create directories if needed
    char dir_buffer[512];
    const char *dir = extract_directory(recording->path, dir_buffer, sizeof(dir_buffer));
    if (dir && dir[0] != '\0' && std::strcmp(dir, ".") != 0)
    {
        create_directories_recursive(dir);
    }

    recording->width = GetWidth();
    recording->height = GetHeight();
    recording->fps = (fps < 1) ? 1 : (fps > 50) ? 50
                                                : fps;
    recording->frameDelayCenti = (int)((100.0 / static_cast<double>(recording->fps)) + 0.5);
    if (recording->frameDelayCenti < 1)
        recording->frameDelayCenti = 1;
    recording->captureInterval = recording->frameDelayCenti / 100.0;
    recording->accumulator = recording->captureInterval;

    // Allocate pixel buffer
    if (!recording->allocate_pixels(recording->width, recording->height))
    {
        LogError( "[Device] Failed to allocate pixel buffer for GIF");
        delete recording;
        return false;
    }

    recording->file = std::fopen(recording->path, "wb");
    if (!recording->file)
    {
        LogError( "[Device] Failed to open GIF file: %s", recording->path);
        delete recording;
        return false;
    }

    msf_gif_alpha_threshold = 0;
    msf_gif_bgra_flag = 0;
    if (!msf_gif_begin_to_file(&recording->gif, recording->width, recording->height,
                               reinterpret_cast<MsfGifFileWriteFunc>(std::fwrite), recording->file))
    {
        LogError( "[Device] Failed to begin GIF recording");
        std::fclose(recording->file);
        delete recording;
        return false;
    }

    LogInfo("[Device] GIF recording started: %s (%dx%d @ %d fps)",
            recording->path, recording->width, recording->height, recording->fps);
    m_gifRecording = recording;
    return true;
}

bool Device::EndGifRecording()
{
    if (!m_gifRecording)
        return false;

    bool success = true;
    if (m_gifRecording->file)
    {
        success = msf_gif_end_to_file(&m_gifRecording->gif) != 0;
        std::fclose(m_gifRecording->file);
        m_gifRecording->file = nullptr;
    }

    LogInfo(success
                ? "[Device] GIF recording saved: %s (%d frames)"
                : "[Device] GIF recording failed while finalizing: %s",
            m_gifRecording->path,
            m_gifRecording->framesWritten);

    m_gifRecording->cleanup();
    delete m_gifRecording;
    m_gifRecording = nullptr;
    return success;
}

bool Device::IsGifRecording() const
{
    return static_cast<bool>(m_gifRecording);
}

const char *Device::GetGifRecordingPath() const
{
    return m_gifRecording ? m_gifRecording->path : "";
}

int Device::GetGifRecordingFPS() const
{
    return m_gifRecording ? m_gifRecording->fps : 0;
}

int Device::GetGifRecordingFrameCount() const
{
    return m_gifRecording ? m_gifRecording->framesWritten : 0;
}
 

void Device::CaptureGifFrame()
{
    if (!m_gifRecording)
        return;

    GifRecordingState &recording = *m_gifRecording;
    if (GetWidth() != recording.width || GetHeight() != recording.height)
    {
        LogInfo("[Device] Window resized during GIF recording, stopping capture");
        EndGifRecording();
        return;
    }

    const double frameSeconds = (m_update > 0.0) ? m_update : ((m_target > 0.0) ? m_target : (1.0 / 60.0));
    recording.accumulator += frameSeconds;
    if (recording.framesWritten > 0 && recording.accumulator + 1e-9 < recording.captureInterval)
        return;

    recording.accumulator = (recording.accumulator - recording.captureInterval > 0.0) ? (recording.accumulator - recording.captureInterval) : 0.0;
    glReadPixels(0, 0, recording.width, recording.height, GL_RGBA, GL_UNSIGNED_BYTE, recording.pixels);

    const int rowBytes = recording.width * 4;
    for (int y = 0; y < recording.height / 2; ++y)
    {
        uint8_t *top = recording.pixels + static_cast<size_t>(y) * static_cast<size_t>(rowBytes);
        uint8_t *bottom = recording.pixels +
                          static_cast<size_t>(recording.height - 1 - y) * static_cast<size_t>(rowBytes);
        for (int x = 0; x < rowBytes; ++x)
        {
            uint8_t tmp = top[x];
            top[x] = bottom[x];
            bottom[x] = tmp;
        }
    }

    if (!msf_gif_frame_to_file(&recording.gif, recording.pixels,
                               recording.frameDelayCenti, 16, rowBytes))
    {
        LogError( "[Device] Failed to append GIF frame");
        EndGifRecording();
        return;
    }

    recording.framesWritten++;
}
 
 