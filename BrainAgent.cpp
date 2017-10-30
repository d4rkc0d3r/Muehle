#include "BrainAgent.h"

BrainAgent::BrainAgent(const Brain& b)
{
    m_brain = b;
}

BrainAgent::~BrainAgent()
{
    //dtor
}

std::string BrainAgent::getName()
{
    return "BrainAgent";
}

int BrainAgent::selectPlay(const std::vector<EncodedBoard>& choices)
{
    float maxScore;
    int bestScoreIndex = 0;
    m_brain.setInputNeurons(choices[0]);
    m_brain.think();
    m_brain.getOutputNeurons(&maxScore);
    for (std::size_t i = 1; i < choices.size(); i++)
    {
        float testScore;
        m_brain.setInputNeurons(choices[i]);
        m_brain.think();
        m_brain.getOutputNeurons(&testScore);
        if (testScore > maxScore)
        {
            maxScore = testScore;
            bestScoreIndex = i;
        }
    }
    return bestScoreIndex;
}
