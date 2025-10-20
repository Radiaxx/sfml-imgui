#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

#include "app.hpp"

App::App(const Config& config) :
m_window(sf::VideoMode(config.initialWindowSize), config.title),
m_uiManager(sf::View(static_cast<sf::FloatRect>(m_window.getViewport(m_window.getDefaultView()))))
{
    m_window.setMinimumSize(config.minimumWindowSize);
    m_window.setSize(config.initialWindowSize);
    m_window.setFramerateLimit(config.frameRateLimit);

    if (!ImGui::SFML::Init(m_window))
    {
        throw std::runtime_error("Failed to initialize ImGui SFML");
    }
}

App::~App()
{
    ImGui::SFML::Shutdown();
}

void App::run()
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

void App::handleEvents()
{
    m_window.handleEvents([this](const sf::Event::Closed&) { this->handleWindowClose(); },
                          [this](const sf::Event::Resized& event) { this->handleWindowResize(event); },
                          [this](const sf::Event::MouseWheelScrolled& event)
                          {
                              m_viewControls.handleMouseWheelScrolled(event, m_window);
                              m_cellTooltip.handleMouseWheelScrolled(event);
                              ImGui::SFML::ProcessEvent(m_window, event);
                          },
                          [this](const sf::Event::MouseButtonPressed& event)
                          {
                              m_viewControls.handleMouseButtonPressed(event);
                              m_cellTooltip.handleMouseButtonPressed(event, m_heatmap, m_window);
                              ImGui::SFML::ProcessEvent(m_window, event);
                          },
                          [this](const sf::Event::MouseButtonReleased& event)
                          {
                              m_viewControls.handleMouseButtonReleased(event);
                              ImGui::SFML::ProcessEvent(m_window, event);
                          },
                          [this](const sf::Event::MouseMoved& event)
                          {
                              m_viewControls.handleMouseMoved(event, m_window);
                              ImGui::SFML::ProcessEvent(m_window, event);
                          },
                          [this](const auto& event) { ImGui::SFML::ProcessEvent(m_window, event); });
}

void App::update(float deltaTime)
{
    ImGui::SFML::Update(m_window, sf::seconds(deltaTime));
}

void App::render()
{
    m_window.clear(sf::Color::Black);

    m_heatmap.draw(m_window.getView());
    m_uiManager.draw(m_heatmap, m_geoData, m_gridOverlay);

    if (m_heatmap.getAscData())
    {
        m_heatmap.getHeatmapShader().setUniform("uClampMin", m_heatmap.getCurrentClampMin());
        m_heatmap.getHeatmapShader().setUniform("uClampMax", m_heatmap.getCurrentClampMax());
        m_window.draw(m_heatmap.getHeatmapSprite(), &m_heatmap.getHeatmapShader());
    }

    m_gridOverlay.draw(m_heatmap, m_window);
    m_cellTooltip.draw(m_heatmap, m_window);

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void App::handleWindowClose()
{
    m_window.close();
}

void App::handleWindowResize(const sf::Event::Resized& resized)
{
    sf::Vector2u  windowSize(resized.size.x, resized.size.y);
    sf::FloatRect visibleArea({0.f, 0.f}, static_cast<sf::Vector2f>(windowSize));

    const sf::View view = sf::View(visibleArea);
    m_window.setView(view);
    m_uiManager.setView(view);
    m_heatmap.updateHeatmapView(view);
}