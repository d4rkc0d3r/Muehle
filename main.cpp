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

    AIPopulation population;
    population.setSeed(0);
    population.setSize(96);
    population.setMatchCount(200);
    population.setSurvivorCount(population.getSize()/3);
    population.setThreadCount(8);
    population.setNetLayerSizes({25, 30, 15, 1});
    population.setAntagonistSpawner([](){return(AIAgent*)new GreedyAgent(0,1);});
    population.reInitialize();
    population.evalGenerationAsync();

    int frameCounter = 0;

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

            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {

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

            population.createNextGeneration();
            population.evalGenerationAsync();
        }

        stringstream ss;
        ss << frameCounter++;
        text.setPosition({100, 800});
        text.setString(ss.str());

        window.clear();
        window.draw(text);
        population.draw(window, {800, 50}, {750, 800});
        window.display();
    }

    population.finalizeEvaluation();

    return 0;
}
