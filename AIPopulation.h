#ifndef AIPOPULATION_H
#define AIPOPULATION_H

#include <vector>
#include <string>
#include <random>
#include <thread>
#include "Brain.h"
#include "BrainAgent.h"
#include "AIAgent.h"

class AIPopulation
{
    public:
        AIPopulation();
        virtual ~AIPopulation();

        uint32_t getSize() { return m_size; }
        void setSize(uint32_t val) { m_size = val; }
        uint32_t getThreadCount() { return m_threadCount; }
        void setThreadCount(uint32_t val);
        uint32_t getMatchCount() { return m_matchCount; }
        void setMatchCount(uint32_t val) { m_matchCount = val; }
        uint32_t getSurvivorCount() { return m_survivorCount; }
        void setSurvivorCount(uint32_t val) { m_survivorCount = val; }
        void setAntagonistSpawner(AIAgent* (*foo)()) { m_createAntagonist = foo; }
        void setSeed(uint32_t seed) { m_nextSeed = seed; }
        void reInitialize();
        void setNetLayerSizes(const std::vector<std::size_t>& netSizes) { m_netLayerSize = netSizes; }

        bool isDoneEvaluatingGeneration();

        uint32_t getGenNumber() { return m_genNumber; }

        std::vector<double> getScores() { return m_scores; }

        void evalGeneration();
        void evalGenerationAsync();
        void finalizeEvaluation();
        void createNextGeneration();

        void load(std::string fileName);
        void save(std::string fileName);

    protected:

    private:
        uint32_t m_size;
        uint32_t m_threadCount;
        uint32_t m_matchCount;
        uint32_t m_survivorCount;
        uint32_t m_genNumber;
        uint32_t m_nextSeed;

        AIAgent* (*m_createAntagonist)();

        std::mt19937 m_rng;

        std::vector<Brain*> m_brains;
        std::vector<BrainAgent*> m_agents;
        std::vector<AIAgent*> m_antagonists;
        std::vector<double> m_scores;
        std::vector<double> m_medianScoreHistory;
        std::vector<double> m_highestScoreHistory;
        std::vector<double> m_progress;
        std::vector<std::thread*> m_threads;

        std::vector<std::size_t> m_netLayerSize;
        void evaluateRange(uint32_t startIndex, uint32_t endIndex, uint32_t threadIndex);
        void sortByScore();
};

#endif // AIPOPULATION_H
