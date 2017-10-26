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

int main()
{
    using namespace std::chrono;

    sf::RenderWindow window(sf::VideoMode(1600, 900), "SFML works!");
    sf::Font font;
    font.loadFromFile("font/arial.ttf");
    Board::s_font = &font;

    sf::Text text;
    text.setFont(font);

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
