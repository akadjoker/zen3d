#include "pch.h"
#include "Font.hpp"
#include "Batch.hpp"
#include "Color.hpp"
#include "Texture.hpp"
 
#include "Pixmap.hpp"

 

int GetCodepoint(const char *text, int *codepointSize)
{
    int codepoint = 0x3f;
    int octet = (unsigned char)(text[0]);
    *codepointSize = 1;

    if (octet <= 0x7f)
    {
        codepoint = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        unsigned char octet1 = text[1];
        if ((octet1 == '\0') || ((octet1 >> 6) != 2))
        {
            *codepointSize = 2;
            return codepoint;
        }

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            codepoint = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *codepointSize = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2))
        {
            *codepointSize = 2;
            return codepoint;
        }

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2))
        {
            *codepointSize = 3;
            return codepoint;
        }

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f))))
        {
            *codepointSize = 2;
            return codepoint;
        }

        if ((octet >= 0xe0) && (octet <= 0xef))
        {
            codepoint = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *codepointSize = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        if (octet > 0xf4)
            return codepoint;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2))
        {
            *codepointSize = 2;
            return codepoint;
        }

        octet2 = text[2];
        if ((octet2 == '\0') || ((octet2 >> 6) != 2))
        {
            *codepointSize = 3;
            return codepoint;
        }

        octet3 = text[3];
        if ((octet3 == '\0') || ((octet3 >> 6) != 2))
        {
            *codepointSize = 4;
            return codepoint;
        }

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f))))
        {
            *codepointSize = 2;
            return codepoint;
        }

        if (octet >= 0xf0)
        {
            codepoint = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) |
                        ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *codepointSize = 4;
        }
    }

    if (codepoint > 0x10ffff)
        codepoint = 0x3f;

    return codepoint;
}

int GetCodepointNext(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;
    *codepointSize = 1;

    if (0xf0 == (0xf8 & ptr[0]))
    {
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80))
            return codepoint;

        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) |
                    ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0]))
    {
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80))
            return codepoint;

        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0]))
    {
        if ((ptr[1] & 0xC0) ^ 0x80)
            return codepoint;

        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0]))
    {
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

 

std::string FormatText(const char* fmt, ...)
{
    static char buffer[1024];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    return std::string(buffer);
}

int GetTextLength(const char* text)
{
    if (!text) return 0;
    
    int length = 0;
    int i = 0;
    
    while (text[i] != '\0')
    {
        int codepointSize = 0;
        GetCodepointNext(&text[i], &codepointSize);
        i += codepointSize;
        length++;
    }
    
    return length;
}

int GetByteLength(const char* text, int chars)
{
    if (!text || chars <= 0) return 0;
    
    int byteLength = 0;
    int charCount = 0;
    int i = 0;
    
    while (text[i] != '\0' && charCount < chars)
    {
        int codepointSize = 0;
        GetCodepointNext(&text[i], &codepointSize);
        i += codepointSize;
        byteLength += codepointSize;
        charCount++;
    }
    
    return byteLength;
}

const char* GetTextSubstring(const char* text, int start, int length)
{
    static char buffer[1024];
    
    if (!text || start < 0 || length <= 0) return "";
    
    int byteStart = GetByteLength(text, start);
    int byteLength = GetByteLength(&text[byteStart], length);
    
    if (byteLength >= sizeof(buffer))
        byteLength = sizeof(buffer) - 1;
    
    memcpy(buffer, &text[byteStart], byteLength);
    buffer[byteLength] = '\0';
    
    return buffer;
}

std::string StripColorCodes(const char* text)
{
    if (!text) return "";
    
    std::string result;
    result.reserve(strlen(text));
    
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '^' && text[i + 1] >= '0' && text[i + 1] <= '9')
        {
            i++; // Skip color code
        }
        else
        {
            result += text[i];
        }
    }
    
    return result;
}

 

Font::Font()
{
    texture = nullptr;
    batch = nullptr;

    color = Color::WHITE;
    fontSize = 16.0f;
    spacing = 1.0f;
    textLineSpacing = 0.0f;

    enableClip = false;
    clip = IntRect(0, 0, 0, 0);

    m_baseSize = 0;
    m_glyphCount = 0;
    m_glyphPadding = 0;

    m_recs.clear();
    m_glyphs.clear();
    m_glyphCache.clear();
}

