#ifndef BOARD_H
#define BOARD_H

#include <SFML/Graphics.hpp>
#include <vector>

typedef unsigned long long EncodedBoard;

class Board
{
    public:
        Board();
        virtual ~Board();

        void draw(sf::RenderTarget& renderTarget, const sf::Vector2<float>& pos);

        void decode(EncodedBoard b);
        EncodedBoard encode();

        void invert();

        std::vector<EncodedBoard> getNextLegalStates();

        unsigned short getTurnNumber();

        static sf::Font* s_font;
        static std::vector<char> s_mills;
        static std::vector<std::vector<char>> s_neighbors;

        static constexpr float BLUE = 1.0f;
        static constexpr float RED = -1.0f;
        static constexpr float EMPTY = 0;

    protected:

    private:
        unsigned short m_activeRounds;
        float* m_field;

        bool isPartOfMill(int index, float player);
};

#endif // BOARD_H
