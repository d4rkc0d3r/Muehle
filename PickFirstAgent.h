#ifndef PICKFIRSTAGENT_H
#define PICKFIRSTAGENT_H

#include "AIAgent.h"


class PickFirstAgent : public AIAgent
{
    public:
        PickFirstAgent();
        virtual ~PickFirstAgent();

        virtual std::string getName();
        virtual int selectPlay(const std::vector<EncodedBoard>& choices);
    protected:

    private:
};

#endif // PICKFIRSTAGENT_H
