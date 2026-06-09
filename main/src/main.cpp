#include "Batch.hpp"
#include "Color.hpp"
#include "Device.hpp"
#include "Font.hpp"
#include "Math.hpp"
#include "RenderState.hpp"
#include "Utils.hpp"

extern "C" const char *__lsan_default_suppressions()
{
    return "leak:libSDL2\n"
           "leak:SDL_DBus\n";
}

namespace
{
Mat4 BuildScreenMatrix(int width, int height)
{
    return Mat4::Ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}
}

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    Device &device = Device::Instance();
    if (!device.Create(screenWidth, screenHeight, "zen3d - batch/font test", true))
    {
        LogError("[Main] Failed to create device");
        return 1;
    }

    RenderBatch batch;
    batch.Init(1, 8192);
    batch.SetMatrix(BuildScreenMatrix(device.GetWidth(), device.GetHeight()));

    Font font;
    font.SetBatch(&batch);
    font.SetFontSize(20.0f);
    font.SetSpacing(2.0f);
    if (!font.LoadDefaultFont())
    {
        LogError("[Main] Failed to load default font");
        device.Close();
        return 1;
    }

    Font ttfFont;
    ttfFont.SetBatch(&batch);
    ttfFont.SetFontSize(20.0f);
    if (!ttfFont.LoadTTFRange("../assets/fonts/DejaVuSans.ttf", 20.0f, 32, 127, 512))
    {
        LogError("[Main] Failed to load TTF font");
        device.Close();
        return 1;
    }

    Font iconFont;
    iconFont.SetBatch(&batch);
    iconFont.SetFontSize(32.0f);
    // Carrega a área privada do Unicode (PUA) usada pelo FontAwesome, gerando um atlas de 1024x1024
    if (!iconFont.LoadTTFRange("../assets/fonts/fa-solid-900.ttf", 32.0f, 0xF000, 0xF2FF, 1024))
    {
        LogWarning("[Main] Failed to load icon font. Verifique se assets/fa-solid-900.ttf existe.");
    }

    LogInfo("[Main] Running batch/font test scene");

    while (device.Run())
    {
        if (device.IsResize())
            batch.SetMatrix(BuildScreenMatrix(device.GetWidth(), device.GetHeight()));

        const float time = (float)device.GetTime();
        const float pulse = 0.5f + 0.5f * sinf(time * 2.0f);

        RenderState::Instance().SetViewport(0, 0, device.GetWidth(), device.GetHeight());
        RenderState::Instance().SetDepthTest(false);
        RenderState::Instance().SetDepthWrite(false);
        RenderState::Instance().SetCull(false);
        RenderState::Instance().SetBlend(true);
        RenderState::Instance().SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        RenderState::Instance().SetClearColor(0.08f, 0.10f, 0.14f, 1.0f);
        RenderState::Instance().Clear(true, true);

        batch.SetColor(32, 38, 48, 255);
        batch.Rectangle(36, 36, device.GetWidth() - 72, device.GetHeight() - 72, true);

        batch.SetColor(72, 92, 128, 255);
        batch.RoundedRectangle(52, 52, device.GetWidth() - 104, 118, 0.18f, 10, true);

        batch.SetColor(90, 160, 255, 255);
        batch.Circle(120, 250, 36.0f + pulse * 10.0f, true);

        batch.SetColor(255, 180, 90, 255);
        batch.Ring(260, 250, 24.0f, 42.0f + pulse * 8.0f, 0.0f, 320.0f, 48, true);

        batch.SetColor(120, 220, 160, 255);
        batch.ThickLine2D(80.0f, 340.0f, 420.0f, 420.0f + pulse * 40.0f, 8.0f);

        batch.SetColor(220, 90, 120, 255);
        batch.Rectangle(500, 210, 180, 120, false);
        //batch.Render();

        font.SetColor(Color::WHITE);
        font.Print("Zen3D batch + font smoke test", 74.0f, 86.0f);

        font.SetColor(180, 205, 255, 255);
        font.Print(74.0f, 120.0f, "FPS: %d", device.GetFPS());

        font.SetColor(200, 230, 200, 255);
        font.Print(74.0f, 152.0f, "Frame time: %.3f ms", device.GetFrameTime() * 1000.0f);

        font.SetColor(255, 220, 140, 255);
        font.Print("Default font atlas rendering is active.", 74.0f, 194.0f);

        ttfFont.SetColor(255, 255, 255, 255);
        ttfFont.Print("TTF Icons (FontAwesome):", 74.0f, 226.0f);

        if (iconFont.IsValid())
        {
            iconFont.SetColor(255, 100, 100, 255);
            // Heart (f004), User (f007), Cog (f013), Shopping Cart (f07a), Save (f0c7)
            iconFont.Print(u8"\uf004  \uf007  \uf013  \uf07a  \uf0c7", 74.0f, 260.0f);

            iconFont.SetColor(255, 215, 0, 255); // Amarelo "Gold" para os smileys
            // Smile (f118), Meh (f11a), Frown (f119)
            iconFont.Print(u8"Smileys: \uf118  \uf11a  \uf119", 74.0f, 300.0f);
        }



        batch.Render();
        device.Flip();
    }

    font.Release();
    ttfFont.Release();
    iconFont.Release();

    device.Close();
    return 0;
}
