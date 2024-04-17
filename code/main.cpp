#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>

using namespace sf;
using namespace std;

const unsigned int DEFAULT_MAX_ITER = 64;
const float DEFAULT_BASE_WIDTH = 4.0;
const float DEFAULT_BASE_HEIGHT = 4.0;
const float DEFAULT_BASE_ZOOM = 0.5;

const Color DEFAULT_TEXT_COLOR = Color::White;
const string DEFAULT_TEXT_FILE = "KOMIKAP_.ttf";
const string DEFAULT_PRESENTATION_NAME = "Mandelbrot Set\n";
const string DEFAULT_WINDOW_NAME = "Mandelbrot Set Visualizer";

const int DEFAULT_ZOOM_COUNT = 0;
const Vector2f DEFAULT_PLANE_CENTER = { 0, 0 };
const int DEFAULT_CHARACTER_SIZE = 16;
const complex<double> DEFAULT_Z_VALUE = { 0, 0 };
const float DEFAULT_ABS_THRESHOLD = 2.0;
const int MAX_ITER_REGIONS = 5;

const int MAX_RGB_VALUE = 255;
const int HALF_RGB_VALUE = 128;
const int NO_RGB_VALUE = 0;

class ComplexPlane : public Drawable {
public:
    ComplexPlane(
        const string& windowName = DEFAULT_WINDOW_NAME,
        int pixelWidth = VideoMode::getDesktopMode().width / 2,
        int pixelHeight = VideoMode::getDesktopMode().height / 2,
        float baseWidth = DEFAULT_BASE_WIDTH,
        float baseHeight = DEFAULT_BASE_HEIGHT,
        const string& presentationName = DEFAULT_PRESENTATION_NAME,
        const string& textFile = DEFAULT_TEXT_FILE,
        const Color& textColor = DEFAULT_TEXT_COLOR,
        float baseZoom = DEFAULT_BASE_ZOOM,
        unsigned int maxIter = DEFAULT_MAX_ITER
    );

    void initialize();
    void run();

    virtual void draw(RenderTarget& target, RenderStates states) const override;

private:
    RenderWindow m_window;
    VertexArray m_vArray;
    int m_pixelWidth;
    int m_pixelHeight;
    float m_aspectRatio;
    Vector2f m_plane_center;
    Vector2f m_plane_size;
    int m_zoomCount;
    Vector2f m_mouseLocation;
    unsigned int m_maxIter;
    float m_baseWidth;
    float m_baseHeight;
    float m_baseZoom;
    Color m_textColor;
    string m_textFile;
    string m_presentationName;

    size_t countIterations(Vector2f coord);
    void iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b);
    Vector2f mapPixelToCoords(Vector2i mousePixel);
    void handleEvent(Event& event, bool& update);
    void handleMouseClick(Event::MouseButtonEvent& mouseEvent);
    void handleKeyboard(Event::KeyEvent& keyEvent);
    void handleException(const std::exception& e);
    void updateRender();
    void zoomIn();
    void zoomOut();
    void setCenter(Vector2i mousePixel);
    void setMouseLocation(Vector2i mousePixel);
    void loadText(Text& text);
};

ComplexPlane::ComplexPlane(
    const string& windowName,
    int pixelWidth,
    int pixelHeight,
    float baseWidth,
    float baseHeight,
    const string& presentationName,
    const string& textFile,
    const Color& textColor,
    float baseZoom,
    unsigned int maxIter
) : 
    m_window(VideoMode(pixelWidth, pixelHeight), windowName),
    m_vArray(Points, pixelWidth * pixelHeight),
    m_pixelWidth(pixelWidth),
    m_pixelHeight(pixelHeight),
    m_aspectRatio(static_cast<float>(pixelHeight) / pixelWidth),
    m_baseWidth(baseWidth),
    m_baseHeight(baseHeight),
    m_baseZoom(baseZoom),
    m_maxIter(maxIter),
    m_zoomCount(DEFAULT_ZOOM_COUNT),
    m_plane_center(DEFAULT_PLANE_CENTER),
    m_plane_size(Vector2f(m_baseWidth, m_baseHeight * m_aspectRatio)),
    m_presentationName(presentationName),
    m_textFile(textFile),
    m_textColor(textColor)
{}

void ComplexPlane::initialize() {
    Text text;
    Font font;
    if (!font.loadFromFile(m_textFile)) {
        cerr << "Error loading font" << endl;
        return;
    }
    text.setFont(font);
    text.setCharacterSize(DEFAULT_CHARACTER_SIZE);
    text.setFillColor(m_textColor);
    m_window.clear();
    m_window.draw(*this);
    m_window.draw(text);
    m_window.display();
}

void ComplexPlane::run() {
    initialize();

    bool update = true;
    
    while (m_window.isOpen()) {
        Event event;
        while (m_window.pollEvent(event)) {
            handleEvent(event, update);
        }

        if (update) {
            updateRender();
            Text text;
            loadText(text);
            m_window.clear();
            m_window.draw(*this);
            m_window.draw(text);
            m_window.display();
            update = false;
        }
    }
}

