#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <ctime>
#include <chrono>
#include "LineShape.h"
#include <cmath>
#include "Brain.h"
#include "Board.h"
#include <random>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "RandomAgent.h"
#include "GreedyAgent.h"
#include "PickFirstAgent.h"
#include "BrainAgent.h"
#include <thread>

sf::Font* Board::s_font = nullptr;

void sortBrainsAndScores(Brain** brains, double* scores, uint32_t POP_SIZE)
{
    for (uint32_t i = 0; i < POP_SIZE - 1; i++)
    {
        double largestValue = scores[i];
        uint32_t largestIndex = i;
        for (uint32_t j = i + 1; j < POP_SIZE; j++)
        {
            if (scores[j] > largestValue)
            {
                largestIndex = j;
                largestValue = scores[j];
            }
        }
        if (largestIndex != i)
        {
            Brain* tmp = brains[i];
            brains[i] = brains[largestIndex];
            brains[largestIndex] = tmp;
            scores[largestIndex] = scores[i];
            scores[i] = largestValue;
        }
    }
}

void saveGeneration(Brain** brains, uint32_t POP_SIZE, std::string fileName)
{
    std::ofstream o("output/" + fileName, std::ios_base::binary);
    o.write((char*)&POP_SIZE, sizeof(uint32_t));
    for (uint32_t i = 0; i < POP_SIZE; i++)
    {
        brains[i]->save(o);
    }
}

void loadGeneration(Brain** brains, uint32_t POP_SIZE)
{
    std::ifstream in("output/gen", std::ios_base::binary);
    uint32_t s;
    in.read((char*)&s, sizeof(uint32_t));
    if (s != POP_SIZE)
    {
        std::cerr << "POP_SIZE(" << POP_SIZE << ") != " << s << std::endl;
        return;
    }
    for (uint32_t i = 0; i < POP_SIZE; i++)
    {
        brains[i]->load(in);
    }
}

void evaluate(AIAgent* ai, AIAgent* antagonist, double* score, uint32_t MATCH_COUNT, EncodedBoard* output)
{
    Board board;
    for (uint32_t i = 0; i < MATCH_COUNT; i++)
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
            std::vector<EncodedBoard> n = board.getNextLegalStates();
            if (n.size() > 0)
            {
                board.decode(n[(isPlayer1Turn) ? ai->selectPlay(n) : antagonist->selectPlay(n)]);
                if (!isPlayer1Turn)
                    board.invert();
            }
            else
            {
                if (!isPlayer1Turn)
                {
                    *score = *score + 1;
                    board.invert();
                }
                break;
            }
        }
    }
    *output = board.encode();
}

int main()
{
    using namespace std::chrono;

    sf::RenderWindow window(sf::VideoMode(1600, 900), "SFML works!");
    sf::Font font;
    font.loadFromFile("font/arial.ttf");
    Board::s_font = &font;

    sf::Text text;
    text.setFont(font);

    std::vector<std::size_t> netSize {25, 16, 8, 1};

    std::mt19937 rng;
    rng.seed(2);

    const uint32_t POP_SIZE = 100;
    const uint32_t MATCH_COUNT = 250;
    const uint32_t THREAD_COUNT = 10;
    const uint32_t SURVIVORS = POP_SIZE / 5;

    uint32_t genNumber = 0;
    uint32_t evalIndex = 0;

    high_resolution_clock::time_point tStartGen = high_resolution_clock::now();

    double lastMedianScore = 0;

    Brain** brains = new Brain*[POP_SIZE];
    double* scores = new double[POP_SIZE];
    BrainAgent* ais = new BrainAgent[POP_SIZE];
    GreedyAgent* antagonist = new GreedyAgent[POP_SIZE];

    for(uint32_t i = 0; i < POP_SIZE; i++)
    {
        brains[i] = new Brain(netSize);
        brains[i]->randomizeAll(rng);
        ais[i].setBrain(*brains[i]);
        scores[i] = 0;
        antagonist[i].init(0, 1);
    }

    Board board;
    board.decode(0);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::S)
                {
                    std::stringstream ss;
                    for (uint32_t i = 0; i < netSize.size() - 1; i++)
                    {
                        ss << netSize[i] << "-";
                    }
                    ss << netSize[netSize.size() - 1];
                    ss << "_" << lastMedianScore << "%.gen";
                    saveGeneration(brains, POP_SIZE, ss.str());
                }
                else if (event.key.code == sf::Keyboard::L)
                {
                    loadGeneration(brains, POP_SIZE);
                    for(uint32_t i = 0; i < POP_SIZE; i++)
                    {
                        ais[i].setBrain(*brains[i]);
                        scores[i] = 0;
                    }
                    evalIndex = 0;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if(event.mouseButton.button == sf::Mouse::Left)
                    std::cout << "x: " << event.mouseButton.x << " | y: " << event.mouseButton.y << std::endl;
            }
        }


        if (evalIndex >= POP_SIZE)
        {
            duration<double> time_span = duration_cast<duration<double>>(high_resolution_clock::now() - tStartGen);
            sortBrainsAndScores(brains, scores, POP_SIZE);

            lastMedianScore = (100 * scores[POP_SIZE / 2] / MATCH_COUNT);

            std::cout << "gen" << genNumber << " finished in " << time_span.count() << "s\n";
            std::cout << " + " << (100 * scores[0] / MATCH_COUNT) << "% win rate\n";
            std::cout << " ~ " << (100 * scores[POP_SIZE / 2] / MATCH_COUNT) << "% win rate\n";
            std::cout << " - " << (100 * scores[POP_SIZE - 1] / MATCH_COUNT) << "% win rate\n";
            std::cout << std::endl;

            for(uint32_t i = SURVIVORS; i < POP_SIZE; i++)
            {
                *brains[i] = *brains[i % SURVIVORS];
            }

            for(uint32_t i = 0; i < POP_SIZE; i++)
            {
                brains[i]->randomize(rng);
                ais[i].setBrain(*brains[i]);
                scores[i] = 0;
            }

            genNumber++;
            evalIndex = 0;
            tStartGen = high_resolution_clock::now();
        }

        {
            EncodedBoard ret;
            EncodedBoard trash;
            uint32_t actualThreadCount = (THREAD_COUNT < POP_SIZE - evalIndex) ? THREAD_COUNT : POP_SIZE - evalIndex;
            std::thread** threads = new std::thread*[actualThreadCount];
            for (uint32_t i = 0; i < actualThreadCount; i++)
            {
                EncodedBoard* b = (i == actualThreadCount - 1) ? &ret : &trash;
                uint32_t index = evalIndex + i;
                threads[i] = new std::thread(evaluate, &ais[index], &antagonist[index], &scores[index], MATCH_COUNT, b);
            }
            for (uint32_t i = 0; i < actualThreadCount; i++)
            {
                threads[i]->join();
                delete threads[i];
            }
            delete[] threads;
            board.decode(ret);
            evalIndex += actualThreadCount;
        }

        std::stringstream ss;
        if (evalIndex > 0)
        {
            ss << "gen" << genNumber << ", eval index " << evalIndex - 1 << " got a win rate of ";
            ss << std::fixed << std::setprecision(2) << (100 * scores[evalIndex - 1] / MATCH_COUNT) << "%";
        }
        text.setPosition({100, 800});
        text.setString(ss.str());

        window.clear();
        board.draw(window, {0, 20});
        window.draw(text);
        window.display();
    }

    return 0;
}
