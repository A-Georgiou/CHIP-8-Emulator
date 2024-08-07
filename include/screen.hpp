#include "raylib.h"
#include <iostream>
#include <memory>
#include <vector>

class Screen {
private:
    int width;
    int height;
    int textureWidth;
    int textureHeight;
    std::unique_ptr<Color[]> buffer;
    Texture2D smallTexture;
    RenderTexture2D renderTexture;
    Sound beep;

    void InitialiseImageTexture() {
        for (int i = 0; i < textureWidth * textureHeight; i++) {
            buffer[i] = Color{ 255, 255, 255, 255 };
        }

        Image img = {
            .data = buffer.get(),
            .width = textureWidth,
            .height = textureHeight,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            .mipmaps = 1
        };

        smallTexture = LoadTextureFromImage(img);
        renderTexture = LoadRenderTexture(width, height);
    }

public:
    Screen(const char* title, int width, int height, int textureWidth, int textureHeight, int delay)
        : width(width),
          height(height),
          textureWidth(textureWidth),
          textureHeight(textureHeight),
          buffer(std::make_unique<Color[]>(textureWidth * textureHeight))
    {
        InitWindow(width, height, title);
        InitAudioDevice();

        std::string workingDir = GetWorkingDirectory();
        std::string beepPath = workingDir + "/sounds/beep.wav"; // Adjust the path as necessary

        beep = LoadSound(beepPath.c_str());
        if (beep.frameCount == 0) {
            std::cerr << "Failed to load beep sound from path: " << beepPath << std::endl;
        }
        SetTargetFPS(delay);
        InitialiseImageTexture();
    }

    Screen(const Screen& other) = delete;
    Screen& operator=(const Screen& other) = delete;

    Screen(Screen&& other) noexcept = default;
    Screen& operator=(Screen&& other) noexcept = default;

    ~Screen() {
        CloseWindow();
        UnloadTexture(smallTexture);
        UnloadRenderTexture(renderTexture);
        UnloadSound(beep);
    }

    void Update2DTexture(const uint32_t* m_Display) {
        for (int i = 0; i < textureWidth * textureHeight; i++) {
            buffer[i] = (m_Display[i] == 0x00000000) ? Color{ 0, 0, 0, 255 } : Color{ 255, 255, 255, 255 };
        }

        UpdateTexture(smallTexture, buffer.get());
    }

   void DrawScaledTexture() const {
        BeginTextureMode(renderTexture);
        ClearBackground(BLANK);

        float scaleX = static_cast<float>(width) / textureWidth;
        float scaleY = static_cast<float>(height) / textureHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY; 

        float scaledWidth = textureWidth * scale;
        float scaledHeight = textureHeight * scale;

        float offsetX = (width - scaledWidth) / 2.0f; 
        float offsetY = (height - scaledHeight) / 2.0f;

        DrawTexturePro(smallTexture,
                       { 0, 0, static_cast<float>(smallTexture.width), -static_cast<float>(smallTexture.height) },
                       { offsetX, offsetY, scaledWidth, scaledHeight },
                       { 0, 0 }, 0.0f, WHITE);

        EndTextureMode();
        DrawTexture(renderTexture.texture, 0, 0, WHITE);
    }

    void PlayBeep() const {
        if (beep.frameCount > 0){
            PlaySound(beep);
        } else {
            std::cout << "BEEP!" << std::endl;
        }
    }

    bool ProcessInput(uint8_t* keys) const {
        std::fill(keys, keys + 16, 0);

        if (IsKeyDown(KEY_X)) keys[0] = 1;
        if (IsKeyDown(KEY_ONE)) keys[1] = 1;
        if (IsKeyDown(KEY_TWO)) keys[2] = 1;
        if (IsKeyDown(KEY_THREE)) keys[3] = 1;
        if (IsKeyDown(KEY_Q)) keys[4] = 1;
        if (IsKeyDown(KEY_W)) keys[5] = 1;
        if (IsKeyDown(KEY_E)) keys[6] = 1;
        if (IsKeyDown(KEY_A)) keys[7] = 1;
        if (IsKeyDown(KEY_S)) keys[8] = 1;
        if (IsKeyDown(KEY_D)) keys[9] = 1;
        if (IsKeyDown(KEY_Z)) keys[0xA] = 1;
        if (IsKeyDown(KEY_C)) keys[0xB] = 1;
        if (IsKeyDown(KEY_FOUR)) keys[0xC] = 1;
        if (IsKeyDown(KEY_R)) keys[0xD] = 1;
        if (IsKeyDown(KEY_F)) keys[0xE] = 1;
        if (IsKeyDown(KEY_V)) keys[0xF] = 1;

        if (IsKeyReleased(KEY_ESCAPE)) return true;

        return false;
    }
};
