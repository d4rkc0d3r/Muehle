#ifndef AIAGENT_H
#define AIAGENT_H

#include <string>
#include "Board.h"

class AIAgent
{
    public:
        virtual ~AIAgent() {}
        virtual std::string getName() = 0;
        virtual int selectPlay(const std::vector<EncodedBoard>& choices) = 0;
        virtual void seed(uint32_t seed) { return; }

    protected:

    private:
};

#endif // AIAGENT_H