Font::~Font()
{
    Release();
}

void Font::Release()
{
    if (texture)
    {
        delete texture;
        texture = nullptr;
    }

    m_recs.clear();
    m_glyphs.clear();
    m_glyphCache.clear();
}


// ========================================
// Glyph Management
// ========================================

int Font::getGlyphIndex(int codepoint)
{
    // Try cache first
    auto it = m_glyphCache.find(codepoint);
    if (it != m_glyphCache.end())
        return it->second;

    int index = 0;
    int fallbackIndex = 0;

    for (int i = 0; i < m_glyphCount; i++)
    {
        if (m_glyphs[i].value == 63) // '?'
            fallbackIndex = i;

        if (m_glyphs[i].value == codepoint)
        {
            index = i;
            m_glyphCache[codepoint] = index;
            return index;
        }
    }

    if ((index == 0) && (m_glyphs[0].value != codepoint))
        index = fallbackIndex;

    m_glyphCache[codepoint] = index;
    return index;
}

float Font::GetCharWidth(int codepoint)
{
    int index = getGlyphIndex(codepoint);
    float scaleFactor = fontSize / m_baseSize;

    if (m_glyphs[index].advanceX != 0)
        return m_glyphs[index].advanceX * scaleFactor;
    else
        return (m_recs[index].width + m_glyphs[index].offsetX) * scaleFactor;
}

// ========================================
// Text Measurement
// ========================================

Vec2 Font::GetTextSize(const char *text)
{
    TextMetrics metrics = MeasureText(text, 0.0f);
    return metrics.size;
}

float Font::GetTextWidth(const char *text)
{
    return GetTextSize(text).x;
}

float Font::GetTextHeight(const char *text)
{
    return GetTextSize(text).y;
}

TextMetrics Font::MeasureText(const char *text, float maxWidth)
{
    TextMetrics metrics;
    metrics.lineCount = 0;
    metrics.maxLineWidth = 0.0f;

    if (!text || !IsValid())
        return metrics;

    float scaleFactor = fontSize / (float)m_baseSize;
    float lineHeight = m_baseSize * scaleFactor;
    float extraLineSpacing = (textLineSpacing > 0) ? textLineSpacing : (lineHeight * 0.15f);

    metrics.lineHeight = lineHeight + extraLineSpacing;

    int size = strlen(text);
    float currentLineWidth = 0.0f;

    for (int i = 0; i < size;)
    {
        int next = 0;
        int codepoint = GetCodepointNext(&text[i], &next);
        i += next;

        if (codepoint == '\n')
        {
            metrics.lineWidths.push_back(currentLineWidth);
            if (currentLineWidth > metrics.maxLineWidth)
                metrics.maxLineWidth = currentLineWidth;

            currentLineWidth = 0.0f;
            metrics.lineCount++;
        }
        else
        {
            int index = getGlyphIndex(codepoint);

            float charWidth = 0.0f;
            if (m_glyphs[index].advanceX != 0)
                charWidth = m_glyphs[index].advanceX * scaleFactor;
            else
                charWidth = (m_recs[index].width + m_glyphs[index].offsetX) * scaleFactor;

            currentLineWidth += charWidth + spacing;
        }
    }

    // Last line
    if (currentLineWidth > 0.0f)
    {
        metrics.lineWidths.push_back(currentLineWidth);
        if (currentLineWidth > metrics.maxLineWidth)
            metrics.maxLineWidth = currentLineWidth;
        metrics.lineCount++;
    }

    if (metrics.lineCount == 0)
        metrics.lineCount = 1;

    metrics.size.x = metrics.maxLineWidth;
    metrics.size.y = metrics.lineCount * metrics.lineHeight;

    return metrics;
}

// ========================================
// Word Wrapping
// ========================================

