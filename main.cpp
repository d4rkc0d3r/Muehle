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

    uint32_t matchCount = 128;

    AIPopulation population;
    population.setSeed(0);
    population.setSize(128);
    population.setMatchCount(matchCount);
    population.setSurvivorCount(population.getSize()/3);
    population.setNonMutatedCount(0);
    population.setThreadCount(8);
    population.setNetLayerSizes({25, 40, 25, 1});
    population.setAntagonistSpawner([](){return(AIAgent*)new GreedyAgent(0,1);});
    population.reInitialize();
    population.evalGenerationAsync();

    bool saveNextGen = false;
    bool loadNextGen = false;

    uint32_t gamesPlayed = 0;
    uint32_t gamesWon = 0;

    Board board;
    board.decode(0);

    Brain brain({25, 30, 15, 1});
    BrainAgent* ai = new BrainAgent(brain);
    AIAgent* antagonist = new GreedyAgent(0, 1);

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
                default:
                    break;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {

            }
        }

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
            vector<EncodedBoard> n = board.getNextLegalStates();
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
            ai->setBrain(brain);
            board.decode(0);
            gamesPlayed = 0;
            gamesWon = 0;

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
        if (gamesPlayed > 0)
        {
            ss << ai->getName() << " vs. " << antagonist->getName() << " has a " << (100.0 * gamesWon / gamesPlayed) << "% win rate";
        }
        text.setPosition({50, 800});
        text.setString(ss.str());

        window.clear();
        window.draw(text);
        board.draw(window, {50, 50});
        population.draw(window, {800, 50}, {750, 800});
        window.display();
    }

    population.finalizeEvaluation();

    return 0;
}
