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
#include "AIPopulation.h"

sf::Font* Board::s_font = nullptr;

using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode(1600, 900), "SFML works!");
    sf::Font font;
    font.loadFromFile("font/arial.ttf");
    Board::s_font = &font;

    sf::Text text;
    text.setFont(font);

    uint32_t matchCount = 512;

    AIPopulation population;
    population.setSeed(2);
    population.setSize(128);
    population.setMatchCount(matchCount);
    population.setSurvivorCount(population.getSize()/3);
    population.setNonMutatedCount(0);
    population.setThreadCount(8);
    population.setNetLayerSizes({25, 40, 20, 1});
    population.setAntagonistSpawner([](){return(AIAgent*)new GreedyAgent(0,1);});
    population.reInitialize();
    population.evalGenerationAsync();

    bool saveNextGen = false;
    bool loadNextGen = false;

    bool showNeuralNet = false;
    bool pause = false;
    bool singleAiStep = false;

    uint32_t gamesPlayed = 0;
    uint32_t gamesWon = 0;

    Board board;
    board.decode(0);

    Brain brain({25, 30, 15, 1});
    BrainAgent* brainAi = new BrainAgent(brain);
    AIAgent* ai = new GreedyAgent(0, 2);
    AIAgent* antagonist = new GreedyAgent(0, 1);

    Brain testBrain({1, 50, 50, 1});
    mt19937 rng;
    rng.seed(69);
    testBrain.randomizeAll(rng);
    vector<TrainingSample> trainingData;

    for(int i = 0; i <= 120; i++)
    {
        float f = i / 120.0f;
        trainingData.push_back({{f}, {4 * (f - 0.5f) * (f - 0.5f)}});
    }

    float stepSize = 0.0001f;
    float lastError = testBrain.backPropagation(trainingData, stepSize);

    LineShape line;

    bool testBackprop = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Up:
                    matchCount *= 2;
                    break;
                case sf::Keyboard::Down:
                    matchCount /= 2;
                    break;
                case sf::Keyboard::S:
                    saveNextGen = true;
                    break;
                case sf::Keyboard::L:
                    loadNextGen = true;
                    break;
                case sf::Keyboard::P:
                    pause = !pause;
                    board.decode(0);
                    break;
                case sf::Keyboard::N:
                    showNeuralNet = !showNeuralNet;
                    break;
                case sf::Keyboard::C:
                    singleAiStep = true;
                    break;
                case sf::Keyboard::I:
                    board.decode(Board::invert(board.encode()));
                    break;
                case sf::Keyboard::R:
                    gamesPlayed = 0;
                    gamesWon = 0;
                    break;
                default:
                    break;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {

            }
        }

        if (!pause || singleAiStep)
        {
            if (board.getTurnNumber() > 1000)
            {
                gamesPlayed++;
                board.decode(0);
            }
            else
            {
                bool isPlayer1Turn = board.getTurnNumber() % 2 == 0;
                if (!isPlayer1Turn)
                    board.invert();
                vector<EncodedBoard> n;
                Board::getNextLegalStates(board.encode(), n);
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
                        gamesWon++;
                        board.invert();
                    }
                    gamesPlayed++;
                    board.decode(0);
                }
            }
            singleAiStep = false;
        }

        if (population.isDoneEvaluatingGeneration())
        {
            population.finalizeEvaluation();

            vector<double> scores = population.getScores();
            uint32_t matches = population.getMatchCount();
            cout << "gen" << population.getGenNumber() << ":\n";
            cout << " + " << (100 * scores[0] / matches) << "% win rate\n";
            cout << " ~ " << (100 * scores[population.getSize() / 2] / matches) << "% win rate\n";
            cout << " - " << (100 * scores[population.getSize() - 1] / matches) << "% win rate\n";

            population.copyBestBrain(&brain);
            brainAi->setBrain(brain);
            //gamesPlayed = 0;
            //gamesWon = 0;

            if (saveNextGen)
            {
                population.autoSave();
                saveNextGen = false;
            }

            if (loadNextGen)
            {
                population.load("gen");
                loadNextGen = false;
            }

            population.setMatchCount(matchCount);
            population.createNextGeneration();
            population.evalGenerationAsync();
        }

        stringstream ss;
        double winRate = (gamesPlayed > 0) ? (100.0 * gamesWon / gamesPlayed) : 0;
        ss << ai->getName() << " vs. " << antagonist->getName() << " has a " << winRate << "% win rate";
        text.setPosition({50, 800});
        text.setString(ss.str());

        window.clear();
        window.draw(text);
        board.draw(window, {50, 50});
        if (testBackprop)
        {
            LineShape redLine;
            redLine.setColor(sf::Color::Red);
            float lf1 = trainingData[0].input[0];
            float lf2 = trainingData[0].output[0];
            for (uint32_t i = 1; i < trainingData.size(); i++)
            {
                float f1 = trainingData[i].input[0];
                float f2 = trainingData[i].output[0];
                redLine.setAll({850.0f + lf1 * 600, 300 + lf2 * 500}, {850.0f + f1 * 600, 300 + f2 * 500}, 1);
                redLine.draw(window);
                lf1 = f1;
                lf2 = f2;
            }
            float input = 0.0f;
            float lastOutput = 0;
            testBrain.setInputNeurons(&input);
            testBrain.think();
            testBrain.getOutputNeurons(&lastOutput);
            //testBrain.draw(window, {850, 50});
            for (int i = 1; i <= 120; i++)
            {
                float input = i / 120.0f;
                float output = 0;
                testBrain.setInputNeurons(&input);
                testBrain.think();
                testBrain.getOutputNeurons(&output);
                line.setAll({850.0f + (i - 1) * 5, 300 + lastOutput * 500}, {850.0f + i * 5, 300 + output * 500}, 1);
                line.draw(window);
                lastOutput = output;
            }
            Brain b;
            b = testBrain;
            float e = testBrain.backPropagation(trainingData, stepSize);
            if (e > lastError)
            {
                //testBrain = b;
                stepSize /= 4;
            }
            else
            {
                stepSize *= 1.01;
            }
            lastError = e;
            if (stepSize < 0.0001f)
                stepSize = 0.0001f;
        }
        else
        {
            if (showNeuralNet)
            {
                brain.setInputNeurons(board.encode());
                brain.think();
                brain.draw(window, {800, 50});
            }
            else
            {
                population.draw(window, {800, 50}, {750, 800});
            }
        }
        window.display();
    }

    population.finalizeEvaluation();

    return 0;
}