std::vector<std::string> Font::WrapText(const char *text, float maxWidth)
{
    std::vector<std::string> lines;

    if (!text || maxWidth <= 0.0f)
    {
        lines.push_back(text ? text : "");
        return lines;
    }

    std::istringstream stream(text);
    std::string line, word;

    while (stream >> word)
    {
        std::string testLine = line.empty() ? word : line + " " + word;
        float width = GetTextWidth(testLine.c_str());

        if (width > maxWidth)
        {
            if (!line.empty())
            {
                lines.push_back(line);
                line = word;
            }
            else
            {
                // Word is too long, force break
                lines.push_back(word);
                line.clear();
            }
        }
        else
        {
            line = testLine;
        }
    }

    if (!line.empty())
        lines.push_back(line);

    return lines;
}

std::vector<std::string> Font::SplitIntoLines(const char *text)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line))
        lines.push_back(line);

    return lines;
}

float Font::GetLineWidth(const char *line)
{
    return GetTextWidth(line);
}

// ========================================
// Configuration
// ========================================

void Font::SetColor(u8 r, u8 g, u8 b, u8 a)
{
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
}

void Font::SetClip(int x, int y, int w, int h)
{
    clip.x = x;
    clip.y = y;
    clip.width = w;
    clip.height = h;
}

void Font::EnableClip(bool enable)
{
    enableClip = enable;
}

// ========================================
// Basic Rendering
// ========================================

void Font::drawTextCodepoint(int codepoint, float x, float y)
{
    if (!batch || !texture)
        return;

    int index = getGlyphIndex(codepoint);
    float scaleFactor = fontSize / m_baseSize;

    FloatRect srcRec(
        m_recs[index].x - (float)m_glyphPadding,
        m_recs[index].y - (float)m_glyphPadding,
        m_recs[index].width + 2.0f * m_glyphPadding,
        m_recs[index].height + 2.0f * m_glyphPadding);

    FloatRect dstRec(
        x + m_glyphs[index].offsetX * scaleFactor - (float)m_glyphPadding * scaleFactor,
        y + m_glyphs[index].offsetY * scaleFactor - (float)m_glyphPadding * scaleFactor,
        (m_recs[index].width + 2.0f * m_glyphPadding) * scaleFactor,
        (m_recs[index].height + 2.0f * m_glyphPadding) * scaleFactor);

    // Clipping
    if (enableClip)
    {
        if (dstRec.x < clip.x)
            return;
        if (dstRec.y < clip.y)
            return;
        if (dstRec.x + dstRec.width > clip.x + clip.width)
            return;
        if (dstRec.y + dstRec.height > clip.y + clip.height)
            return;
    }

    // Setup coordinates
    coords[0] = Vec2(dstRec.x, dstRec.y);
    coords[1] = Vec2(dstRec.x, dstRec.y + dstRec.height);
    coords[2] = Vec2(dstRec.x + dstRec.width, dstRec.y + dstRec.height);
    coords[3] = Vec2(dstRec.x + dstRec.width, dstRec.y);

    float texWidth = (float)texture->GetWidth();
    float texHeight = (float)texture->GetHeight();

    texcoords[0] = Vec2(srcRec.x / texWidth, srcRec.y / texHeight);
    texcoords[1] = Vec2(srcRec.x / texWidth, (srcRec.y + srcRec.height) / texHeight);
    texcoords[2] = Vec2((srcRec.x + srcRec.width) / texWidth, (srcRec.y + srcRec.height) / texHeight);
    texcoords[3] = Vec2((srcRec.x + srcRec.width) / texWidth, srcRec.y / texHeight);

    batch->SetColor(color);
    batch->Quad(texture, coords, texcoords);
}

void Font::Print(const char *text, float x, float y)
{
    if (!text || !batch || !IsValid())
        return;

    int size = strlen(text);
    float textOffsetX = 0.0f;
    float textOffsetY = 0.0f;
    float scaleFactor = fontSize / m_baseSize;
    float lineHeight = m_baseSize * scaleFactor;
    float extraLineSpacing = (textLineSpacing > 0) ? textLineSpacing : 0.0f;

    for (int i = 0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = getGlyphIndex(codepoint);

        if (codepoint == '\n')
        {
            textOffsetY += lineHeight + extraLineSpacing;
            textOffsetX = 0.0f;
        }
        else
        {
            if (codepoint != ' ' && codepoint != '\t')
            {
                drawTextCodepoint(codepoint, x + textOffsetX, y + textOffsetY);
            }

            if (m_glyphs[index].advanceX == 0)
                textOffsetX += (m_recs[index].width * scaleFactor + spacing);
            else
                textOffsetX += (m_glyphs[index].advanceX * scaleFactor + spacing);
        }

        i += codepointByteCount;
    }
}

