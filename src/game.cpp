#include "game.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

Game::Game(std::string title, sf::Vector2u initialWindowSize, unsigned int frameRateLimit) :
m_window(sf::VideoMode(initialWindowSize), title),
m_shape(100.f)
{
    m_window.setMinimumSize(initialWindowSize);
    m_window.setFramerateLimit(frameRateLimit);

    if (!ImGui::SFML::Init(m_window))
    {
        throw std::runtime_error("Failed to initialize ImGui SFML");
    }

    m_shape.setFillColor(sf::Color::Green);
}

Game::~Game()
{
    ImGui::SFML::Shutdown();
}

void Game::run()
{
    m_clock.restart();

    while (m_window.isOpen())
    {
        float deltaTime = m_clock.restart().asSeconds();

        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents()
{
    m_window.handleEvents([this](const sf::Event::Closed&) { this->handleWindowClose(); },
                          [this](const sf::Event::Resized& event) { this->handleWindowResize(event); },
                          [this](const auto& event) { ImGui::SFML::ProcessEvent(m_window, event); });
}

void Game::update(float deltaTime)
{
    ImGui::SFML::Update(m_window, sf::seconds(deltaTime));

    ImGui::ShowDemoWindow();

    ImGui::Begin("Hello, world!");
    ImGui::Button("Look at this pretty button");
    ImGui::End();
}

void Game::render()
{
    m_window.clear(sf::Color::Black);

    m_window.draw(m_shape);
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::handleWindowClose()
{
    m_window.close();
}

void Game::handleWindowResize(const sf::Event::Resized& resized)
{
    sf::Vector2u  windowSize(resized.size.x, resized.size.y);
    sf::FloatRect visibleArea({0.f, 0.f}, static_cast<sf::Vector2f>(windowSize));

    m_window.setView(sf::View(visibleArea));

    sf::Vector2f newSize(static_cast<float>(resized.size.x), static_cast<float>(resized.size.y));
}