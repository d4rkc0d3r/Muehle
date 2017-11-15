#include "GreedyAgent.h"
#include <sstream>

GreedyAgent::GreedyAgent()
{
    m_rng.seed(0);
    m_recursiveLevel = 0;
    m_name = "Greedy";
}

GreedyAgent::GreedyAgent(unsigned int seed)
{
    m_rng.seed(seed);
    m_recursiveLevel = 0;
    m_name = "Greedy";
}

GreedyAgent::GreedyAgent(unsigned int seed, int recursiveLevel)
{
    m_rng.seed(seed);
    m_recursiveLevel = recursiveLevel;
    std::stringstream ss;
    ss << "MiniMax" << recursiveLevel;
    m_name = ss.str();
}

void GreedyAgent::init(unsigned int seed, int recursiveLevel)
{
    m_rng.seed(seed);
    m_recursiveLevel = recursiveLevel;
    if (recursiveLevel > 0)
    {
        std::stringstream ss;
        ss << "MiniMax" << recursiveLevel;
        m_name = ss.str();
    }
    else
    {
        m_name = "Greedy";
    }
}

GreedyAgent::~GreedyAgent()
{
    //dtor
}

std::string GreedyAgent::getName()
{
    return m_name;
}

int heuristic(EncodedBoard b)
{
    int sum1 = 0;
    int sum2 = 0;
    for (int i = 0; i < 24; i++)
    {
        sum1 += b & 1;
        sum2 += b & 2;
        b >>= 2;
    }
    sum2 >>= 1;
    if (sum1 < 3)
        return 24;
    if (sum2 < 3)
        return -24;
    return sum2 - sum1;
}

int negamax(EncodedBoard eb, int depth, int color)
{
    if (depth <= 0)
        return color * heuristic(eb);
    Board b;
    if (color < 0)
        eb = Board::invert(eb);
    b.decode(eb);
    std::vector<EncodedBoard> n;
    b.getNextLegalStates(n);
    int bestValue = -24;
    for(std::size_t i = 0; i < n.size(); i++)
    {
        EncodedBoard next = (color < 0) ? Board::invert(n[i]) : n[i];
        int s = -negamax(next, depth - 1, -color);
        bestValue = (bestValue > s) ? bestValue : s;
    }
    return bestValue;
}

int GreedyAgent::selectPlay(const std::vector<EncodedBoard>& choices)
{
    int* score = new int[choices.size()];
    int maxScore = -24;
    for (std::size_t i = 0; i < choices.size(); i++)
    {
        int s = -negamax(choices[i], m_recursiveLevel, -1);
        score[i] = s;
        maxScore = (maxScore > s) ? maxScore : s;
    }
    int scoresWithMaxScore = 0;
    for (std::size_t i = 0; i < choices.size(); i++)
    {
        scoresWithMaxScore += score[i] == maxScore;
    }
    std::uniform_int_distribution<> dis(0, scoresWithMaxScore);
    int index = dis(m_rng);
    for (std::size_t i = 0; i < choices.size(); i++)
    {
        if (score[i] != maxScore)
            continue;
        if (index-- == 0)
        {
            index = i;
            break;
        }
    }
    delete [] score;
    return index;
}
