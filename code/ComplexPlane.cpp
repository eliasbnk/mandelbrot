#include "ComplexPlane.h"

const unsigned int DEFAULT_MAX_ITER = 64;
const float DEFAULT_BASE_WIDTH = 4.0;
const float DEFAULT_BASE_HEIGHT = 4.0;
const float DEFAULT_BASE_ZOOM = 0.5;

const Color DEFAULT_TEXT_COLOR = Color::White;
const string DEFAULT_FONT_FILE = "KOMIKAP_.ttf";
const string DEFAULT_MUSIC_FILE = "muzika.wav";
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

ComplexPlane::ComplexPlane()
    : MAX_ITER(DEFAULT_MAX_ITER), BASE_WIDTH(DEFAULT_BASE_WIDTH), BASE_HEIGHT(DEFAULT_BASE_HEIGHT), BASE_ZOOM(DEFAULT_BASE_ZOOM),
      m_zoomCount(DEFAULT_ZOOM_COUNT), m_State(State::CALCULATING) {
    m_pixelWidth = VideoMode::getDesktopMode().width / 2;
    m_pixelHeight = VideoMode::getDesktopMode().height / 2;
    m_aspectRatio = static_cast<float>(m_pixelHeight) / m_pixelWidth;
    m_plane_center = DEFAULT_PLANE_CENTER;
    m_plane_size = { BASE_WIDTH, BASE_HEIGHT * m_aspectRatio };

    m_vArray.setPrimitiveType(Points);
    m_vArray.resize(m_pixelWidth * m_pixelHeight);
}

void ComplexPlane::run() {
    RenderWindow window(VideoMode(m_pixelWidth, m_pixelHeight), DEFAULT_WINDOW_NAME);

    Font font;
    if (!font.loadFromFile(DEFAULT_FONT_FILE)) {
        cerr << "Error loading font" << endl;
        return;
    }
    SoundBuffer buffer;
    if(!buffer.loadFromFile(DEFAULT_MUSIC_FILE)){
        cerr << "Error loading music" << endl;
        return;
    }
    Sound sound;
    sound.setBuffer(buffer);
    sound.setLoop(true);
    thread musicThread(playMusic, std::ref(music));
    musicThread.join();

    Text text("", font, DEFAULT_CHARACTER_SIZE);
    text.setFillColor(DEFAULT_TEXT_COLOR);
    bool update = true;
    

    while (window.isOpen()) {

        
        handleEvent(window, update);

        if (update) {
            updateRender();
            loadText(text);
            update = false;
        }

        window.clear();
        window.draw(*this);
        window.draw(text);
        window.display();
    }
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
    ss << DEFAULT_PRESENTATION_NAME;
    ss << "Center: (" << m_plane_center.x << ", " << m_plane_center.y << ")\n";
    ss << "Cursor: (" << m_mouseLocation.x << ", " << m_mouseLocation.y << ")\n";
    ss << "Left click to zoom in\n";
    ss << "Right click to zoom out\n";
    text.setString(ss.str());
}

void ComplexPlane::handleEvent(RenderWindow& window, bool& update) {
    Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case Event::Closed:
                window.close();
                break;
            case Event::MouseButtonPressed:
                if (event.mouseButton.button == Mouse::Right) {
                    zoomOut();
                    update = true;
                }
                if (event.mouseButton.button == Mouse::Left) {
                    zoomIn();
                    setCenter(Mouse::getPosition(window));
                    update = true;
                }
                break;
            case Event::MouseMoved:
                setMouseLocation(Mouse::getPosition(window));
                update = true;
                break;
            default:
                break;
        }
    }
    if (Keyboard::isKeyPressed(Keyboard::Escape))
        window.close();
}

size_t ComplexPlane::countIterations(Vector2f coord) {
    complex<double> c(coord.x, coord.y);
    complex<double> z(0, 0);
    size_t count = 0;
    while (abs(z) <= DEFAULT_ABS_THRESHOLD && count < MAX_ITER) {
        z = z * z + c;
        count++;
    }
    return count;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b) {
    if (count == MAX_ITER) {
        r = g = b = NO_RGB_VALUE;
    } else {
        int region = count / (MAX_ITER / MAX_ITER_REGIONS);
        int remainder = count % (MAX_ITER / MAX_ITER_REGIONS);
        int increment = MAX_RGB_VALUE / (MAX_ITER / MAX_ITER_REGIONS);
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
