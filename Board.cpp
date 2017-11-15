#include "Board.h"
#include <sstream>
#include "LineShape.h"
#include <cstring>

Board::Board()
{
    m_field = new float[25];
    m_activeRounds = 0;
}

Board::~Board()
{
    delete [] m_field;
}

void Board::invert()
{
    for (int i = 0; i < 24; i++)
    {
        m_field[i] *= -1.0f;
    }
}

unsigned short Board::getTurnNumber()
{
    return m_activeRounds;
}

void Board::decode(EncodedBoard b, float* target)
{
    for(int i = 23; i >= 0; i--)
    {
        target[i] = (b & 3) ? (b & 2 ? BLUE : RED) : EMPTY;
        b >>= 2;
    }
    target[24] = (b < 18) ? 0.0f : 1.0f;
}

EncodedBoard Board::invert(EncodedBoard b)
{
    uint64_t p1 = 0x0000AAAAAAAAAAAA;
    uint64_t p2 = 0x0000555555555555;
    uint64_t turn = 0xFFFF000000000000;
    return (b & turn) | ((b & p1) >> 1) | (((b & p2) << 1));
}

void Board::decode(EncodedBoard b)
{
    decode(b, m_field);
    m_activeRounds = b >> 48;
}

EncodedBoard Board::encode()
{
    EncodedBoard result = m_activeRounds;
    for(int i = 0; i < 24; i++)
    {
        result <<= 2;
        result |= (m_field[i] == EMPTY) ? 0 : ((m_field[i] == RED) ? 1 : 2);
    }
    return result;
}

bool Board::isPartOfMill(unsigned int i, float player)
{
    if(m_field[i] != player)
        return false;
    if(i % 2 == 0) // i is in a corner
    {
        return (m_field[(i + 1) % 8 + i / 8 * 8] == player && m_field[(i + 2) % 8 + i / 8 * 8] == player) ||
               (m_field[(i + 6) % 8 + i / 8 * 8] == player && m_field[(i + 7) % 8 + i / 8 * 8] == player);
    }
    else
    {
        return (m_field[(i + 1) % 8 + i / 8 * 8] == player && m_field[(i + 7) % 8 + i / 8 * 8] == player) ||
               (m_field[(i + 8) % 24] == player && m_field[(i + 16) % 24] == player);
    }
}

void Board::tryMoveTo(unsigned int from, unsigned int to, Board& next, std::vector<EncodedBoard>& result)
{
    unsigned int i = from;
    unsigned int n = to;
    if(n < 0 || n >= 24 || m_field[n] != EMPTY)
        return;
    next.m_field[n] = BLUE;
    next.m_field[i] = EMPTY;
    if(next.isPartOfMill(n, BLUE))
    {
        bool isInEnemyMill[24];
        bool enemyHasStoneNotInMill = false;
        for(int j = 0; j < 24; j++)
        {
            isInEnemyMill[j] = next.isPartOfMill(j, RED);
            enemyHasStoneNotInMill |= (next.m_field[j] == RED && !isInEnemyMill[j]);
        }
        for(int j = 0; j < 24; j++)
        {
            if(m_field[j] == RED && (!enemyHasStoneNotInMill || !isInEnemyMill[j]))
            {
                next.m_field[j] = EMPTY;
                result.push_back(next.encode());
                next.m_field[j] = RED;
            }
        }
    }
    else
    {
        result.push_back(next.encode());
    }
    next.m_field[n] = EMPTY;
    next.m_field[i] = BLUE;
}

std::vector<EncodedBoard> Board::getNextLegalStates()
{
    Board next;
    next.m_activeRounds = m_activeRounds + 1;
    std::vector<EncodedBoard> result;

    if(m_activeRounds < 18) // phase 1, place stone
    {
        memcpy(next.m_field, m_field, sizeof(float) * 24);
        for(int i = 0; i < 24; i++)
        {
            if(m_field[i] == EMPTY)
            {
                next.m_field[i] = BLUE;
                if(next.isPartOfMill(i, BLUE))
                {
                    bool isInEnemyMill[24];
                    bool enemyHasStoneNotInMill = false;
                    for(int j = 0; j < 24; j++)
                    {
                        isInEnemyMill[j] = next.isPartOfMill(j, RED);
                        enemyHasStoneNotInMill |= (next.m_field[j] == RED && !isInEnemyMill[j]);
                    }
                    for(int j = 0; j < 24; j++)
                    {
                        if(m_field[j] == RED && (!enemyHasStoneNotInMill || !isInEnemyMill[j]))
                        {
                            next.m_field[j] = EMPTY;
                            result.push_back(next.encode());
                            next.m_field[j] = RED;
                        }
                    }
                }
                else
                {
                    result.push_back(next.encode());
                }
                next.m_field[i] = EMPTY;
            }
        }
    }
    else
    {
        int blueStoneCount = 0;
        for(int i = 0; i < 24; i++)
        {
            blueStoneCount += m_field[i] == BLUE;
        }
        if(blueStoneCount < 3)
            return result;
        // TODO implement flying when blueStoneCount == 3
        memcpy(next.m_field, m_field, sizeof(float) * 24);
        for(unsigned int i = 0; i < 24; i++)
        {
            if(m_field[i] != BLUE)
                continue;
            tryMoveTo(i, (i + 1) % 8 + i / 8 * 8, next, result);
            tryMoveTo(i, (i + 7) % 8 + i / 8 * 8, next, result);
            if(i % 2 == 1) // not in corner
            {
                tryMoveTo(i, i + 8, next, result);
                tryMoveTo(i, i - 8, next, result);
            }
        }
    }

    return result;
}

