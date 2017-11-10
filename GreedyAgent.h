#ifndef GREEDYAGENT_H
#define GREEDYAGENT_H

#include "AIAgent.h"
#include <random>

class GreedyAgent : public AIAgent
{
    public:
        GreedyAgent();
        GreedyAgent(unsigned int seed);
        GreedyAgent(unsigned int seed, int recursiveLevel);
        virtual ~GreedyAgent();

        void init(unsigned int seed, int recursiveLevel);

        virtual std::string getName();
        virtual int selectPlay(const std::vector<EncodedBoard>& choices);
        virtual void seed(uint32_t seed) { m_rng.seed(seed); }
    protected:

    private:
        std::mt19937 m_rng;
        std::string m_name;
        int m_recursiveLevel;
};

#endif // GREEDYAGENT_H