void Font::Print(float x, float y, const char *fmt, ...)
{
    static char buffer[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    Print(buffer, x, y);
}

// ========================================
// Advanced Rendering
// ========================================

void Font::Print(const char *text, float x, float y, const TextStyle &style)
{
    if (!text || !batch || !IsValid())
        return;

    // Save current state
    Color oldColor = color;
    float oldFontSize = fontSize;
    float oldSpacing = spacing;
    float oldLineSpacing = textLineSpacing;

    // Apply style
    color = style.color;
    fontSize = style.fontSize;
    spacing = style.spacing;
    textLineSpacing = style.lineSpacing;

    // Render shadow
    if (style.enableShadow)
    {
        Color shadowColor = color;
        color = style.shadowColor;
        Print(text, x + style.shadowOffset.x, y + style.shadowOffset.y);
        color = shadowColor;
    }

    // Render outline
    if (style.enableOutline)
    {
        Color outlineColor = color;
        color = style.outlineColor;

        float offset = style.outlineThickness;
        Print(text, x - offset, y);
        Print(text, x + offset, y);
        Print(text, x, y - offset);
        Print(text, x, y + offset);

        color = outlineColor;
    }

    // Render main text
    Print(text, x, y);

    // Restore state
    color = oldColor;
    fontSize = oldFontSize;
    spacing = oldSpacing;
    textLineSpacing = oldLineSpacing;
}

void Font::PrintAligned(const char *text, const FloatRect &bounds,
                        TextAlign align, TextVAlign valign)
{
    if (!text || !batch || !IsValid())
        return;

    Vec2 textSize = GetTextSize(text);
    float x = bounds.x;
    float y = bounds.y;

    // Horizontal alignment
    switch (align)
    {
    case TextAlign::CENTER:
        x = bounds.x + (bounds.width - textSize.x) * 0.5f;
        break;
    case TextAlign::RIGHT:
        x = bounds.x + bounds.width - textSize.x;
        break;
    case TextAlign::LEFT:
    default:
        break;
    }

    // Vertical alignment
    switch (valign)
    {
    case TextVAlign::MIDDLE:
        y = bounds.y + (bounds.height - textSize.y) * 0.5f;
        break;
    case TextVAlign::BOTTOM:
        y = bounds.y + bounds.height - textSize.y;
        break;
    case TextVAlign::TOP:
    default:
        break;
    }

    Print(text, x, y);
}

void Font::PrintWrapped(const char *text, float x, float y, float maxWidth)
{
    if (!text || !batch || !IsValid())
        return;

    std::vector<std::string> lines = WrapText(text, maxWidth);

    float scaleFactor = fontSize / m_baseSize;
    float lineHeight = m_baseSize * scaleFactor;
    float extraLineSpacing = (textLineSpacing > 0) ? textLineSpacing : (lineHeight * 0.15f);
    float currentY = y;

    for (const auto &line : lines)
    {
        Print(line.c_str(), x, currentY);
        currentY += lineHeight + extraLineSpacing;
    }
}

void Font::PrintWrapped(const char *text, const FloatRect &bounds, const TextStyle &style)
{
    if (!text || !batch || !IsValid())
        return;

    // Save state
    Color oldColor = color;
    float oldFontSize = fontSize;
    float oldSpacing = spacing;
    float oldLineSpacing = textLineSpacing;

    // Apply style
    color = style.color;
    fontSize = style.fontSize;
    spacing = style.spacing;
    textLineSpacing = style.lineSpacing;

    // Wrap text
    std::vector<std::string> lines = WrapText(text, bounds.width);

    float scaleFactor = fontSize / m_baseSize;
    float lineHeight = m_baseSize * scaleFactor;
    float extraLineSpacing = (textLineSpacing > 0) ? textLineSpacing : (lineHeight * 0.15f);

    float totalHeight = lines.size() * (lineHeight + extraLineSpacing);
    float startY = bounds.y;

    // Vertical alignment
    switch (style.valign)
    {
    case TextVAlign::MIDDLE:
        startY = bounds.y + (bounds.height - totalHeight) * 0.5f;
        break;
    case TextVAlign::BOTTOM:
        startY = bounds.y + bounds.height - totalHeight;
        break;
    default:
        break;
    }

    float currentY = startY;

    for (const auto &line : lines)
    {
        float x = bounds.x;

        // Horizontal alignment
        switch (style.align)
        {
        case TextAlign::CENTER:
        {
            float lineWidth = GetTextWidth(line.c_str());
            x = bounds.x + (bounds.width - lineWidth) * 0.5f;
            break;
        }
        case TextAlign::RIGHT:
        {
            float lineWidth = GetTextWidth(line.c_str());
            x = bounds.x + bounds.width - lineWidth;
            break;
        }
        default:
            break;
        }

        Print(line.c_str(), x, currentY);
        currentY += lineHeight + extraLineSpacing;
    }

    // Restore state
    color = oldColor;
    fontSize = oldFontSize;
    spacing = oldSpacing;
    textLineSpacing = oldLineSpacing;
}

void Font::PrintWithShadow(const char *text, float x, float y,
                           const Color &textColor, const Color &shadowColor,
                           const Vec2 &shadowOffset)
{
    if (!text || !batch || !IsValid())
        return;

    Color oldColor = color;

    // Draw shadow
    color = shadowColor;
    Print(text, x + shadowOffset.x, y + shadowOffset.y);

    // Draw text
    color = textColor;
    Print(text, x, y);

    color = oldColor;
}

void Font::PrintWithOutline(const char *text, float x, float y,
                            const Color &textColor, const Color &outlineColor,
                            float thickness)
{
    if (!text || !batch || !IsValid())
        return;

    Color oldColor = color;

    // Draw outline (8 directions)
    color = outlineColor;
    Print(text, x - thickness, y);
    Print(text, x + thickness, y);
    Print(text, x, y - thickness);
    Print(text, x, y + thickness);
    Print(text, x - thickness, y - thickness);
    Print(text, x + thickness, y - thickness);
    Print(text, x - thickness, y + thickness);
    Print(text, x + thickness, y + thickness);

    // Draw text
    color = textColor;
    Print(text, x, y);

    color = oldColor;
}


#include "data.cc"


bool Font::LoadDefaultFont()
{
    #define BIT_CHECK(a, b) ((a) & (1u << (b)))
    
    // Default font data (you need to include your data.cc here)
    
    
    m_glyphCount = 224;
    m_glyphPadding = 0;
    
    int charsHeight = 10;
    int charsDivisor = 1;
    
    // Create pixmap with grayscale + alpha
    Pixmap pixmap(128, 128, 2);
    
    // Fill pixmap with font data
    for (int i = 0, counter = 0; i < pixmap.width * pixmap.height; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (BIT_CHECK(defaultFontData[counter], j))
            {
                ((unsigned short *)pixmap.pixels)[i + j] = 0xffff;
            }
            else
            {
                ((unsigned short *)pixmap.pixels)[i + j] = 0x00ff;
            }
        }
        counter++;
    }
    pixmap.Save("default_font.png");
    // Reserve space for glyphs
    m_glyphs.reserve(m_glyphCount);
    m_recs.reserve(m_glyphCount);
    m_glyphs.resize(m_glyphCount);
    m_recs.resize(m_glyphCount);
    
    // Create texture
    texture = new Texture2D(pixmap);
    // Calculate glyph positions
    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;
    
    for (int i = 0; i < m_glyphCount; i++)
    {
        m_glyphs[i].value = 32 + i; // First char is space (32)
        
        m_recs[i].x = (float)currentPosX;
        m_recs[i].y = (float)(charsDivisor + currentLine * (charsHeight + charsDivisor));
        m_recs[i].width = (float)charsWidth[i];
        m_recs[i].height = (float)charsHeight;
        
        testPosX += (int)(m_recs[i].width + (float)charsDivisor);
        
        if (testPosX >= pixmap.width)
        {
            currentLine++;
            currentPosX = 2 * charsDivisor + charsWidth[i];
            testPosX = currentPosX;
            
            m_recs[i].x = (float)charsDivisor;
            m_recs[i].y = (float)(charsDivisor + currentLine * (charsHeight + charsDivisor));
        }
        else
        {
            currentPosX = testPosX;
        }
        
        m_glyphs[i].offsetX = 0;
        m_glyphs[i].offsetY = 0;
        m_glyphs[i].advanceX = 0;
    }
    
    m_baseSize = (int)m_recs[0].height;
    
    // Build cache
    m_glyphCache.clear();
    for (int i = 0; i < m_glyphCount; i++)
    {
        m_glyphCache[m_glyphs[i].value] = i;
    }


    
    LogInfo("[Font] Default font loaded successfully (%d glyphs)", m_glyphCount);
    
    return true;
}

