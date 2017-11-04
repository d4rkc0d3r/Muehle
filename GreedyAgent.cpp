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

int evalBoard(EncodedBoard b)
{
    int sum1 = 0;
    int sum2 = 0;
    for(int i = 0; i < 24; i++)
    {
        sum1 += b & 1;
        sum2 += b & 2;
        b >>= 2;
    }
    sum2 >>= 1;
    return sum2 - sum1;
}

int evalBoardRecursive(EncodedBoard eb, int recursionLevel, bool enemyTurn)
{
    if(recursionLevel <= 0)
        return evalBoard(eb) ;//* ((enemyTurn) ? 1 : -1);
    Board b;
    b.decode(eb);
    b.invert();
    std::vector<EncodedBoard> n = b.getNextLegalStates();
    int maxScore = -24;
    int minScore = 24;
    for(std::size_t i = 0; i < n.size(); i++)
    {
        int s = -evalBoardRecursive(n[i], recursionLevel - 1, !enemyTurn);
        maxScore = (maxScore > s) ? maxScore : s;
        minScore = (minScore < s) ? minScore : s;
    }
    return enemyTurn ? minScore : maxScore;
}

int GreedyAgent::selectPlay(const std::vector<EncodedBoard>& choices)
{
    int* score = new int[choices.size()];
    int maxScore = -24;
    for(std::size_t i = 0; i < choices.size(); i++)
    {
        int s = evalBoardRecursive(choices[i], m_recursiveLevel, true);
        score[i] = s;
        maxScore = (maxScore > s) ? maxScore : s;
    }
    int scoresWithMaxScore = 0;
    for(std::size_t i = 0; i < choices.size(); i++)
    {
        scoresWithMaxScore += score[i] == maxScore;
    }
    std::uniform_int_distribution<> dis(0, scoresWithMaxScore);
    int index = dis(m_rng);
    for(std::size_t i = 0; i < choices.size(); i++)
    {
        if(score[i] != maxScore)
            continue;
        if(index-- == 0)
        {
            index = i;
            break;
        }
    }
    delete [] score;
    return index;
}
