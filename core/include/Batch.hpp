#pragma once
#include <vector>
#include "Config.hpp"
#include "Math.hpp"
#include "Color.hpp"
#include "Opengl.hpp"

class Color;
class Texture;
class Texture2D;
class Shader;

struct TexVertex
{
    Vec2 position;
    Vec2 texCoord;
};

struct HermitePoint
{
    Vec2 position;
    Vec2 tangent;
};

class RenderBatch
{
public:
    RenderBatch();
    virtual ~RenderBatch();

    void Release();

    void Init(int numBuffers = 1, int bufferElements = 10000);

    void SetColor(const Color &color);
    void SetColor(float r, float g, float b);

    void SetColor(u8 r, u8 g, u8 b, u8 a);

    void SetAlpha(float a);

    void Line2D(int startPosX, int startPosY, int endPosX, int endPosY);
    void Line2D(const Vec2 &start, const Vec2 &end);

    void Line3D(const Vec3 &start, const Vec3 &end);
    void Line3D(float startX, float startY, float startZ, float endX, float endY, float endZ);

    void Circle(int centerX, int centerY, float radius, bool fill = false);
    void Rectangle(int posX, int posY, int width, int height, bool fill = false);

    void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, bool fill = true);
    void Ellipse(int centerX, int centerY, float radiusX, float radiusY, bool fill = true);

    void Polygon(int centerX, int centerY, int sides, float radius, float rotation = 0.0f, bool fill = true);
    void Polyline(const Vec2 *points, int pointCount);

    void RoundedRectangle(int posX, int posY, int width, int height, float roundness, int segments = 8, bool fill = true);
    void CircleSector(int centerX, int centerY, float radius, float startAngle, float endAngle, int segments = 16, bool fill = true);
    void Grid(int posX, int posY, int width, int height, int cellWidth, int cellHeight);

    void TexturedPolygon(const Vec2 *points, int pointCount, unsigned int textureId);
    void TexturedPolygonCustomUV(const TexVertex *vertices, int vertexCount, unsigned int textureId);
    void TexturedQuad(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, unsigned int textureId);
    void TexturedQuad(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, const Vec2 &uv1, const Vec2 &uv2, const Vec2 &uv3, const Vec2 &uv4, unsigned int textureId);
    void TexturedTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, unsigned int textureId);
    void TexturedTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &uv1, const Vec2 &uv2, const Vec2 &uv3, unsigned int textureId);

    void BezierQuadratic(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, int segments = 20);
    void BezierCubic(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int segments = 30);
    void CatmullRomSpline(const Vec2 *points, int pointCount, int segments = 20);
    void BSpline(const Vec2 *points, int pointCount, int segments = 20, int degree = 3);
    void HermiteSpline(const HermitePoint *points, int pointCount, int segments = 20);

    // Spline com espessura
    void ThickSpline(const Vec2 *points, int pointCount, float thickness, int segments = 20);

    void Box(const BoundingBox &box);
    void Box(const BoundingBox &box, const Mat4&transform);

    void Circle3D(const Vec3& center, float radius, const Vec3& normal, int segments);
    void CircleXZ(const Vec3& center, float radius, int segments);

    void Cube(const Vec3 &position, float width, float height, float depth, bool wire = true);
    void Sphere(const Vec3 &position, float radius, int rings, int slices, bool wire = true);
    void Cone(const Vec3 &position, float radius, float height, int segments, bool wire);
    void Cylinder(const Vec3 &position, float radius, float height, int segments, bool wire);
    void Capsule(const Vec3 &position, float radius, float height, int segments, bool wire);

    void TriangleLines(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3);

    void Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3);
    void Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, const Vec2 &t1, const Vec2 &t2, const Vec2 &t3);

    void Grid(int slices, float spacing, bool axes = true);

    void Quad(const Vec2 *coords, const Vec2 *texcoords);
    void Quad(Texture *texture, const Vec2 *coords, const Vec2 *texcoords);
    void Quad(Texture2D *texture, const Vec2 *coords, const Vec2 *texcoords);
    void Quad(Texture2D *texture, float x, float y, float width, float height);
    void Quad(Texture *texture, float x, float y, float width, float height);
    void Quad(Texture2D *texture, const FloatRect &src, float x, float y, float width, float height);
    void Quad(Texture *texture, const FloatRect &src, float x, float y, float width, float height);
    void Quad(u32 texture, float x, float y, float width, float height);
    void Quad(Texture2D *texture, float x1, float y1, float x2, float y2, const FloatRect &src);
    void Quad(Texture *texture, float x1, float y1, float x2, float y2, const FloatRect &src);
    void QuadCentered(Texture2D *texture, float x, float y, float size, const FloatRect &clip);

    void DrawQuad(float x1, float y1, float x2, float y2,
                  float u0, float v0, float u1, float v1);
    
    void DrawQuad(Texture2D* texture,
                  float x1, float y1, float x2, float y2,
                  float u0, float v0, float u1, float v1);
    
    void DrawQuad(float x1, float y1, float x2, float y2,
                  float u0, float v0, float u1, float v1,
                  const Color& color);

    // Sprite / atlas helpers
    void TexturedRect(unsigned int textureId, float x, float y, float width, float height);
    void Sprite(unsigned int textureId,
                float srcX, float srcY, float srcW, float srcH,
                float dstX, float dstY, float dstW, float dstH,
                float texWidth, float texHeight);
    void SpriteEx(unsigned int textureId,
                  float srcX, float srcY, float srcW, float srcH,
                  float dstX, float dstY, float dstW, float dstH,
                  float angle, float originX, float originY,
                  bool flipH, bool flipV,
                  float texWidth, float texHeight);
    void NineSlice(unsigned int textureId,
                   float x, float y, float width, float height,
                   float borderLeft, float borderTop, float borderRight, float borderBottom,
                   float texWidth, float texHeight);

    // Thick primitives
    void ThickLine2D(float x1, float y1, float x2, float y2, float thickness);
    void Ring(int centerX, int centerY, float innerRadius, float outerRadius,
              float startAngle, float endAngle, int segments = 16, bool fill = true);
    void Arc(int centerX, int centerY, float radius,
             float startAngle, float endAngle, float thickness, int segments = 16);

    void BeginTransform(const Mat4&transform);
    void EndTransform();

    void Render();

    void SetMode(int mode);

    void Vertex2f(float x, float y);
    void Vertex3f(float x, float y, float z);
    void TexCoord2f(float x, float y);

    void SetTexture(unsigned int id);
    void SetTexture(Texture2D *texture);

    void SetMatrix(const Mat4&matrix);

