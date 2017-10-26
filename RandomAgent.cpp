#include "RandomAgent.h"

RandomAgent::RandomAgent(unsigned int seed)
{
    m_rng.seed(seed);
}

RandomAgent::~RandomAgent()
{
    //dtor
}

std::string RandomAgent::getName()
{
    return "Random";
}

int RandomAgent::selectPlay(const std::vector<EncodedBoard>& choices)
{
    std::uniform_int_distribution<> dis(0, choices.size() - 1);
    return dis(m_rng);
}