// ========================================
// LoadBMFont - Angel Code BMFont Format
// ========================================

bool Font::LoadBMFont(const char* fntPath, const char* texturePath)
{
    Release();
    
    std::ifstream file(fntPath);
    if (!file.is_open())
    {
        LogError("[Font] Failed to open BMFont file: %s", fntPath);
        return false;
    }
    
    std::string line;
    std::string textureFile;
   // int scaleW = 0, scaleH = 0;
    
    std::vector<GlyphInfo> glyphs;
    std::vector<FloatRect> recs;
    
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        
        if (type == "common")
        {
            // Parse: common lineHeight=X base=Y scaleW=Z scaleH=W
            std::string token;
            while (iss >> token)
            {
                size_t pos = token.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = token.substr(0, pos);
                    int value = std::stoi(token.substr(pos + 1));
                    
                    if (key == "lineHeight") m_baseSize = value;
                   // else if (key == "scaleW") scaleW = value;
                  //  else if (key == "scaleH") scaleH = value;
                }
            }
        }
        else if (type == "page")
        {
            // Parse: page id=0 file="texture.png"
            std::string token;
            while (iss >> token)
            {
                size_t pos = token.find("file=");
                if (pos != std::string::npos)
                {
                    textureFile = token.substr(pos + 6); // Skip 'file="'
                    textureFile.pop_back(); // Remove trailing '"'
                    break;
                }
            }
        }
        else if (type == "char")
        {
            // Parse: char id=X x=Y y=Z width=W height=H xoffset=A yoffset=B xadvance=C
            GlyphInfo glyph;
            FloatRect rec;
            
            std::string token;
            while (iss >> token)
            {
                size_t pos = token.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = token.substr(0, pos);
                    int value = std::stoi(token.substr(pos + 1));
                    
                    if (key == "id") glyph.value = value;
                    else if (key == "x") rec.x = (float)value;
                    else if (key == "y") rec.y = (float)value;
                    else if (key == "width") rec.width = (float)value;
                    else if (key == "height") rec.height = (float)value;
                    else if (key == "xoffset") glyph.offsetX = value;
                    else if (key == "yoffset") glyph.offsetY = value;
                    else if (key == "xadvance") glyph.advanceX = value;
                }
            }
            
            glyphs.push_back(glyph);
            recs.push_back(rec);
        }
    }
    
    file.close();
    
    if (glyphs.empty())
    {
        LogError("[Font] No glyphs found in BMFont file");
        return false;
    }
    
    // Load texture
    const char* texPath = texturePath ? texturePath : textureFile.c_str();
    
    Pixmap pixmap;
    if (!pixmap.Load(texPath))
    {
        LogError("[Font] Failed to load BMFont texture: %s", texPath);
        return false;
    }
    
    texture = new Texture2D(pixmap);
   
    
    // Copy glyph data
    m_glyphCount = (int)glyphs.size();
    m_glyphs = glyphs;
    m_recs = recs;
    m_glyphPadding = 0;
    
    // Build cache
    m_glyphCache.clear();
    for (int i = 0; i < m_glyphCount; i++)
    {
        m_glyphCache[m_glyphs[i].value] = i;
    }
    
    LogInfo("[Font] BMFont loaded: %s (%d glyphs)", fntPath, m_glyphCount);
    
    return true;
}

