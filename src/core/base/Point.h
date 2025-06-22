#pragma once

#include "Types.h"

namespace alice2 {

class Point {
public:
    Point(const Vec3f& position = Vec3f(), float size = 1.0f, const Color& color = Color::White());
    
    void SetPosition(const Vec3f& position) { m_Position = position; }
    void SetSize(float size) { m_Size = size; }
    void SetColor(const Color& color) { m_Color = color; }
    
    const Vec3f& GetPosition() const { return m_Position; }
    float GetSize() const { return m_Size; }
    const Color& GetColor() const { return m_Color; }

private:
    Vec3f m_Position;
    float m_Size;
    Color m_Color;
};

} // namespace alice2 