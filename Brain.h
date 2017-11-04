#ifndef BRAIN_H
#define BRAIN_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "Board.h"
#include <random>

class Brain
{
    public:
        Brain();
        Brain(const std::vector<std::size_t>& layerSize);
        virtual ~Brain();

        void init(const std::vector<std::size_t>& layerSize);

        void draw(sf::RenderTarget& renderTarget, const sf::Vector2<float>& pos);

        void setInputNeurons(float* input);
        void setInputNeurons(EncodedBoard b);
        void think();
        void getOutputNeurons(float* output);

        Brain& operator=(const Brain& b);

        void randomizeAll(std::mt19937& rng);
        void randomize(std::mt19937& rng);

        void load(std::istream& input);
        void save(std::ostream& output);
    protected:

    private:
        std::vector<std::size_t> m_layerSize;

        std::size_t m_neuronCount;
        std::size_t m_biasCount;
        std::size_t m_connectionCount;

        float* m_neurons;
        float* m_bias;
        float* m_weights;

        void initArrays();
        void cleanArrays();
};

#endif // BRAIN_H