void Board::draw(sf::RenderTarget& renderTarget, const sf::Vector2<float>& pos)
{
    float stoneSize = 28;
    float lineWidth = 5;
    float dotSize = 13;
    float distance = 100;
    float fontSize = 30;

    static std::vector<sf::Vector2<float>> offsetMap;
    if(offsetMap.size() == 0)
    {
        offsetMap.push_back({2, 2});
        offsetMap.push_back({3, 2});
        offsetMap.push_back({4, 2});
        offsetMap.push_back({4, 3});
        offsetMap.push_back({4, 4});
        offsetMap.push_back({3, 4});
        offsetMap.push_back({2, 4});
        offsetMap.push_back({2, 3});

        offsetMap.push_back({1, 1});
        offsetMap.push_back({3, 1});
        offsetMap.push_back({5, 1});
        offsetMap.push_back({5, 3});
        offsetMap.push_back({5, 5});
        offsetMap.push_back({3, 5});
        offsetMap.push_back({1, 5});
        offsetMap.push_back({1, 3});

        offsetMap.push_back({0, 0});
        offsetMap.push_back({3, 0});
        offsetMap.push_back({6, 0});
        offsetMap.push_back({6, 3});
        offsetMap.push_back({6, 6});
        offsetMap.push_back({3, 6});
        offsetMap.push_back({0, 6});
        offsetMap.push_back({0, 3});
    }

    sf::Vector2<float> stoneOff = {0, fontSize};
    sf::Vector2<float> lineOff = {stoneSize, stoneSize + fontSize};
    sf::Vector2<float> dotOff = {stoneSize - dotSize, stoneSize - dotSize + fontSize};

    stoneOff += pos;
    lineOff += pos;
    dotOff += pos;

    LineShape line;
    line.setColor(sf::Color(127, 127, 127));
    sf::Text roundNumber;
    roundNumber.setFont(*Board::s_font);
    std::stringstream stream;
    stream << m_activeRounds;
    roundNumber.setString(stream.str());
    roundNumber.setPosition(pos);
    roundNumber.setCharacterSize(fontSize);
    renderTarget.draw(roundNumber);
    sf::CircleShape dot(dotSize);
    dot.setFillColor(sf::Color(127, 127, 127));
    sf::CircleShape player1(stoneSize);
    player1.setFillColor(sf::Color::Blue);
    sf::CircleShape player2(stoneSize);
    player2.setFillColor(sf::Color::Red);

    for(int i = 0; i < 3; i++)
    {
        sf::Vector2<float> p1 = {distance * i + lineOff.x, distance * i + lineOff.y};
        sf::Vector2<float> p2 = {distance * (6 - i) + lineOff.x, distance * i + lineOff.y};
        sf::Vector2<float> p3 = {distance * i + lineOff.x, distance * (6 - i) + lineOff.y};
        sf::Vector2<float> p4 = {distance * (6 - i) + lineOff.x, distance * (6 - i) + lineOff.y};
        line.setAll(p1, p2, lineWidth);
        line.draw(renderTarget);
        line.setAll(p3, p4, lineWidth);
        line.draw(renderTarget);
        line.setAll(p1, p3, lineWidth);
        line.draw(renderTarget);
        line.setAll(p2, p4, lineWidth);
        line.draw(renderTarget);
    }

    line.setAll({lineOff.x, lineOff.y + distance * 3}, {lineOff.x + distance * 2, lineOff.y + distance * 3}, lineWidth);
    line.draw(renderTarget);
    line.setAll({lineOff.x + distance * 4, lineOff.y + distance * 3}, {lineOff.x + distance * 6, lineOff.y + distance * 3}, lineWidth);
    line.draw(renderTarget);
    line.setAll({lineOff.x + distance * 3, lineOff.y}, {lineOff.x + distance * 3, lineOff.y + distance * 2}, lineWidth);
    line.draw(renderTarget);
    line.setAll({lineOff.x + distance * 3, lineOff.y + distance * 4}, {lineOff.x + distance * 3, lineOff.y + distance * 6}, lineWidth);
    line.draw(renderTarget);

    for(int i = 0; i < 24; i++)
    {
        sf::Vector2<float> p = offsetMap[i] * distance;
        if(m_field[i] == EMPTY)
        {
            dot.setPosition(p + dotOff);
            renderTarget.draw(dot);
        }
        else
        {
            if(m_field[i] == BLUE)
            {
                player1.setPosition(p + stoneOff);
                renderTarget.draw(player1);
            }
            else
            {
                player2.setPosition(p + stoneOff);
                renderTarget.draw(player2);
            }
        }
    }
}
