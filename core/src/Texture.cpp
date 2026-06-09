#include "pch.h"
#include "Texture.hpp"
#include "Color.hpp"
#include "Utils.hpp"
#include "stb_image.h"
#include "stb_image_write.h"
#include "glad/glad.h"

Texture::Texture() : HorizontalWrap(WrapMode::Repeat),
                     VerticalWrap(WrapMode::Repeat),
                     MinificationFilter(FilterMode::NearestMipLinear),
                     MagnificationFilter(FilterMode::Linear),
                     MaxAnisotropic(0.0f)
{

    id = 0;
    width = 0;
    height = 0;
}

Texture::~Texture()
{

    Release();
}

void Texture::SetMinFilter(FilterMode filter)
{
    this->MinificationFilter = filter;
    if (id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinificationFilter);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetMagFilter(FilterMode filter)
{
    this->MagnificationFilter = filter;
    if (id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagnificationFilter);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetWrapS(WrapMode mode)
{
    this->HorizontalWrap = mode;
    if (id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, HorizontalWrap);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetWrapT(WrapMode mode)
{
    this->VerticalWrap = mode;
    if (id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, VerticalWrap);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetAnisotropicFiltering(float level)
{
    this->MaxAnisotropic = level;
    if (id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropic);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::Release()
{
    if (id != 0)
    {
        glDeleteTextures(1, &id);
        LogInfo("Texture: [ID %i] Release", id);
        id = 0;
    }
}

void Texture::createTexture(bool sizzle)
{
    if (id != 0)
    {
        glDeleteTextures(1, &id);
    }
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (sizzle)
    {

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, HorizontalWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, VerticalWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinificationFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagnificationFilter);
    if (MaxAnisotropic > 0.0f)
    {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropic);
    }
}

void Texture::Use(u32 unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}
void Texture::Update(const Pixmap &pixmap)
{
    if (pixmap.pixels)
    {
        u32 components = pixmap.components;
        u32 width = pixmap.width;
        u32 height = pixmap.height;

        GLenum format = GL_RGBA;
        GLenum glFormat = GL_RGBA;

        switch (components)
        {
        case STBI_grey:
        {
            format = GL_R8;
            glFormat = GL_RED;
            break;
        }
        case STBI_grey_alpha:
        {
            format = GL_RG8;
            glFormat = GL_RG;
            break;
        }
        case STBI_rgb:
        {
            format = GL_RGB;
            glFormat = GL_RGB;

            break;
        }
        case STBI_rgb_alpha:
        {
            format = GL_RGBA8;
            glFormat = GL_RGBA;
            break;
        }
        }

        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, pixmap.pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
void Texture::Update(const unsigned char *buffer, u16 components, int width, int height)
{
    if (buffer)
    {
        GLenum format = GL_RGBA;
        GLenum glFormat = GL_RGBA;
        switch (components)
        {
        case STBI_grey:
        {
            format = GL_R8;
            glFormat = GL_RED;
            break;
        }
        case STBI_grey_alpha:
        {
            format = GL_RG8;
            glFormat = GL_RG;
            break;
        }
        case STBI_rgb:
        {
            format = GL_RGB;
            glFormat = GL_RGB;

            break;
        }
        case STBI_rgb_alpha:
        {
            format = GL_RGBA8;
            glFormat = GL_RGBA;
            break;
        }
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, buffer);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

Texture2D::Texture2D() : Texture()
{
}

Texture2D::Texture2D(int w, int h, u16 components) : Texture()
{
    this->width = w;
    this->height = h;
    this->components = components;
}

Texture2D::Texture2D(const Pixmap &pixmap) : Texture()
{

    components = pixmap.components;
    width = pixmap.width;
    height = pixmap.height;

    GLenum format = GL_RGBA;
    GLenum glFormat = GL_RGBA;
    bool swizzle = false;
    switch (components)
    {
    case STBI_grey:
    {
        format = GL_R8;
        glFormat = GL_RED;

        break;
    }
    case STBI_grey_alpha:
    {
        format = GL_RG8;
        glFormat = GL_RG;
        swizzle = true;

        break;
    }
    case STBI_rgb:
    {
        format = GL_RGB;
        glFormat = GL_RGB;

        break;
    }
    case STBI_rgb_alpha:
    {
        format = GL_RGBA8;
        glFormat = GL_RGBA;
        break;
    }
    }

    createTexture(swizzle);

    if (pixmap.pixels)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, pixmap.pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    //   Log(0, "TEXTURE2D: [ID %i] Create Opengl Texture2D (%d,%d) bpp:%d", id, width, height, components);
}

bool Texture2D::Load(const Pixmap &pixmap)
{
    components = pixmap.components;
    width = pixmap.width;
    height = pixmap.height;
    bool swizzle = false;

    GLenum format = GL_RGBA;
    GLenum glFormat = GL_RGBA;
    switch (components)
    {
    case STBI_grey:
    {
        format = GL_R8;
        glFormat = GL_RED;

        break;
    }
    case STBI_grey_alpha:
    {
        format = GL_RG8;
        glFormat = GL_RG;
        swizzle = true;
        break;
    }
    case STBI_rgb:
    {
        format = GL_RGB;
        glFormat = GL_RGB;

        break;
    }
    case STBI_rgb_alpha:
    {
        format = GL_RGBA8;
        glFormat = GL_RGBA;
        break;
    }
    }

    createTexture(swizzle);

    if (pixmap.pixels)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, pixmap.pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    LogInfo("TEXTURE2D: [ID %i] Create Opengl Texture2D (%d,%d) bpp:%d", id, width, height, components);
    return true;
}

static unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead)
{
    unsigned char *data = NULL;
    *bytesRead = 0;

    SDL_RWops *file = SDL_RWFromFile(fileName, "rb");

    if (file != NULL)
    {
        unsigned int size = (int)SDL_RWsize(file);

        if (size > 0)
        {
            data = (unsigned char *)malloc(size * sizeof(unsigned char));

            unsigned int count = (unsigned int)SDL_RWread(file, data, sizeof(unsigned char), size);
            *bytesRead = count;

            LogInfo("FILEIO: [%s] File loaded successfully", fileName);
        }
        else
            LogError("FILEIO: [%s] Failed to read file", fileName);
        SDL_RWclose(file);
    }
    else
        LogError("FILEIO: [%s] Failed to open file", fileName);

    return data;
}

bool Texture2D::Load(const char *file_name)
{

    unsigned int bytesRead;
    unsigned char *fileData = LoadFileData(file_name, &bytesRead);
    if (!fileData)
        return false;

    unsigned char *data = stbi_load_from_memory(fileData, bytesRead, &width, &height, &components, 0);

    if (data == NULL)
    {
        LogError("Texture2D: Failed to load image: %s", file_name);
        return false;
    }

    GLenum format = GL_RGBA;
    GLenum glFormat = GL_RGBA;
    bool swizzle = false;
    switch (components)
    {
    case STBI_grey:
    {
        format = GL_R8;
        glFormat = GL_RED;
        break;
    }
    case STBI_grey_alpha:
    {
        format = GL_RG;
        glFormat = GL_RG;
        swizzle = true;
        break;
    }
    case STBI_rgb:
    {
        format = GL_RGB;
        glFormat = GL_RGB;

        break;
    }
    case STBI_rgb_alpha:
    {
        format = GL_RGBA8;
        glFormat = GL_RGBA;
        break;
    }
    }

    createTexture(swizzle);

    // GLfloat maxAniso = 1.0f;
    // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fminf(8.0f, maxAniso));

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    //   Log(0, "TEXTURE2D: [ID %i] Create Opengl Texture2D (%d,%d) bpp:%d", id, width, height, components);

    free(fileData);
    free(data);
    return true;
}

Texture2D::Texture2D(const char *file_name) : Texture()
{
    Load(file_name);
}

Texture2D *Texture2D::CreateColor(int w, int h)
{

    Texture2D *tex = new Texture2D(w, h, 4);
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture2D *Texture2D::CreateDepth(int w, int h)
{
    Texture2D *tex = new Texture2D(w, h, 4);

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, tex->width, tex->height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

bool Texture2D::LoadFromMemory(const unsigned char *buffer, u16 components, int width, int height)
{

    this->components = components;
    this->width = width;
    this->height = height;

    GLenum format = GL_RGBA;
    GLenum glFormat = GL_RGBA;
    bool swizzle = false;
    switch (components)
    {
    case STBI_grey:
    {
        format = GL_R8;
        glFormat = GL_RED;
        break;
    }
    case STBI_grey_alpha:
    {
        format = GL_RG8;
        glFormat = GL_RG;
        swizzle = true;
        break;
    }
    case STBI_rgb:
    {
        format = GL_RGB;
        glFormat = GL_RGB;

        break;
    }
    case STBI_rgb_alpha:
    {
        format = GL_RGBA8;
        glFormat = GL_RGBA;
        break;
    }
    }

    createTexture(swizzle);

    if (buffer)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, glFormat, GL_UNSIGNED_BYTE, buffer);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    LogInfo("TEXTURE2D: [ID %i] Create Opengl Texture2D (%d,%d) bpp:%d", id, width, height, components);

    return true;
}

static const GLenum types[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

CubemapTexture::CubemapTexture(
    const std::string &PosXFilename,
    const std::string &NegXFilename,
    const std::string &PosYFilename,
    const std::string &NegYFilename,
    const std::string &PosZFilename,
    const std::string &NegZFilename) : Texture()
{

    m_fileNames[0] = PosXFilename;
    m_fileNames[1] = NegXFilename;
    m_fileNames[2] = PosYFilename;
    m_fileNames[3] = NegYFilename;
    m_fileNames[4] = PosZFilename;
    m_fileNames[5] = NegZFilename;
}

bool CubemapTexture::Load()
{

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++)
    {
        int Width, Height;
        void *pData = NULL;

        int BPP;
        unsigned char *image_data = stbi_load(m_fileNames[i].c_str(), &Width, &Height, &BPP, 0);

        if (!image_data)
        {
            LogError("Can't load texture from '%s' - %s\n", m_fileNames[i].c_str(), stbi_failure_reason());
        }

        pData = image_data;

        glTexImage2D(types[i], 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_image_free(image_data);
    }

    return true;
}
TextureManager &TextureManager::Instance()
{
    static TextureManager instance;
    return instance;
}
TextureManager *TextureManager::InstancePtr()
{
    return &Instance();
}

Texture2D *TextureManager::createFromPixmap(const std::string &name, const Pixmap &pixmap)
{
    return Get(pixmap, name.c_str());
}

Texture2D *TextureManager::createFromPixmap(const char *name, const Pixmap &pixmap)
{
    return Get(pixmap, name);
}

TextureManager::TextureManager()
{
    LogInfo("TextureManager: Create");
    m_texturePath = "../assets/textures";
    m_defaultTexture = nullptr;
}

TextureManager::~TextureManager()
{
    LogInfo("TextureManager: Destroy");
 
}

void TextureManager::Clear()
{
    m_defaultTexture = nullptr;

    m_loadedTextures.clear();



    for (auto it = m_textures.begin(); it != m_textures.end(); it++)
    {
        delete it->second;
    }

    m_textures.clear();
}

void TextureManager::FlipTextureOnLoad(bool flip)
{
    int isFlip = (flip == true) ? 1 : 0;
    stbi_set_flip_vertically_on_load(isFlip);
}

void TextureManager::Initialize()
{

    {

        unsigned char checker_data[64 * 64 * 4];
        int x, y, i = 0;
        for (y = 0; y < 64; y++)
        {
            for (x = 0; x < 64; x++)
            {
                checker_data[i++] = 255;
                checker_data[i++] = 255;
                checker_data[i++] = 255;
                checker_data[i++] = 255;
            }
        }

        Pixmap image(64, 64, 4, checker_data);

       m_defaultTexture=createFromPixmap("white", image);
    }

    {
        const int size = 64 * 64 * 4;
         unsigned char checker_data[64 * 64 * 4];
        for (int i = 0; i < size; i += 4)
        {
            int x = (i / 4) % 64;
            int y = (i / 4) / 64;
            bool checker = ((x / 8) % 2) ^ ((y / 8) % 2);
            checker_data[i + 0] = checker ? 255 : 0; // R
            checker_data[i + 1] = checker ? 255 : 0; // G
            checker_data[i + 2] = checker ? 255 : 0; // B
            checker_data[i + 3] = 255;               // A
        }

        createFromPixmap("checker", Pixmap(64, 64, 4, checker_data));
    }
}

Texture2D *TextureManager::Create(const unsigned char *buffer, u16 components, int width, int height, const char *name)
{
    if (name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
    }

    Texture2D *texture = new Texture2D();
    if (texture->LoadFromMemory(buffer, components, width, height))
    {
        if (name)
        {
            m_textures.emplace(name, texture);
        }
        return texture;
    }
    delete texture;
    return nullptr;
}

Texture2D *TextureManager::Create(const Pixmap &pixmap, const char *name)
{
    if (name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
    }

    Texture2D *texture = new Texture2D();
    if (texture->Load(pixmap))
    {
        if (name)
        {
            m_textures.emplace(name, texture);
        }
        return texture;
    }
    delete texture;
    return nullptr;
}

Texture2D *TextureManager::Create(const char *file_name, const char *name)
{
    if (name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
    }

    Texture2D *texture = new Texture2D();
    if (texture->Load(file_name))
    {
        if (name)
        {
            m_textures.emplace(name, texture);
        }
        return texture;
    }
    delete texture;
    return nullptr;
}

Texture2D *TextureManager::Create(u16 components, int width, int height, const char *name)
{
    if (name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
    }

    Texture2D *texture = new Texture2D();
    if (texture->LoadFromMemory(0, components, width, height))
    {
        if (name)
        {
            m_textures.emplace(name, texture);
        }
        return texture;
    }
    delete texture;
    return nullptr;
}

Texture2D *TextureManager::Get(const Pixmap &pixmap, const char *name)
{
    Texture2D *texture = 0x0;
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        return it->second;
    }
    else
    {
        texture = new Texture2D();
        if (texture->Load(pixmap))
        {
            m_textures.emplace(name, texture);
        }
        else
        {
            delete texture;
            texture = m_defaultTexture;
        }
    }

    return texture;
}

Texture2D *TextureManager::Get(const unsigned char *buffer, u16 components, int width, int height, const char *name)
{

    Texture2D *texture = 0x0;
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        return it->second;
    }
    else
    {
        texture = new Texture2D();
        if (texture->LoadFromMemory(buffer, components, width, height))
        {
            m_textures.emplace(name, texture);
        }
        else
        {
            delete texture;
            texture = m_defaultTexture;
        }
    }

    return texture;
}

bool TextureManager::Add(Texture2D *texture, const char *name)
{
    if (m_textures.find(name) != m_textures.end())
    {
        return false;
    }
    else
    {
        m_textures.emplace(name, texture);
        return true;
    }
}

bool TextureManager::Remove(const char *file_name)
{
    Texture2D *texture = 0x0;
    auto it = m_textures.find(file_name);
    if (it != m_textures.end())
    {
        texture = it->second;
        m_textures.erase(it);
        delete texture;
        return true;
    }

    for (auto it = m_loadedTextures.begin(); it != m_loadedTextures.end(); it++)
    {
        if (*it == texture)
        {
            m_loadedTextures.erase(it);
            return true;
        }
    }

    return false;
}

Texture2D *TextureManager::Get(const char *name)
{
    std::string path = name;

    Texture2D *texture = nullptr;

    auto it = m_textures.find(path.c_str());
    if (it != m_textures.end())
    {
        LogInfo("TextureManager: Get Texture: %s", path.c_str());
        return it->second;
    }
    else
    {
        texture = new Texture2D();
        if (texture->Load(path.c_str()))
        {
            m_textures.emplace(path.c_str(), texture);
            m_loadedTextures.push_back(texture);
        }
        else
        {
            delete texture;
            texture = m_defaultTexture;
        }
    }

    return texture;
}

Texture2D *TextureManager::GetTexture(int id)
{
    if (id >= 0 && id < m_loadedTextures.size())
    {
        return m_loadedTextures[id];
    }
    return nullptr;
}

Texture2D *TextureManager::Load(const char *name)
{
    std::string path = m_texturePath + name;
    Texture2D *texture = nullptr;
    auto it = m_textures.find(path);
    if (it != m_textures.end())
    {
        LogInfo("TextureManager: Get Texture: %s", path.c_str());
        return it->second;
    }
    else
    {
        texture = new Texture2D();
        if (texture->Load(path.c_str()))
        {
            m_textures.emplace(path, texture);
            m_loadedTextures.push_back(texture);
        }
        else
        {
            delete texture;
            texture = m_defaultTexture;
        }
    }

    return texture;
}

Texture2D *TextureManager::Load(const std::string &filename, const std::string &name)
{
    std::string path = m_texturePath + "/" + filename;
    Texture2D *texture = nullptr;
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        LogInfo("TextureManager: Get Texture: %s", name.c_str());
        return it->second;
    }
    else
    {
        texture = new Texture2D();
        if (texture->Load(path.c_str()))
        {
            m_textures.emplace(name, texture);
            m_loadedTextures.push_back(texture);
        }
        else
        {
            delete texture;
            texture = m_defaultTexture;
        }
    }

    return texture;
}

bool TextureManager::LoadTexture(const char *name)
{

    std::string path = m_texturePath + name;

    auto it = m_textures.find(path.c_str());
    if (it != m_textures.end())
    {
        return true;
    }
    else
    {
        Texture2D *texture = new Texture2D();
        if (texture->Load(path.c_str()))
        {
            // LogInfo("TextureManager: Get Texture: %s", path.c_str());
            m_textures.emplace(path.c_str(), texture);
            m_loadedTextures.push_back(texture);
            return true;
        }
        else
        {
            // LogWarning("TextureManager: Texture not found: %s", name);
            delete texture;
        }
    }

    return false;
}
