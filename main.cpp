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

    std::vector<std::size_t> netSize {25, 40, 25, 1};

    std::mt19937 rng;
    rng.seed(0);

    const uint32_t POP_SIZE = 50;
    const uint32_t MATCH_COUNT = 100;

    uint32_t genNumber = 0;
    uint32_t evalIndex = 0;

    high_resolution_clock::time_point tStartGen = high_resolution_clock::now();

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
                if (event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    std::cout << (char)('A' + event.key.code) << std::endl;
                else
                    std::cout << event.key.code << std::endl;
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
            std::cout << "gen" << genNumber << " finished in " << time_span.count() << "s\n";
            std::cout << " + " << (100 * scores[0] / MATCH_COUNT) << "% win rate\n";
            std::cout << " ~ " << (100 * scores[POP_SIZE / 2] / MATCH_COUNT) << "% win rate\n";
            std::cout << " - " << (100 * scores[POP_SIZE - 1] / MATCH_COUNT) << "% win rate\n";
            std::cout << std::endl;

            for(uint32_t i = POP_SIZE / 2; i < POP_SIZE; i++)
            {
                *brains[i] = *brains[i - POP_SIZE / 2];
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
        else
        {
            EncodedBoard ret;
            evaluate(&ais[evalIndex], &antagonist[evalIndex], &scores[evalIndex], MATCH_COUNT, &ret);
            board.decode(ret);
            evalIndex++;
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
