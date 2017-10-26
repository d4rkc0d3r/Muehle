#include "LineShape.h"

#define PI 3.14159265359f

LineShape::LineShape()
{
    m_width = 1;
    m_start = {0, 0};
    m_target = {0, 0};
    m_shape.setSize({1, 1});
}

LineShape::~LineShape()
{
    //dtor
}

void LineShape::draw(sf::RenderTarget& target)
{
    target.draw(m_shape);
}

void LineShape::setColor(const sf::Color& color)
{
    m_shape.setFillColor(color);
}

const sf::Color& LineShape::getColor()
{
    return m_shape.getFillColor();
}

void LineShape::setStart(const sf::Vector2<float>& start)
{
    m_start = start;
    recalculate();
}

const sf::Vector2<float>& LineShape::getStart()
{
    return m_start;
}

void LineShape::setTarget(const sf::Vector2<float>& target)
{
    m_target = target;
    recalculate();
}

const sf::Vector2<float>& LineShape::getTarget()
{
    return m_target;
}

void LineShape::setWidth(float width)
{
    m_width = width;
    recalculate();
}
float LineShape::getWidth()
{
    return m_width;
}

void LineShape::setAll(const sf::Vector2<float>& start, const sf::Vector2<float>& target, float width)
{
    m_width = width;
    m_start = start;
    m_target = target;
    recalculate();
}

void LineShape::setAll(const sf::Vector2<float>& start, const sf::Vector2<float>& target, float width, const sf::Color& color)
{
    setColor(color);
    m_width = width;
    m_start = start;
    m_target = target;
    recalculate();
}

void LineShape::recalculate()
{
    if (m_start == m_target)
    {
        m_shape.setPosition(m_start);
        m_shape.setSize({1, 1});
        m_shape.setRotation(0);
        return;
    }
    float x = m_target.x - m_start.x;
    float y = m_target.y - m_start.y;
    float atan = atan2(y, x);
    m_shape.setRotation(atan / PI * 180);
    m_shape.setSize({(float)sqrt(x * x + y * y), m_width});
    m_shape.setPosition(m_start.x + -cos(atan + PI / 2) * m_width / 2, m_start.y + -sin(atan + PI / 2) * m_width / 2);
}
