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
        static uint16_t getTurnNumber(EncodedBoard b);

        static void decode(EncodedBoard b, float* target);
        static EncodedBoard invert(EncodedBoard b);
        static void getNextLegalStates(EncodedBoard b, std::vector<EncodedBoard>& result);

        static sf::Font* s_font;

        static constexpr float BLUE = 1.0f;
        static constexpr float RED = -1.0f;
        static constexpr float EMPTY = 0;

        static constexpr uint32_t BLUE_INT = 2;
        static constexpr uint32_t RED_INT = 1;
        static constexpr uint32_t EMPTY_INT = 0;

    protected:

    private:
        public:
        unsigned short m_activeRounds;
        float m_field[25];

        static void setPosition(EncodedBoard& b, uint32_t pos, uint32_t color);
        static uint32_t getPosition(EncodedBoard b, uint32_t pos);
        static bool isRed(EncodedBoard b, uint32_t pos);
        static bool isBlue(EncodedBoard b, uint32_t pos);
        static bool isEmpty(EncodedBoard b, uint32_t pos);

        static bool isPartOfMill(EncodedBoard b, uint32_t i, uint32_t player);
        static bool fillMillMap(EncodedBoard b, uint32_t player, uint32_t& millMap);
        static void tryMoveTo(uint32_t from, uint32_t to, EncodedBoard& next, std::vector<EncodedBoard>& result);

        bool isPartOfMill(unsigned int index, float player);
        bool fillMillMap(float player, bool* millMap);
        void tryMoveTo(unsigned int from, unsigned int to, Board& next, std::vector<EncodedBoard>& result);
};

#endif // BOARD_H