private:
    // Interleaved vertex layout (pos xyz + uv + color rgba)
    struct Vertex
    {
        float x, y, z;
        float u, v;
        u8 r, g, b, a;
    };

    struct DrawCall
    {
        int   mode;
        int   first; // index into vertices[] where this draw starts
        int   count;
        unsigned int textureId;
    };

    bool CheckRenderBatchLimit(int vCount);
    void ensureDrawCall();
    bool createDeviceObjects();
    void destroyDeviceObjects();
    bool ensureIndexCapacity(std::size_t vertexCount);
    bool compileShaderProgram();
    Vec3 transformPoint(const Vec3 &p) const;

    int          maxVertices;
    int          currentMode;
    unsigned int currentTexture;
    float        currentDepth;

    bool         gpuReady;
    bool         use_matrix;
    Mat4   modelMatrix;
    Mat4   viewMatrix;

    std::vector<Vertex>   vertices;
    std::vector<DrawCall> draws;
    std::vector<unsigned int> quadIndices;

    float texcoordx, texcoordy;
    u8 colorr, colorg, colorb, colora;

    // GL objects
    GLuint programId;
    GLuint vaoId;
    GLuint vboId;
    GLuint eboId;
    GLuint whiteTextureId;
    GLint  uMvpLocation;
    GLint  uTextureLocation;
    GLint  aPosLocation;
    GLint  aUvLocation;
    GLint  aColorLocation;
};
