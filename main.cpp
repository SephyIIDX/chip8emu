
#include <iostream>
#include "chip8.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/glcanvas.h>
#include <gl/GLU.h>
#include <gl/GL.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    void OnOpen(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
};

class MyGLCanvas : public wxGLCanvas
{
public:
    MyGLCanvas(wxFrame *parent);

    void prepare2DViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y);

    void keyPressed(wxKeyEvent &event);
    void keyReleased(wxKeyEvent &event);
    void OnIdle(wxIdleEvent &event);
    void OnTimer(wxTimerEvent &event);
    void OnPaint(wxPaintEvent &event);

private:
    wxTimer m_timer;
    DECLARE_EVENT_TABLE();
};

enum
{
    ID_Timer = wxID_HIGHEST + 1
};

// Display size
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define FRAME_LIMIT 60

string romFilename;
Chip8 chip8;
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;
int ms_interval = 1000 / FRAME_LIMIT;
bool frame_limit_reached = false;

GLubyte screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];

//---------------------------------------------------------------------------
// MyApp
//---------------------------------------------------------------------------

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Chip8 Emulator", wxDefaultPosition, wxSize(display_width + 16, display_height + 60))
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);
    
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    MyGLCanvas *glCanvas = new MyGLCanvas(this);
}
void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog openFileDialog(this, _("Open Chip8 file"), "", "",
                       "C8 files (*.c8)|*.c8", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    chip8.initialize();
    chip8.loadGame(openFileDialog.GetFilename().ToStdString());
}

void MyFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
EVT_PAINT(MyGLCanvas::OnPaint)
EVT_IDLE(MyGLCanvas::OnIdle)
EVT_TIMER(ID_Timer, MyGLCanvas::OnTimer)
EVT_KEY_DOWN(MyGLCanvas::keyPressed)
EVT_KEY_UP(MyGLCanvas::keyReleased)
END_EVENT_TABLE()

MyGLCanvas::MyGLCanvas(wxFrame *parent)
    : wxGLCanvas(parent, wxID_ANY, {WX_GL_RGBA, WX_GL_DOUBLEBUFFER}), m_timer(this, ID_Timer)
{
    //m_timer.Start(ms_interval);
}

void MyGLCanvas::OnIdle(wxIdleEvent &event)
{
    // Emulate one cycle
    chip8.emulateCycle();

    // If the draw flag is set, update the screen
    if (chip8.drawFlag)
    {
        Refresh(false);
        chip8.drawFlag = false;
    }
    event.RequestMore();
}

void MyGLCanvas::OnTimer(wxTimerEvent &event)
{
}

void MyGLCanvas::keyPressed(wxKeyEvent &event)
{
    switch (event.GetUnicodeKey())
    {
    case '1':
        chip8.key[0x1] = 1;
        break;
    case '2':
        chip8.key[0x2] = 1;
        break;
    case '3':
        chip8.key[0x3] = 1;
        break;
    case '4':
        chip8.key[0xC] = 1;
        break;

    case 'Q':
        chip8.key[0x4] = 1;
        break;
    case 'W':
        chip8.key[0x5] = 1;
        break;
    case 'E':
        chip8.key[0x6] = 1;
        break;
    case 'R':
        chip8.key[0xD] = 1;
        break;

    case 'A':
        chip8.key[0x7] = 1;
        break;
    case 'S':
        chip8.key[0x8] = 1;
        break;
    case 'D':
        chip8.key[0x9] = 1;
        break;
    case 'F':
        chip8.key[0xE] = 1;
        break;

    case 'Z':
        chip8.key[0xA] = 1;
        break;
    case 'X':
        chip8.key[0x0] = 1;
        break;
    case 'C':
        chip8.key[0xB] = 1;
        break;
    case 'V':
        chip8.key[0xF] = 1;
        break;
    }
}
void MyGLCanvas::keyReleased(wxKeyEvent &event)
{
    switch (event.GetUnicodeKey())
    {
    case '1':
        chip8.key[0x1] = 0;
        break;
    case '2':
        chip8.key[0x2] = 0;
        break;
    case '3':
        chip8.key[0x3] = 0;
        break;
    case '4':
        chip8.key[0xC] = 0;
        break;

    case 'Q':
        chip8.key[0x4] = 0;
        break;
    case 'W':
        chip8.key[0x5] = 0;
        break;
    case 'E':
        chip8.key[0x6] = 0;
        break;
    case 'R':
        chip8.key[0xD] = 0;
        break;

    case 'A':
        chip8.key[0x7] = 0;
        break;
    case 'S':
        chip8.key[0x8] = 0;
        break;
    case 'D':
        chip8.key[0x9] = 0;
        break;
    case 'F':
        chip8.key[0xE] = 0;
        break;

    case 'Z':
        chip8.key[0xA] = 0;
        break;
    case 'X':
        chip8.key[0x0] = 0;
        break;
    case 'C':
        chip8.key[0xB] = 0;
        break;
    case 'V':
        chip8.key[0xF] = 0;
        break;
    }
}

void MyGLCanvas::OnPaint(wxPaintEvent &event)
{
    if (!IsShown())
        return;

    // This is a dummy, to avoid an endless succession of paint messages.
    // OnPaint handlers must always create a wxPaintDC.
    wxPaintDC(this);

    SetCurrent();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prepare2DViewport(0, 0, display_width, display_height);

    glLoadIdentity();

    // Update pixels
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
        for (int x = 0; x < SCREEN_WIDTH; ++x)
            if (chip8.gfx[(y * SCREEN_WIDTH) + x] == 0)
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0; // Disabled
            else
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255; // Enabled

    // Create a texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, screenData);

    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(display_width, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(display_width, display_height);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0, display_height);
    glEnd();

    glFlush();
    SwapBuffers();
}

void MyGLCanvas::prepare2DViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y)
{

    /*
     *  Inits the OpenGL viewport for drawing in 2D
     */

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);              // textures
    //glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_BLEND);
    //glDisable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleft_x, topleft_y, bottomright_x - topleft_x, bottomright_y - topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(topleft_x, bottomright_x, bottomright_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
