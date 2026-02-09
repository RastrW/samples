#ifndef _SDL_CHILD_H
#define _SDL_CHILD_H

#include <qmdisubwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qgridlayout.h>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths> // For a more robust temporary location

#if(!defined(SDL_NO))
#include <SDL3\SDL.h>
#include <SDL3_image\SDL_image.h>
#endif

class SDLChild : public QMdiSubWindow {
    Q_OBJECT
public:
    SDLChild(QWidget * parent);
    ~SDLChild();
#if(!defined(SDL_NO))
    SDL_AppResult SDLInit();
    SDL_AppResult  SDLInit2();
    void SetWindow(SDL_Window * ref);
    void SetRenderer(SDL_Renderer * ref);
    SDL_Window * GetWindow();
    SDL_Renderer * GetRenderer();
private:
    SDL_Window * WindowRef;
    SDL_Renderer * RendererRef;
private slots:
    void Render();
    void OnClose();
#endif
private:
    QWidget * MainWindowWidget;
    QTimer * Time;
    int position;
    int dir;

};

#endif
