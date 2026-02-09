#include "SDLChild.h"

SDLChild::SDLChild(QWidget * parent) : QMdiSubWindow(parent) {
    /*
        If you do not set an inner widget, the SubWindow itself IS
        a widget, so SDL renders over it... AND its controls. Not
        quite the desired behavior, unless you want to hide the
        controls until mouse over. But once the mouse is over, SDL
        writes over the controls again. Keep this in mind.
    */
    setWidget(new QWidget);

    /*
    I used a timer for animation rendering.
    I tried using update() and repaint() as the slot, but this had
    no effect. I later tried calling update/repaint from within the
    Render() slot, and this caused a flicker. The only thing that I
    can assume is that after the paintEvent override, qt paints the
    grey backgrounds.
    */
    Time = new QTimer(this);
    connect(Time, SIGNAL(timeout()), this, SLOT(Render()));
    Time->start(1000 / 60);
#if(!defined(SDL_NO))
    RendererRef = 0;
    WindowRef = 0;
#endif
    position = 0;
    dir = 1;
}

SDLChild::~SDLChild() {
 #if(!defined(SDL_NO))
    SDL_DestroyRenderer(RendererRef);	// Basic SDL garbage collection
    SDL_DestroyWindow(WindowRef);


    delete Time;

    RendererRef = 0;
    WindowRef = 0;
    Time = 0;
    #endif
}
#if(!defined(SDL_NO))
SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}
SDL_AppResult SDL_OK(){
    return SDL_APP_SUCCESS;
}

SDL_AppResult SDLChild::SDLInit() {

    HWND qtWindowHandle = (HWND)this->winId();

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Графика");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true );
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, qtWindowHandle);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 640);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 480);

    SDL_Window* pWindow = SDL_CreateWindowWithProperties(props);
    // create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(pWindow, NULL);
    if (not renderer){
        return SDL_Fail();
    }

    // load the SVG
    //SDL_Surface* svg_surface = IMG_Load(":/images/cx195.svg");

    //Get  absolute Path from resourceFile
   /* QString absoluteTempPath = "";
    QString resourcePath = ":/images/cx195.svg";
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        QFile resourceFile(resourcePath);
        if (resourceFile.open(QIODevice::ReadOnly)) {
            tempFile.write(resourceFile.readAll());
            resourceFile.close();
            tempFile.close(); // Close to ensure data is flushed
            absoluteTempPath = tempFile.fileName();
        } else {
            qDebug() << "Failed to open resource file:" << resourcePath;
        }
    } else {
        qDebug() << "Failed to create temporary file.";
    }*/

    QFile f(":/images/cx195.svg");
    //QTemporaryFile* ptf = QTemporaryFile::createNativeFile(f); // Returns a pointer to a temporary file

    QString tempDirPath = QDir::tempPath();
    QString str_f_copy = QDir::toNativeSeparators(tempDirPath + "/" + "cx195_copy.svg");
    QFile::remove(str_f_copy);
    f.copy(str_f_copy);

    std::string str_file = str_f_copy.toStdString().c_str();
    //std::string str_file = QDir::toNativeSeparators(ptf->fileName()).toStdString().c_str();
    SDL_Surface* svg_surface = IMG_Load(str_file.c_str());          // не получается через QTemporaryFile
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, svg_surface);
    SDL_DestroySurface(svg_surface);

    // Select the color for drawing. It is set to red here.
    //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Clear the entire screen to our selected color.
    //SDL_RenderClear(renderer);

    // Up until now everything was drawn behind the scenes.
    // This will show the new, red contents of the window.

    // Clear screen with a background color (e.g., gray)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Render the texture to the entire window
    SDL_RenderTexture(renderer, tex, NULL, NULL);

    SDL_RenderPresent(renderer);

    return SDL_OK();

    //SetRenderer(SDL_CreateRenderer(GetWindow(), -1, SDL_RENDERER_ACCELERATED));
}
SDL_AppResult  SDLChild::SDLInit2() {
    /*
        In order to do rendering, I need to save the window and renderer contexts
        of this window.
        I use SDL_CreateWindowFrom and pass it the winId() of the widget I wish to
        render to. In this case, I want to render to a widget that I added.
    */
    // SetWindow(SDL_CreateWindowFrom((void *)widget()->winId()));

    constexpr uint32_t windowStartWidth = 400;
    constexpr uint32_t windowStartHeight = 400;
    SDL_Window* window = SDL_CreateWindow("SDL Minimal Sample", windowStartWidth, windowStartHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (not window){
        return SDL_Fail();

    }
    // create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (not renderer){
        return SDL_Fail();
    }
    return SDL_OK();
}

void SDLChild::Render() {
    // Basic square bouncing animation
    SDL_Rect spos;
    spos.h = 100;
    spos.w = 100;
    spos.y = height() / 2 - 50;
    spos.x = position;

    SDL_SetRenderDrawColor(RendererRef, 0, 0, 0, 255);
    SDL_RenderFillRect(RendererRef, 0);
    SDL_SetRenderDrawColor(RendererRef, 0xFF, 0x0, 0x0, 0xFF);
   // SDL_RenderFillRect(RendererRef, &spos);
    SDL_RenderPresent(RendererRef);

    if (position >= width() - 100)
        dir = 0;
    else if (position <= 0)
        dir = 1;

    if (dir)
        position += 5;
    else
        position -= 5;
}
void SDLChild::OnClose()
{
    //Close SDL Window
    SDL_Quit();
}


void SDLChild::SetWindow(SDL_Window * ref) {
    WindowRef = ref;
}

void SDLChild::SetRenderer(SDL_Renderer * ref) {
    RendererRef = ref;
}

SDL_Window * SDLChild::GetWindow() {
    return WindowRef;
}

SDL_Renderer * SDLChild::GetRenderer() {
    return RendererRef;
}
#endif