// ========================================
// LoadFromPixmap - Custom Bitmap Font
// ========================================

bool Font::LoadFromPixmap(const Pixmap& pixmap, int charWidth, int charHeight,
                         const char* charset)
{
    Release();
    
    if (!pixmap.pixels || charWidth <= 0 || charHeight <= 0)
    {
        LogError("[Font] Invalid pixmap or dimensions");
        return false;
    }
    
    // Default charset if none provided (ASCII printable)
    const char* chars = charset ? charset : 
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    
    m_glyphCount = strlen(chars);
    
    // Calculate grid
    int charsPerRow = pixmap.width / charWidth;
    int rows = (m_glyphCount + charsPerRow - 1) / charsPerRow;
    
    if (rows * charHeight > pixmap.height)
    {
        LogError("[Font] Pixmap too small for all characters");
        return false;
    }
    
    // Create texture
    texture = new Texture2D(pixmap);
    
    // Setup glyphs
    m_glyphs.resize(m_glyphCount);
    m_recs.resize(m_glyphCount);
    m_glyphPadding = 0;
    m_baseSize = charHeight;
    
    for (int i = 0; i < m_glyphCount; i++)
    {
        int row = i / charsPerRow;
        int col = i % charsPerRow;
        
        m_glyphs[i].value = chars[i];
        m_glyphs[i].offsetX = 0;
        m_glyphs[i].offsetY = 0;
        m_glyphs[i].advanceX = charWidth;
        
        m_recs[i].x = (float)(col * charWidth);
        m_recs[i].y = (float)(row * charHeight);
        m_recs[i].width = (float)charWidth;
        m_recs[i].height = (float)charHeight;
    }
    
    // Build cache
    m_glyphCache.clear();
    for (int i = 0; i < m_glyphCount; i++)
    {
        m_glyphCache[m_glyphs[i].value] = i;
    }
    
    LogInfo("[Font] Custom pixmap font loaded (%d glyphs)", m_glyphCount);
    
    return true;
}

