#ifndef RANDOMAGENT_H
#define RANDOMAGENT_H

#include "AIAgent.h"
#include <random>

class RandomAgent : public AIAgent
{
    public:
        RandomAgent(unsigned int seed);
        virtual ~RandomAgent();

        virtual std::string getName();
        virtual int selectPlay(const std::vector<EncodedBoard>& choices);
    protected:

    private:
        std::mt19937 m_rng;
};

#endif // RANDOMAGENT_H
