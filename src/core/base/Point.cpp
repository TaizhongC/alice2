#include "Point.h"

namespace alice2 {

Point::Point(const Vec3f& position, float size, const Color& color)
    : m_Position(position)
    , m_Size(size)
    , m_Color(color)
{
}

} // namespace alice2 