void ComplexPlane::handleEvent(Event& event, bool& update) {
    switch (event.type) {
        case Event::Closed:
            m_window.close();
            break;
        case Event::MouseButtonPressed:
            handleMouseClick(event.mouseButton);
            update = true;
            break;
        case Event::MouseMoved:
            setMouseLocation(Mouse::getPosition(m_window));
            update = true;
            break;
        case Event::KeyPressed:
            handleKeyboard(event.key);
            break;
        default:
            break;
    }
}

void ComplexPlane::handleMouseClick(Event::MouseButtonEvent& mouseEvent) {
    if (mouseEvent.button == Mouse::Right) {
        zoomOut();
    }
    if (mouseEvent.button == Mouse::Left) {
        zoomIn();
        setCenter(Mouse::getPosition(m_window));
    }
}

void ComplexPlane::handleKeyboard(Event::KeyEvent& keyEvent) {
    if (keyEvent.code == Keyboard::Escape) {
        m_window.close();
    }
}

void ComplexPlane::handleException(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    m_window.close();
}

void ComplexPlane::updateRender() {
    for (int i = 0; i < m_pixelHeight; ++i) {
        for (int j = 0; j < m_pixelWidth; ++j) {
            Vector2f coord = mapPixelToCoords({ j, i });
            size_t iterations = countIterations(coord);

            Uint8 r, g, b;
            iterationsToRGB(iterations, r, g, b);

            m_vArray[j + i * m_pixelWidth].position = { (float)j, (float)i };
            m_vArray[j + i * m_pixelWidth].color = { r, g, b };
        }
    }
}

void ComplexPlane::zoomIn() {
    m_zoomCount++;
    float newSizeX = m_baseWidth * pow(m_baseZoom, m_zoomCount);
    float newSizeY = m_baseHeight * m_aspectRatio * pow(m_baseZoom, m_zoomCount);
    m_plane_size = { newSizeX, newSizeY };
}

void ComplexPlane::zoomOut() {
    m_zoomCount--;
    float newSizeX = m_baseWidth * pow(m_baseZoom, m_zoomCount);
    float newSizeY = m_baseHeight * m_aspectRatio * pow(m_baseZoom, m_zoomCount);
    m_plane_size = { newSizeX, newSizeY };
}

void ComplexPlane::setCenter(Vector2i mousePixel) {
    m_plane_center = mapPixelToCoords(mousePixel);
}

void ComplexPlane::setMouseLocation(Vector2i mousePixel) {
    m_mouseLocation = mapPixelToCoords(mousePixel);
}

void ComplexPlane::loadText(Text& text) {
    stringstream ss;
    ss << m_presentationName;
    ss << "Center: (" << m_plane_center.x << ", " << m_plane_center.y << ")\n";
    ss << "Cursor: (" << m_mouseLocation.x << ", " << m_mouseLocation.y << ")\n";
    ss << "Left click to zoom in\n";
    ss << "Right click to zoom out\n";
    text.setString(ss.str());
}

size_t ComplexPlane::countIterations(Vector2f coord) {
    complex<double> c(coord.x, coord.y);
    complex<double> z(DEFAULT_Z_VALUE);
    size_t count = 0;
    while (abs(z) <= DEFAULT_ABS_THRESHOLD && count < m_maxIter) {
        z = z * z + c;
        count++;
    }
    return count;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b) {
    if (count == m_maxIter) {
        r = g = b = NO_RGB_VALUE;
    } else {
        int region = count / (m_maxIter / MAX_ITER_REGIONS);
        int remainder = count % (m_maxIter / MAX_ITER_REGIONS);
        int increment = MAX_RGB_VALUE / (m_maxIter / MAX_ITER_REGIONS);
        switch (region) {
            case 0:
                r = HALF_RGB_VALUE + remainder * increment;
                g = NO_RGB_VALUE;
                b = MAX_RGB_VALUE;
                break;
            case 1:
                r = NO_RGB_VALUE;
                g = remainder * increment;
                b = MAX_RGB_VALUE;
                break;
            case 2:
                r = NO_RGB_VALUE;
                g = MAX_RGB_VALUE;
                b = MAX_RGB_VALUE - remainder * increment;
                break;
            case 3:
                r = remainder * increment;
                g = MAX_RGB_VALUE;
                b = NO_RGB_VALUE;
                break;
            case 4:
                r = MAX_RGB_VALUE;
                g = MAX_RGB_VALUE - remainder * increment;
                b = NO_RGB_VALUE;
                break;
        }
    }
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel) {
    float newX = ((mousePixel.x - 0) / static_cast<float>(m_pixelWidth)) * m_plane_size.x + (m_plane_center.x - m_plane_size.x / 2.0);
    float newY = ((mousePixel.y - m_pixelHeight) / static_cast<float>(0 - m_pixelHeight)) * m_plane_size.y + (m_plane_center.y - m_plane_size.y / 2.0);
    return { newX, newY };
}

void ComplexPlane::draw(RenderTarget& target, RenderStates states) const {
    target.draw(m_vArray, states);
}

int main() {
    ComplexPlane complexPlane;
    complexPlane.run();

    return EXIT_SUCCESS;
}
