#include "Board.h"
#include <sstream>
#include "LineShape.h"
#include <cstring>

Board::Board()
{
    m_activeRounds = 0;
}

Board::~Board()
{

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
    for(int i = 0; i < 24; i++)
    {
        uint32_t p = getPosition(b, i);
        target[i] = (p & 3) ? (p & 2 ? BLUE : RED) : EMPTY;
    }
    target[24] = ((b >> 48) < 18) ? 0.0f : 1.0f;
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
    EncodedBoard result = ((uint64_t)m_activeRounds) << 48;
    for(int i = 0; i < 24; i++)
    {
        setPosition(result, i, (m_field[i] == EMPTY) ? 0 : ((m_field[i] == RED) ? 1 : 2));
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

bool Board::fillMillMap(float player, bool* millMap)
{
    for(int i = 0; i < 24; i += 2)
    {
        millMap[i] = false;
    }

    for(int i = 0; i < 24; i += 2)
    {
        if (m_field[i] != player || m_field[i + 1] != player)
            continue;
        uint32_t nextCornerIndex = (i + 2) % 8 + i / 8 * 8;
        if (m_field[nextCornerIndex] != player)
        {
            if (i % 8 != 6)
                i += 2;
            continue;
        }
        millMap[i] = true;
        millMap[i + 1] = true;
        millMap[nextCornerIndex] = true;
    }

    for(int i = 1; i < 8; i += 2)
    {
        bool mill = (m_field[i] == player && m_field[i + 8] == player && m_field[i + 16] == player);
        millMap[i] |= mill;
        millMap[i + 8] |= mill;
        millMap[i + 16] |= mill;
    }

    for(int i = 0; i < 24; i++)
    {
        if (m_field[i] == player && !millMap[i])
            return true;
    }

    return false;
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
        bool enemyHasStoneNotInMill = fillMillMap(RED, isInEnemyMill);
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

void Board::getNextLegalStates(std::vector<EncodedBoard>& result)
{
    Board next;
    next.m_activeRounds = m_activeRounds + 1;

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
                    bool enemyHasStoneNotInMill = fillMillMap(RED, isInEnemyMill);
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
            return;
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
}

std::vector<EncodedBoard> Board::getNextLegalStates()
{
    std::vector<EncodedBoard> result;
    getNextLegalStates(result);
    return result;
}

bool Board::isPartOfMill(EncodedBoard b, uint32_t i, uint32_t player)
{
    if(getPosition(b, i) != player)
        return false;
    if(i % 2 == 0) // i is in a corner
    {
        return (getPosition(b, (i + 1) % 8 + i / 8 * 8) == player && getPosition(b, (i + 2) % 8 + i / 8 * 8) == player) ||
               (getPosition(b, (i + 6) % 8 + i / 8 * 8) == player && getPosition(b, (i + 7) % 8 + i / 8 * 8) == player);
    }
    else
    {
        return (getPosition(b, (i + 1) % 8 + i / 8 * 8) == player && getPosition(b, (i + 7) % 8 + i / 8 * 8) == player) ||
               (getPosition(b, (i + 8) % 24) == player && getPosition(b, (i + 16) % 24) == player);
    }
}

bool Board::fillMillMap(EncodedBoard b, uint32_t player, uint32_t& millMap)
{
    millMap = 0;

    for(int i = 0; i < 24; i += 2)
    {
        if (getPosition(b, i) != player || getPosition(b, i + 1) != player)
            continue;
        uint32_t nextCornerIndex = (i + 2) % 8 + i / 8 * 8;
        if (getPosition(b, nextCornerIndex) != player)
        {
            if (i % 8 != 6)
                i += 2;
            continue;
        }
        millMap |= 1 << i;
        millMap |= 1 << (i + 1);
        millMap |= 1 << nextCornerIndex;
    }

    for(int i = 1; i < 8; i += 2)
    {
        bool mill = (getPosition(b, i) == player && getPosition(b, i + 8) == player && getPosition(b, i + 16) == player);
        millMap |= mill << i;
        millMap |= mill << (i + 8);
        millMap |= mill << (i + 16);
    }

    for(int i = 0; i < 24; i++)
    {
        if (getPosition(b, i) == player && !((millMap >> i) & 1))
            return true;
    }

    return false;
}

void Board::tryMoveTo(uint32_t from, uint32_t to, EncodedBoard& next, std::vector<EncodedBoard>& result)
{
    uint32_t i = from;
    uint32_t n = to;
    if(n < 0 || n >= 24 || getPosition(next, n) != EMPTY)
        return;
    setPosition(next, n, BLUE_INT);
    setPosition(next, i, EMPTY_INT);
    if(isPartOfMill(next, n, BLUE_INT))
    {
        uint32_t enemyMillMap = 0;
        bool enemyHasStoneNotInMill = fillMillMap(next, RED_INT, enemyMillMap);
        for(int j = 0; j < 24; j++)
        {
            if(isRed(next, j) && (!enemyHasStoneNotInMill || !((enemyMillMap >> j) & 1)))
            {
                setPosition(next, j, EMPTY_INT);
                result.push_back(next);
                setPosition(next, j, RED_INT);
            }
        }
    }
    else
    {
        result.push_back(next);
    }
    setPosition(next, n, EMPTY_INT);
    setPosition(next, i, BLUE_INT);
}

void Board::getNextLegalStates(EncodedBoard b, std::vector<EncodedBoard>& result)
{
    EncodedBoard next = b + (1ULL << 48);

    uint16_t activeRounds = b >> 48;

    if(activeRounds < 18) // phase 1, place stone
    {
        for(int i = 0; i < 24; i++)
        {
            if(isEmpty(b, i))
            {
                setPosition(next, i, BLUE_INT);
                if(isPartOfMill(next, i, BLUE_INT))
                {
                    uint32_t enemyMillMap = 0;
                    bool enemyHasStoneNotInMill = fillMillMap(next, RED_INT, enemyMillMap);
                    for(int j = 0; j < 24; j++)
                    {
                        if(isRed(next, j) && (!enemyHasStoneNotInMill || !((enemyMillMap >> j) & 1)))
                        {
                            setPosition(next, j, EMPTY_INT);
                            result.push_back(next);
                            setPosition(next, j, RED_INT);
                        }
                    }
                }
                else
                {
                    result.push_back(next);
                }
                setPosition(next, i, EMPTY_INT);
            }
        }
    }
    else
    {
        int blueStoneCount = 0;
        for(int i = 0; i < 24; i++)
        {
            blueStoneCount += isBlue(b, i);
        }
        if(blueStoneCount < 3)
            return;
        // TODO implement flying when blueStoneCount == 3
        for(uint32_t i = 0; i < 24; i++)
        {
            if(!isBlue(b, i))
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
}

void Board::setPosition(EncodedBoard& b, uint32_t pos, uint32_t color)
{
    b = (b & ~(3ULL << (pos * 2))) | (((uint64_t)color) << (pos * 2));
}

uint32_t Board::getPosition(EncodedBoard b, uint32_t pos)
{
    return (b >> (pos * 2)) & 3;
}

bool Board::isRed(EncodedBoard b, uint32_t pos)
{
    return RED_INT == getPosition(b, pos);
}

bool Board::isBlue(EncodedBoard b, uint32_t pos)
{
    return BLUE_INT == getPosition(b, pos);
}

bool Board::isEmpty(EncodedBoard b, uint32_t pos)
{
    return EMPTY_INT == getPosition(b, pos);
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

    EncodedBoard b = encode();
    sf::CircleShape blueMill(dotSize);
    blueMill.setFillColor(sf::Color(127, 127, 255));
    sf::CircleShape redMill(dotSize);
    redMill.setFillColor(sf::Color(255, 127, 127));

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
        if(isPartOfMill(b, i, BLUE_INT))
        {
            blueMill.setPosition(p + dotOff);
            renderTarget.draw(blueMill);
        }
        if(isPartOfMill(b, i, RED_INT))
        {
            redMill.setPosition(p + dotOff);
            renderTarget.draw(redMill);
        }
    }
}
