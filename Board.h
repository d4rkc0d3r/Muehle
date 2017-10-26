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

        static constexpr float BLUE = 1.0f;
        static constexpr float RED = -1.0f;
        static constexpr float EMPTY = 0;

    protected:

    private:
        unsigned short m_activeRounds;
        float* m_field;

        bool isPartOfMill(unsigned int index, float player);
        void tryMoveTo(unsigned int from, unsigned int to, Board& next, std::vector<EncodedBoard>& result);
};

#endif // BOARD_H
