#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>

using namespace sf;
using namespace std;

class ComplexPlane : public Drawable {
public:
    ComplexPlane(int pixelWidth, int pixelHeight);
    void draw(RenderTarget& target, RenderStates states) const override;
    void updateRender();
    void zoomIn();
    void zoomOut();
    void setCenter(Vector2i mousePixel);
    void setMouseLocation(Vector2i mousePixel);
    void loadText(Text& text);

private:
    const unsigned int MAX_ITER = 64;
    const float BASE_WIDTH = 4.0;
    const float BASE_HEIGHT = 4.0;
    const float BASE_ZOOM = 0.5;

    enum class State { CALCULATING, DISPLAYING };

    VertexArray m_vArray;
    int m_pixelWidth;
    int m_pixelHeight;
    float m_aspectRatio;
    Vector2f m_plane_center;
    Vector2f m_plane_size;
    int m_zoomCount;
    State m_State;
    Vector2f m_mouseLocation;

    size_t countIterations(Vector2f coord);
    void iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b);
    Vector2f mapPixelToCoords(Vector2i mousePixel);
};

ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight)
    : m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight), m_zoomCount(0), m_State(State::CALCULATING) {
    m_aspectRatio = static_cast<float>(pixelHeight) / pixelWidth;
    m_plane_center = { 0, 0 };
    m_plane_size = { BASE_WIDTH, BASE_HEIGHT * m_aspectRatio };

    m_vArray.setPrimitiveType(Points);
    m_vArray.resize(pixelWidth * pixelHeight);
}

void ComplexPlane::draw(RenderTarget& target, RenderStates states) const {
    target.draw(m_vArray);
}

void ComplexPlane::updateRender() {
    if (m_State == State::CALCULATING) {
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
        m_State = State::DISPLAYING;
    }
}

void ComplexPlane::zoomIn() {
    m_zoomCount++;
    float newSizeX = BASE_WIDTH * pow(BASE_ZOOM, m_zoomCount);
    float newSizeY = BASE_HEIGHT * m_aspectRatio * pow(BASE_ZOOM, m_zoomCount);
    m_plane_size = { newSizeX, newSizeY };
    m_State = State::CALCULATING;
}

void ComplexPlane::zoomOut() {
    m_zoomCount--;
    float newSizeX = BASE_WIDTH * pow(BASE_ZOOM, m_zoomCount);
    float newSizeY = BASE_HEIGHT * m_aspectRatio * pow(BASE_ZOOM, m_zoomCount);
    m_plane_size = { newSizeX, newSizeY };
    m_State = State::CALCULATING;
}

void ComplexPlane::setCenter(Vector2i mousePixel) {
    m_plane_center = mapPixelToCoords(mousePixel);
    m_State = State::CALCULATING;
}

void ComplexPlane::setMouseLocation(Vector2i mousePixel) {
    m_mouseLocation = mapPixelToCoords(mousePixel);
}

void ComplexPlane::loadText(Text& text) {
    stringstream ss;
    ss << "Mandelbrot Set\n";
    ss << "Center: (" << m_plane_center.x << ", " << m_plane_center.y << ")\n";
    ss << "Cursor: (" << m_mouseLocation.x << ", " << m_mouseLocation.y << ")\n";
    ss << "Left click to zoom in\n";
    ss << "Right click to zoom out\n";
    text.setString(ss.str());
}

size_t ComplexPlane::countIterations(Vector2f coord) {
    complex<double> c(coord.x, coord.y);
    complex<double> z(0, 0);
    size_t count = 0;
    while (abs(z) <= 2.0 && count < MAX_ITER) {
        z = z * z + c;
        count++;
    }
    return count;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b) {
    if (count == MAX_ITER) {
        r = g = b = 0;
    } else {
        int region = count / (MAX_ITER / 5);
        int remainder = count % (MAX_ITER / 5);
        int increment = 255 / (MAX_ITER / 5);
        switch (region) {
            case 0:
                r = 128 + remainder * increment;
                g = 0;
                b = 255;
                break;
            case 1:
                r = 0;
                g = remainder * increment;
                b = 255;
                break;
            case 2:
                r = 0;
                g = 255;
                b = 255 - remainder * increment;
                break;
            case 3:
                r = remainder * increment;
                g = 255;
                b = 0;
                break;
            case 4:
                r = 255;
                g = 255 - remainder * increment;
                b = 0;
                break;
        }
    }
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel) {
    float newX = ((mousePixel.x - 0) / static_cast<float>(m_pixelWidth)) * m_plane_size.x + (m_plane_center.x - m_plane_size.x / 2.0);
    float newY = ((mousePixel.y - m_pixelHeight) / static_cast<float>(0 - m_pixelHeight)) * m_plane_size.y + (m_plane_center.y - m_plane_size.y / 2.0);
    return { newX, newY };
}


void playMahJam() {
    Music jam;

    if (!jam.openFromFile("music.wav")){
        std::cout << "Error..." << std::endl;
    }
    else{
        jam.setLoop(true);
        jam.setVolume(50);
        jam.play();
    }
}


int main() {
    int pixelWidth = VideoMode::getDesktopMode().width / 2;
    int pixelHeight = VideoMode::getDesktopMode().height / 2;
    RenderWindow window(VideoMode(pixelWidth, pixelHeight), "Mandelbrot Set Visualizer");

    ComplexPlane complexPlane(pixelWidth, pixelHeight);

    Font font;
    if (!font.loadFromFile("KOMIKAP_.ttf")) {
        cerr << "Error loading font" << endl;
        return EXIT_FAILURE;
    }

    Text text("", font, 16);
    text.setFillColor(Color::White);
    playMahJam();
    bool update = true;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
    if (event.type == Event::Closed)
        window.close();
    else if (event.type == Event::MouseButtonPressed) {
        if (event.mouseButton.button == Mouse::Right) {
            complexPlane.zoomOut();
            update = true;
        } else if (event.mouseButton.button == Mouse::Left) {
            complexPlane.zoomIn();
            complexPlane.setCenter(Mouse::getPosition(window));
            update = true;
        }
    } else if (event.type == Event::TouchBegan) {
        if (event.touch.finger == 2) {
            complexPlane.zoomIn();
            complexPlane.setCenter(Mouse::getPosition(window));
            update = true;
        }
    } else if (event.type == Event::MouseMoved) {
        complexPlane.setMouseLocation(Mouse::getPosition(window));
        update = true;
    }
}

        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();

        if (update) {
            complexPlane.updateRender();
            complexPlane.loadText(text);
            update = false;
        }

        window.clear();
        window.draw(complexPlane);
        window.draw(text);
        window.display();
    }

    return EXIT_SUCCESS;
}
