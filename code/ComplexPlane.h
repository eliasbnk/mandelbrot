#ifndef COMPLEXPLANE_H
#define COMPLEXPLANE_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <complex>

using namespace sf;
using namespace std;

class ComplexPlane : public Drawable {
public:
    ComplexPlane();
    void run();

private:
    void draw(RenderTarget& target, RenderStates states) const override;
    void updateRender();
    void zoomIn();
    void zoomOut();
    void setCenter(Vector2i mousePixel);
    void setMouseLocation(Vector2i mousePixel);
    void loadText(Text& text);
    void handleEvent(RenderWindow& window, bool& update);
    size_t countIterations(Vector2f coord);
    void iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b);
    Vector2f mapPixelToCoords(Vector2i mousePixel);

    const unsigned int MAX_ITER;
    const float BASE_WIDTH;
    const float BASE_HEIGHT;
    const float BASE_ZOOM;

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
};

#endif
