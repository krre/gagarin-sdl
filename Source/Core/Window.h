#pragma once
#include <string>

struct SDL_Window;

class Window {

public:
    Window();

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    int getX() const { return x; }
    int getY() const { return y; }

    void setWidth(int width) { this->width = width; }
    void setHeight(int height) { this->height = height; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void setTitle(const std::string& title) { this->title = title; }
    std::string getTitle() const { return title; }

private:
    SDL_Window* handle = nullptr;
    int x = 0;
    int y = 0;
    int width = 100;
    int height = 100;
    std::string title = "Untitled";

    void create();
};
