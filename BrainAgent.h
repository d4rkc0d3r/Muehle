#ifndef BRAINAGENT_H
#define BRAINAGENT_H

#include "AIAgent.h"
#include "Brain.h"

class BrainAgent : public AIAgent
{
    public:
        BrainAgent();
        BrainAgent(const Brain& b);
        virtual ~BrainAgent();

        void setBrain(const Brain& b);

        virtual std::string getName();
        virtual int selectPlay(const std::vector<EncodedBoard>& choices);
    protected:

    private:
        Brain m_brain;
};

#endif // BRAINAGENT_H