// ========================================
// LoadTTF - TrueType Font  
// ========================================

 
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace
{
bool LoadFileToMemorySDL(const char *filePath, std::vector<unsigned char> &buffer)
{
    buffer.clear();

    SDL_RWops *file = SDL_RWFromFile(filePath, "rb");
    if (!file)
    {
        LogError("[Font] Failed to open file: %s", filePath);
        return false;
    }

    const Sint64 fileSize = SDL_RWsize(file);
    if (fileSize <= 0)
    {
        SDL_RWclose(file);
        LogError("[Font] Failed to get file size: %s", filePath);
        return false;
    }

    buffer.resize((size_t)fileSize);
    const size_t readCount = SDL_RWread(file, buffer.data(), 1, (size_t)fileSize);
    SDL_RWclose(file);
    if (readCount != (size_t)fileSize)
    {
        LogError("[Font] Failed to read file: %s", filePath);
        return false;
    }

    return true;
}
} // namespace

bool Font::LoadTTF(const char* ttfPath, float size, int atlasSize)
{
    return LoadTTFRange(ttfPath, size, 32, 126, atlasSize);
}

bool Font::LoadTTFRange(const char* ttfPath, float size,
                        int firstCodepoint, int lastCodepoint,
                        int atlasSize)
{
    std::vector<unsigned char> ttfBuffer;
    if (!LoadFileToMemorySDL(ttfPath, ttfBuffer))
        return false;

    return LoadTTFRangeFromMemory(ttfBuffer.data(), (int)ttfBuffer.size(),
                                  size, firstCodepoint, lastCodepoint, atlasSize);
}

bool Font::LoadTTFFromMemory(const unsigned char* data, int dataSize,
                            float size, int atlasSize)
{
    return LoadTTFRangeFromMemory(data, dataSize, size, 32, 126, atlasSize);
}

