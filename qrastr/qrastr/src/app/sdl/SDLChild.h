#pragma once
#include <QWidget>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "IPlainElGraph.h"

class SDLChild : public QWidget {
    Q_OBJECT
public:
    SDLChild(QWidget * parent);
    ~SDLChild();

    SDL_AppResult SDLInit();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    SDL_Window * WindowRef;
    SDL_Renderer * RendererRef;
public slots:
    void OnClose();
private slots:
    void Render();
private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;
    QTimer*       m_timer    = nullptr;
};

