#include "AIPopulation.h"
#include "GreedyAgent.h"
#include "Board.h"
#include <thread>
#include <fstream>
#include <iostream>

using namespace std;

AIPopulation::AIPopulation()
{
    m_matchCount = 100;
    m_size = 128;
    m_nextSeed = 0;
    m_threadCount = 8;
    m_survivorCount = m_size / 3;
    m_netLayerSize = {25, 40, 25, 1};
    m_genNumber = 0;
    m_createAntagonist = []() { return (AIAgent*) new GreedyAgent(0, 1); };
    reInitialize();
}

AIPopulation::~AIPopulation()
{
    for (AIAgent* a : m_antagonists) { delete a; }
    for (BrainAgent* a : m_agents) { delete a; }
    for (Brain* b : m_brains) { delete b; }
}

void AIPopulation::reInitialize()
{
    for (AIAgent* a : m_antagonists) { delete a; }
    for (BrainAgent* a : m_agents) { delete a; }
    for (Brain* b : m_brains) { delete b; }
    m_antagonists.clear();
    m_agents.clear();
    m_brains.clear();
    m_scores.clear();
    m_medianScoreHistory.clear();
    m_highestScoreHistory.clear();
    for (uint32_t i = 0; i < m_size; i++)
    {
        m_antagonists.push_back(m_createAntagonist());
        m_brains.push_back(new Brain(m_netLayerSize));
        m_agents.push_back(new BrainAgent(*m_brains[i]));
        m_scores.push_back(0.0f);
    }
}

void AIPopulation::sortByScore()
{
    for (uint32_t i = 0; i < m_size - 1; i++)
    {
        double largestValue = m_scores[i];
        uint32_t largestIndex = i;
        for (uint32_t j = i + 1; j < m_size; j++)
        {
            if (m_scores[j] > largestValue)
            {
                largestIndex = j;
                largestValue = m_scores[j];
            }
        }
        if (largestIndex != i)
        {
            Brain* tmp = m_brains[i];
            m_brains[i] = m_brains[largestIndex];
            m_brains[largestIndex] = tmp;
            m_scores[largestIndex] = m_scores[i];
            m_scores[i] = largestValue;
        }
    }
}

void AIPopulation::evaluateRange(uint32_t startIndex, uint32_t endIndex)
{
    for (uint32_t i = startIndex; i < endIndex; i++)
    {
        evaluateIndex(i);
    }
}

void AIPopulation::evaluateIndex(uint32_t index)
{
    AIAgent* ai = m_agents[index];
    AIAgent* antagonist = m_antagonists[index];
    Board board;
    vector<EncodedBoard> n;
    for (uint32_t i = 0; i < m_matchCount; i++)
    {
        board.decode(0);
        while (true)
        {
            if (board.getTurnNumber() > 1000)
            {
                break;
            }
            bool isPlayer1Turn = board.getTurnNumber() % 2 == 0;
            if (!isPlayer1Turn)
                board.invert();
            n = board.getNextLegalStates();
            if (n.size() > 0)
            {
                board.decode(n[((isPlayer1Turn) ? ai : antagonist)->selectPlay(n)]);
                if (!isPlayer1Turn)
                    board.invert();
            }
            else
            {
                if (!isPlayer1Turn)
                {
                    m_scores[index]++;
                    board.invert();
                }
                break;
            }
        }
    }
}

void AIPopulation::createNextGeneration()
{
    m_rng.seed(m_nextSeed);

    for(uint32_t i = m_survivorCount; i < m_size; i++)
    {
        *m_brains[i] = *m_brains[i % m_survivorCount];
    }

    for(uint32_t i = 0; i < m_size; i++)
    {
        m_brains[i]->randomize(m_rng);
        m_agents[i]->setBrain(*m_brains[i]);
        m_scores[i] = 0;
        m_antagonists[i]->seed(m_rng());
    }

    m_genNumber++;
}

void AIPopulation::evalGeneration()
{
    uint32_t actualThreadCount = (m_threadCount < m_size) ? m_threadCount : m_size;
    thread** threads = new thread*[actualThreadCount];
    for (uint32_t i = 0; i < actualThreadCount; i++)
    {
        uint32_t startIndex = m_size * (i) / actualThreadCount;
        uint32_t endIndex = m_size * (i + 1) / actualThreadCount;
        threads[i] = new thread(AIPopulation::evaluateRange, this, startIndex, endIndex);
    }
    for (uint32_t i = 0; i < actualThreadCount; i++)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete[] threads;
    sortByScore();
    m_medianScoreHistory.push_back(m_scores[m_size / 2]);
    m_highestScoreHistory.push_back(m_scores[0]);
    m_nextSeed = m_rng();
}

void AIPopulation::load(string fileName)
{
    ifstream in("output/" + fileName, ios_base::binary);
    uint32_t s;
    in.read((char*)&s, sizeof(uint32_t));
    if (s >> 31 == 0) // legacy file
    {
        m_size = s;
        m_genNumber = 0;
        m_survivorCount = m_size / 3;
        m_nextSeed = 0;
        reInitialize();
        for (uint32_t i = 0; i < m_size; i++)
        {
            m_brains[i]->load(in);
        }
        m_netLayerSize = m_brains[0]->getLayerSizes();
    }
    else
    {
        switch (s & 0xFF)
        {
        case 1: {
            uint32_t buffer[4];
            in.read((char*)buffer, 4 * sizeof(uint32_t));
            m_size = buffer[0];
            m_genNumber = buffer[1];
            m_nextSeed = buffer[2];
            m_survivorCount = buffer[3];
            reInitialize();
            for (uint32_t i = 0; i <= m_genNumber; i++)
            {
                double d[2];
                in.read((char*)d, 2 * sizeof(double));
                m_highestScoreHistory.push_back(d[0]);
                m_medianScoreHistory.push_back(d[1]);
            }
            for (uint32_t i = 0; i < m_size; i++)
            {
                in.read((char*)&m_scores[i], sizeof(double));
            }
            for (uint32_t i = 0; i < m_size; i++)
            {
                m_brains[i]->load(in);
            }
            m_netLayerSize = m_brains[0]->getLayerSizes();
            break; }
        default:
            cerr << "[Error] file " << fileName << " has unsupported version " << (s & 0xFF) << endl;
            break;
        }
    }
}

void AIPopulation::save(string fileName)
{
    ofstream o("output/" + fileName, ios_base::binary);
    uint32_t versionNumber = (1U << 31) | 1;
    o.write((char*)&versionNumber, sizeof(uint32_t));
    o.write((char*)&m_size, sizeof(uint32_t));
    o.write((char*)&m_genNumber, sizeof(uint32_t));
    o.write((char*)&m_nextSeed, sizeof(uint32_t));
    o.write((char*)&m_survivorCount, sizeof(uint32_t));
    for (uint32_t i = 0; i <= m_genNumber; i++)
    {
        o.write((char*)&m_highestScoreHistory[i], sizeof(double));
        o.write((char*)&m_medianScoreHistory[i], sizeof(double));
    }
    for (uint32_t i = 0; i < m_size; i++)
    {
        o.write((char*)&m_scores[i], sizeof(double));
    }
    for (uint32_t i = 0; i < m_size; i++)
    {
        m_brains[i]->save(o);
    }
}