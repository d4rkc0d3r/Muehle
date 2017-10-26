#ifndef BRAIN_H
#define BRAIN_H

#include <vector>
#include <SFML/Graphics.hpp>

class Brain
{
    public:
        Brain(const std::vector<std::size_t>& layerSize);
        virtual ~Brain();

        void draw(sf::RenderTarget& renderTarget, const sf::Vector2<float>& pos);

        void setInputNeurons(float* input);
        void think();
        void getOutputNeurons(float* output);

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
};

#endif // BRAIN_H
