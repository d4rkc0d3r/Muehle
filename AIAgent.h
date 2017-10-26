#ifndef AIAGENT_H
#define AIAGENT_H

#include <string>
#include "Board.h"

class AIAgent
{
    public:
        virtual std::string getName() = 0;
        virtual int selectPlay(const std::vector<EncodedBoard>& choices) = 0;

    protected:

    private:
};

#endif // AIAGENT_H
