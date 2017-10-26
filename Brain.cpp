#include "Brain.h"
#include "LineShape.h"
#include <iomanip>
#include <sstream>
#include <cstring>

sf::Font* font = nullptr;

Brain::Brain(const std::vector<std::size_t>& layerSize)
{
    m_layerSize = layerSize;
    initArrays();
}

void Brain::initArrays()
{
    m_neuronCount = m_layerSize[0];
    m_connectionCount = 0;
    for (std::size_t i = 1; i < m_layerSize.size(); i++)
    {
        m_connectionCount += m_layerSize[i - 1] * m_layerSize[i];
        m_neuronCount += m_layerSize[i];
    }
    m_biasCount = m_neuronCount - m_layerSize[0] - m_layerSize[m_layerSize.size() - 1];
    m_neurons = new float[m_neuronCount];
    m_bias = new float[m_biasCount];
    m_weights = new float[m_connectionCount];
    for(int i = 0; i < m_connectionCount; i++)
    {
        m_weights[i] = 1;
    }
    for(int i = 0; i < m_biasCount; i++)
    {
        m_bias[i] = 2 - i;
    }
}

void Brain::setInputNeurons(float* input)
{
    std::memcpy(m_neurons, input, sizeof(float) * m_layerSize[0]);
}

void Brain::getOutputNeurons(float* output)
{
    std::size_t lastLayerSize = m_layerSize[m_layerSize.size() - 1];
    std::memcpy(output, m_neurons + (m_neuronCount - lastLayerSize), sizeof(float) * lastLayerSize);
}

void Brain::draw(sf::RenderTarget& renderTarget, const sf::Vector2<float>& pos)
{
    float neuronRadius = 25;
    float gapX = 150;
    float gapY = 70;

    LineShape line;
    sf::CircleShape circle(neuronRadius, 32);
    sf::Text text;
    if(font == nullptr)
    {
        font = new sf::Font();
        font->loadFromFile("font/arial.ttf");
    }
    text.setFont(*font);
    text.setCharacterSize(neuronRadius - 2);
    text.setColor(sf::Color::Black);

    std::size_t maxNeuronCount = 0;
    for(std::size_t i = 0; i < m_layerSize.size(); i++)
    {
        if(m_layerSize[i] > maxNeuronCount)
            maxNeuronCount = m_layerSize[i];
    }

    std::size_t offset = 0;
    for (std::size_t layerId = 0; layerId < m_layerSize.size() - 1; layerId++)
    {
        std::size_t layerSize = m_layerSize[layerId];
        std::size_t nextLayerSize = m_layerSize[layerId + 1];
        float yOffset = (maxNeuronCount - layerSize) / 2.0f * gapY;
        float nextYOffset = (maxNeuronCount - nextLayerSize) / 2.0f * gapY;
        for (std::size_t sourceId = 0; sourceId < layerSize; sourceId++)
        {
            sf::Vector2<float> sourcePos = {
                layerId * gapX + neuronRadius + pos.x,
                sourceId * gapY + yOffset + neuronRadius + pos.y};
            for (std::size_t targetId = 0; targetId < nextLayerSize; targetId++)
            {
                sf::Vector2<float> targetPos = {
                    (layerId + 1) * gapX + neuronRadius + pos.x,
                    targetId * gapY + nextYOffset + neuronRadius + pos.y};
                line.setAll(sourcePos, targetPos, 1);
                line.draw(renderTarget);
            }
        }
        offset += layerSize;
    }

    offset = 0;
    for (std::size_t layerId = 0; layerId < m_layerSize.size(); layerId++)
    {
        std::size_t layerSize = m_layerSize[layerId];
        float yOffset = (maxNeuronCount - layerSize) / 2.0f * gapY;
        for (std::size_t n = 0; n < layerSize; n++)
        {
            circle.setPosition({layerId * gapX + pos.x, n * gapY + yOffset + pos.y});
            renderTarget.draw(circle);
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << m_neurons[offset + n];
            text.setPosition({layerId * gapX + pos.x, n * gapY + yOffset + neuronRadius / 2.0f - 3 + pos.y});
            text.setString(stream.str());
            renderTarget.draw(text);
        }
        offset += layerSize;
    }
}

float activationFunction(float f)
{
    return (f < 0) ? 0 : f;
}

void Brain::think()
{
    std::size_t offset = 0;
    std::size_t wOffset = 0;
    for (std::size_t layerId = 0; layerId < m_layerSize.size() - 1; layerId++)
    {
        std::size_t layerSize = m_layerSize[layerId];
        std::size_t tOffset = offset + layerSize;
        std::size_t nextLayerSize = m_layerSize[layerId + 1];
        for (std::size_t targetId = 0; targetId < nextLayerSize; targetId++)
        {
            m_neurons[tOffset + targetId] = 0;
        }
        for (std::size_t sourceId = 0; sourceId < layerSize; sourceId++)
        {
            if (layerId > 0)
            {
                m_neurons[offset + sourceId] = activationFunction(
                        m_neurons[offset + sourceId] + m_bias[offset + sourceId - m_layerSize[0]]);
            }
            float currentNeuron = m_neurons[offset + sourceId];
            if (currentNeuron != 0)
            {
                for (std::size_t targetId = 0; targetId < nextLayerSize; targetId++)
                {
                    m_neurons[tOffset + targetId] += currentNeuron * m_weights[wOffset + targetId];
                }
            }
            wOffset += nextLayerSize;
        }
        offset += layerSize;
    }
}

Brain::~Brain()
{
    delete [] m_neurons;
    delete [] m_bias;
    delete [] m_weights;
}
