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
#include <sstream>
#include "RandomAgent.h"
#include "GreedyAgent.h"
#include "PickFirstAgent.h"

sf::Font* Board::s_font = nullptr;
std::vector<char> Board::s_mills;
std::vector<std::vector<char>> Board::s_neighbors;

void initMills()
{
    std::vector<char>* m = &Board::s_mills;

    m->push_back(1);
    m->push_back(2);
    m->push_back(9);
    m->push_back(21);
    m->push_back(0);
    m->push_back(2);
    m->push_back(4);
    m->push_back(7);
    m->push_back(0);
    m->push_back(1);
    m->push_back(14);
    m->push_back(23);

    m->push_back(4);
    m->push_back(5);
    m->push_back(10);
    m->push_back(18);
    m->push_back(3);
    m->push_back(5);
    m->push_back(1);
    m->push_back(7);
    m->push_back(3);
    m->push_back(4);
    m->push_back(13);
    m->push_back(20);

    m->push_back(7);
    m->push_back(8);
    m->push_back(11);
    m->push_back(15);
    m->push_back(6);
    m->push_back(8);
    m->push_back(1);
    m->push_back(4);
    m->push_back(6);
    m->push_back(7);
    m->push_back(12);
    m->push_back(17);

    m->push_back(10);
    m->push_back(11);
    m->push_back(0);
    m->push_back(21);
    m->push_back(9);
    m->push_back(11);
    m->push_back(3);
    m->push_back(18);
    m->push_back(9);
    m->push_back(10);
    m->push_back(6);
    m->push_back(15);

    m->push_back(13);
    m->push_back(14);
    m->push_back(8);
    m->push_back(17);
    m->push_back(12);
    m->push_back(14);
    m->push_back(5);
    m->push_back(20);
    m->push_back(12);
    m->push_back(13);
    m->push_back(2);
    m->push_back(23);

    m->push_back(16);
    m->push_back(17);
    m->push_back(6);
    m->push_back(11);
    m->push_back(15);
    m->push_back(17);
    m->push_back(19);
    m->push_back(22);
    m->push_back(15);
    m->push_back(16);
    m->push_back(8);
    m->push_back(12);

    m->push_back(19);
    m->push_back(20);
    m->push_back(3);
    m->push_back(10);
    m->push_back(18);
    m->push_back(20);
    m->push_back(16);
    m->push_back(22);
    m->push_back(18);
    m->push_back(19);
    m->push_back(5);
    m->push_back(13);

    m->push_back(22);
    m->push_back(23);
    m->push_back(0);
    m->push_back(9);
    m->push_back(21);
    m->push_back(23);
    m->push_back(16);
    m->push_back(19);
    m->push_back(21);
    m->push_back(22);
    m->push_back(2);
    m->push_back(14);
}

void initNeighbors()
{
    std::vector<std::vector<char>>* n = &Board::s_neighbors;
    n->push_back({1, 9});
    n->push_back({0, 2, 4});
    n->push_back({1, 14});

    n->push_back({4, 10});
    n->push_back({1, 3, 5, 7});
    n->push_back({4, 13});

    n->push_back({7, 11});
    n->push_back({4, 6, 8});
    n->push_back({7, 12});

    n->push_back({0, 10, 21});
    n->push_back({3, 9, 11, 18});
    n->push_back({6, 10, 15});

    n->push_back({8, 13, 17});
    n->push_back({5, 12, 14, 20});
    n->push_back({2, 13, 23});

    n->push_back({11, 16});
    n->push_back({15, 17, 19});
    n->push_back({12, 16});

    n->push_back({10, 19});
    n->push_back({16, 18, 20, 22});
    n->push_back({13, 19});

    n->push_back({9, 22});
    n->push_back({19, 21, 23});
    n->push_back({14, 22});
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

    initMills();
    initNeighbors();

    AIAgent* ai1 = new GreedyAgent(0, 1);
    AIAgent* ai2 = new RandomAgent(2);
    int ai1wins = 0;
    int ai2wins = 0;

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

        for (int i = 0; i < 10; i++)
        {
            board.decode(0);
            while (true)
            {
                bool isPlayer1Turn = board.getTurnNumber() % 2 == 0;
                if (!isPlayer1Turn)
                    board.invert();
                std::vector<EncodedBoard> n = board.getNextLegalStates();
                if (n.size() > 0)
                {
                    board.decode(n[((isPlayer1Turn) ? ai1 : ai2)->selectPlay(n)]);
                    if (!isPlayer1Turn)
                        board.invert();
                }
                else
                {
                    if(isPlayer1Turn)
                    {
                        ai2wins++;
                    }
                    else
                    {
                        ai1wins++;
                        board.invert();
                    }
                    break;
                }
            }
        }

        std::stringstream ss;
        ss << ai1->getName() << " vs. " << ai2->getName() << ": ";
        ss << std::fixed << std::setprecision(2) << (100.0 * ai1wins / (ai1wins + ai2wins)) << "% win rate";
        text.setPosition({100, 800});
        text.setString(ss.str());

        window.clear();
        board.draw(window, {0, 20});
        window.draw(text);
        window.display();
    }

    return 0;
}
