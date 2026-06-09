#pragma once

#include "Config.hpp"
#include "Math.hpp"
#include "Utils.hpp"
#include "Pixmap.hpp"
#include "Opengl.hpp"
#include <string>
#include <vector>
#include <map>

enum WrapMode
{
    ClampToEdge = 0x812F,
    ClampToBorder = 0x812D,
    MirroredRepeat = 0x8370,
    Repeat = 0x2901 // Default

};

enum FilterMode
{
    Nearest = 0x2600,
    Linear = 0x2601,
    NearestMipNearest = 0x2700,
    LinearMipNearest = 0x2701,
    NearestMipLinear = 0x2702, // This is the default setting
    LinearMipLinear = 0x2703
};

class Texture
{
public:
    Texture();
    virtual ~Texture();

    u32 GetID() { return id; }

    FilterMode GetMinFilter() const { return MinificationFilter; }
    FilterMode GetMagFilter() const { return MagnificationFilter; }
    WrapMode GetWrapS() const { return HorizontalWrap; }
    WrapMode GetWrapT() const { return VerticalWrap; }

    int GetWidth() { return width; }
    int GetHeight() { return height; }

    void SetMinFilter(FilterMode filter);
    void SetMagFilter(FilterMode filter);
    void SetWrapS(WrapMode mode);
    void SetWrapT(WrapMode mode);
    void SetAnisotropicFiltering(float level = -1.0f);

    void Use(u32 unit = 0);
    void Update(const Pixmap &pixmap);
    void Update(const unsigned char *buffer, u16 components, int width, int height);

    virtual void Release();

protected:
    u32 id;
    WrapMode HorizontalWrap;
    WrapMode VerticalWrap;
    FilterMode MinificationFilter;
    FilterMode MagnificationFilter;
    float MaxAnisotropic;
    int width;
    int height;

    void createTexture(bool sizzle = false);

    Texture &operator=(const Texture &other) = delete;
    Texture(const Texture &other) = delete;
};

class Texture2D : public Texture
{
public:
    Texture2D();

    Texture2D(int w, int h, u16 components);
    Texture2D(const Pixmap &pixmap);
    Texture2D(const char *file_name);

    static  Texture2D *CreateColor(int w, int h);
    static  Texture2D *CreateDepth(int w, int h);

    bool Load(const Pixmap &pixmap);
    bool Load(const char *file_name);
    bool LoadFromMemory(const unsigned char *buffer, u16 components, int width, int height);
    u32 GetID() { return id; }

private:
    friend class Texture;
    s32 components{0};
    static Texture2D *defaultTexture;
};

class CubemapTexture : public Texture
{
public:
    CubemapTexture(const std::string &PosXFilename,
                   const std::string &NegXFilename,
                   const std::string &PosYFilename,
                   const std::string &NegYFilename,
                   const std::string &PosZFilename,
                   const std::string &NegZFilename);

    bool Load();

private:
    std::string m_fileNames[6];
};

class TextureManager
{
public:
    void Initialize();

    Texture2D *Create(const unsigned char *buffer, u16 components, int width, int height,const char *name);
    Texture2D *Create(const Pixmap &pixmap , const char *name);
    Texture2D *Create(const char *file_name , const char *name);
    Texture2D *Create(u16 components, int width, int height, const char *name);

    std::string GetTexturePath() const { return m_texturePath; }

    void SetTexturePath(const std::string &path) { m_texturePath = path; }

    Texture2D *GetTexture(int id);
    u32 GetTotalTextures() { return m_loadedTextures.size(); }

    Texture2D *Load(const char *name); // use texture path
    Texture2D *Load(const std::string &path, const std::string &name);

    bool LoadTexture(const char *name);

    Texture2D *Get(const char *file_name);

    Texture2D *Get(const Pixmap &pixmap, const char *name);

    Texture2D *Get(const unsigned char *buffer, u16 components, int width, int height, const char *name);

    bool Add(Texture2D *texture, const char *name);

    bool Remove(const char *name);

    void Clear();

    void FlipTextureOnLoad(bool flip);

    Texture2D *GetDefault() { return m_defaultTexture; }

    static TextureManager &Instance();
    static TextureManager *InstancePtr();


    Texture2D *createFromPixmap(const std::string &name, const Pixmap &pixmap);
    Texture2D *createFromPixmap(const char *name, const Pixmap &pixmap);
    Texture2D *create(const std::string &name, const Pixmap &pixmap) { return createFromPixmap(name, pixmap); }
    Texture2D *create(const char *name, const Pixmap &pixmap) { return createFromPixmap(name, pixmap); }

private:
    Texture2D *m_defaultTexture;
    std::map<std::string, Texture2D *> m_textures;
    std::vector<Texture2D *> m_loadedTextures;
    std::string m_texturePath;

    TextureManager(const TextureManager &) = delete;
    TextureManager &operator=(const TextureManager &) = delete;
    TextureManager(TextureManager &&) = delete;
    TextureManager &operator=(TextureManager &&) = delete;

    TextureManager();
    ~TextureManager();
};
