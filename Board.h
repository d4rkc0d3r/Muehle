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

        void getNextLegalStates(std::vector<EncodedBoard>& result);
        std::vector<EncodedBoard> getNextLegalStates();

        unsigned short getTurnNumber();

        static void decode(EncodedBoard b, float* target);
        static EncodedBoard invert(EncodedBoard b);

        static sf::Font* s_font;

        static constexpr float BLUE = 1.0f;
        static constexpr float RED = -1.0f;
        static constexpr float EMPTY = 0;

    protected:

    private:
        unsigned short m_activeRounds;
        float m_field[25];

        bool isPartOfMill(unsigned int index, float player);
        bool fillMillMap(float player, bool* millMap);
        void tryMoveTo(unsigned int from, unsigned int to, Board& next, std::vector<EncodedBoard>& result);
};

#endif // BOARD_H
