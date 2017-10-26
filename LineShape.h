#ifndef LINESHAPE_H
#define LINESHAPE_H

#include <SFML/Graphics.hpp>

class LineShape
{
    public:
        LineShape();
        virtual ~LineShape();

        void draw(sf::RenderTarget& target);

        void setColor(const sf::Color& color);
        const sf::Color& getColor();

        void setStart(const sf::Vector2<float>& start);
        const sf::Vector2<float>& getStart();

        void setTarget(const sf::Vector2<float>& target);
        const sf::Vector2<float>& getTarget();

        void setWidth(float width);
        float getWidth();

        void setAll(const sf::Vector2<float>& start, const sf::Vector2<float>& target, float width);
        void setAll(const sf::Vector2<float>& start, const sf::Vector2<float>& target, float width, const sf::Color& color);
    protected:

    private:
        void recalculate();

        sf::RectangleShape m_shape;
        float m_width;
        sf::Vector2<float> m_start;
        sf::Vector2<float> m_target;
};

#endif // LINESHAPE_H