bool Font::LoadTTFRangeFromMemory(const unsigned char* data, int dataSize,
                                  float size,
                                  int firstCodepoint, int lastCodepoint,
                                  int atlasSize)
{
    Release();

    if (!data || dataSize <= 0)
    {
        LogError("[Font] Invalid TTF memory buffer");
        return false;
    }

    if (firstCodepoint > lastCodepoint)
        std::swap(firstCodepoint, lastCodepoint);

    if (atlasSize <= 0 || size <= 0.0f)
    {
        LogError("[Font] Invalid TTF load parameters");
        return false;
    }
    
    // Initialize font
    stbtt_fontinfo font;
    const int fontOffset = stbtt_GetFontOffsetForIndex(data, 0);
    if (fontOffset < 0 || !stbtt_InitFont(&font, data, fontOffset))
    {
        LogError("[Font] Failed to initialize TTF font");
        return false;
    }
    
    // Calculate scale
    float scale = stbtt_ScaleForPixelHeight(&font, size);
    
    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    
    m_baseSize = (int)size;
    
 
    Pixmap atlas(atlasSize, atlasSize, 2);
    //atlas.Fill(255, 255, 255, 0);  
    
    int x = 1, y = 1;
    int rowHeight = 0;
    
    const int requestedGlyphCount = lastCodepoint - firstCodepoint + 1;
    m_glyphCount = 0;
    m_glyphs.resize(requestedGlyphCount);
    m_recs.resize(requestedGlyphCount);
    m_glyphPadding = 1;

    for (int i = 0; i < requestedGlyphCount; i++)
    {
        const int codepoint = firstCodepoint + i;
        
        // Get glyph bitmap
        int width = 0, height = 0, xoff = 0, yoff = 0;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, scale, scale,
                                                         codepoint, &width, &height,
                                                         &xoff, &yoff);

        // Check if glyph fits in current row
        if (x + width + 1 > atlasSize)
        {
            x = 1;
            y += rowHeight + 1;
            rowHeight = 0;
        }
        
        // Check if we're out of space
        if (y + height + 1 > atlasSize)
        {
            LogWarning("[Font] Atlas too small for all glyphs");
            stbtt_FreeBitmap(bitmap, nullptr);
            break;
        }
        
        // Copy bitmap to atlas
        if (bitmap)
        {
            for (int by = 0; by < height; by++)
            {
                for (int bx = 0; bx < width; bx++)
                {
                    const int atlasX = x + bx;
                    const int atlasY = y + by;
                    const unsigned char value = bitmap[by * width + bx];
                    // Cor branca constante, enviando a opacidade/forma da fonte para o canal alpha
                    atlas.SetPixel(atlasX, atlasY, 255, 255, 255, value);
                }
            }
        }
        
        // Get advance
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, codepoint, &advanceWidth, &leftSideBearing);
        
        // Store glyph info
        m_glyphs[i].value = codepoint;
        m_glyphs[i].offsetX = xoff;
        m_glyphs[i].offsetY = yoff + (int)(ascent * scale);
        m_glyphs[i].advanceX = (int)(advanceWidth * scale);
        
        m_recs[i].x = (float)x;
        m_recs[i].y = (float)y;
        m_recs[i].width = (float)width;
        m_recs[i].height = (float)height;
        
        // Update position
        x += width + 1;
        if (height > rowHeight) rowHeight = height;
        m_glyphCount = i + 1;

        if (bitmap)
            stbtt_FreeBitmap(bitmap, nullptr);
    }
    
    // Create texture from atlas
    texture = new Texture2D(atlas);
    if (!texture || texture->GetID() == 0)
    {
        delete texture;
        texture = nullptr;
        LogError("[Font] Failed to create TTF atlas texture");
        return false;
    }

    m_glyphs.resize(m_glyphCount);
    m_recs.resize(m_glyphCount);
    
    // Build cache
    m_glyphCache.clear();
    for (int i = 0; i < m_glyphCount; i++)
    {
        m_glyphCache[m_glyphs[i].value] = i;
    }
    
    LogInfo("[Font] TTF font loaded (%.0fpx, %d glyphs, U+%04X..U+%04X)",
            size, m_glyphCount, firstCodepoint, lastCodepoint);
    
    return true;
}

 
