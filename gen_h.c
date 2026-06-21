#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "soundFactory.h"

#pragma comment(lib, "winmm.lib")

#define PI 3.14159265
#define BUFFERSIZE 1800

#define WINDOWWIDTH 730
#define WINDOWHEIGHT 500

#define ID_BUTTON_EXPORTNOW 31
#define ID_EDIT_FILENAME 32
#define ID_EDIT_NBEATS 33

#define ID_EDIT_FILENAME2 50
#define ID_BUTTON_SAVE 51
#define ID_BUTTON_LOAD 52
#define ID_BUTTON_ADD 53


#define ID_BUTTON_REWIND 20
#define ID_BUTTON_PLAY 21
#define ID_BUTTON_NAVUP 22
#define ID_BUTTON_NAVDOWN 23
#define ID_BUTTON_NAVLEFT 24
#define ID_BUTTON_NAVRIGHT 25
#define ID_BUTTON_CLEARPLAYLIST 26
#define ID_BUTTON_EXPORT 27
#define ID_BUTTON_TEMPOUP 28
#define ID_BUTTON_TEMPODOWN 29
#define ID_BUTTON_LENGTHUP 30
#define ID_BUTTON_LENGTHDOWN 31
#define ID_BUTTON_STARTMIDIIN 40
#define ID_BUTTON_LOADSAVE 41

#define ID_BUTTON_STARTMIDI 68
#define START_MIDI_WINDOW_WIDTH 320
#define START_MIDI_WINDOW_HEIGHT 400




LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndExportProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndStartMIDIINProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndLoadSaveProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


SOUNDFACTORY theFactory;


//MIDI----------------------------
int midiStarted = 0;
int midiDeviceNumber = 0;
HMIDIIN device;
int flag;
int activeMIDIDevices;
//--------------------------------

int mouseInRect(int mouseX, int mouseY, int rectX1, int rectY1, int rectX2, int rectY2);
void fillBuffer(short *bf);

void theGraphics(HWND hwnd);
void theStartMIDIGraphics(HWND hwnd);

void playlistInsertAfter(int v1, int v2, int v3, struct plNode *t);
void playlistDeleteNext(struct plNode *t);
void emptyPlaylist(void);

void exportToWav(int nBeats, char * fileName);

int savePlaylist(char * fileName);
int loadPlaylist(char * fileName, int option);

void uppdateArea(HWND hwnd, int x1, int y1, int x2, int y2, int margin);

WCHAR editInputBuffer[20]; 
WORD cchText;
char editCharBuffer[20];
char inpChar;

WCHAR charBuffer[10];


int playOn = 0;

int i = 0;
int currentBuffer = 1;
int timeShift = 0;
int toneShift = 12;
int currentNoteLength = 4;

int ticker = 0;
int smallTicker = 0;

int savedTicker = 0;

int bpm = 120;
int beatres = 8;
int playlistLength = 128;

exportWindowActive = 0;
startMIDIINWindowActive = 0;
loadSaveWindowActive = 0;

int currentItem = 0; //used for choice in startMIDI-window
MIDIINCAPS theInfo; //used to show info about MIDI-devices
LPCTSTR theString;


//playlist in a linked list-form
struct plNode {
  int tone;
  int timeMark;
  int length;
  struct plNode *next;
};



struct plNode *playlistHead, *playlistZero;

struct plNode *loopPointer;

WAVEHDR header;
WAVEHDR header2;

HWAVEOUT hWaveOut = 0;
WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 44100, 44100*2, 2, 16, 0 };

short buffer1[BUFFERSIZE];
short buffer2[BUFFERSIZE];

RECT areaRect;
RECT visualisationArea;

int markedButton = 0;
int markedButtonOld = 0;

int markedButton2 = 0;   //for start MIDI IN window
int markedButtonOld2 = 0; //for start MIDI IN window

HWND hwnd2;
HINSTANCE thisInstance;
WNDCLASSW wc2;

HWND hwnd3; //export
WNDCLASSW wc3;

HWND hwnd4; //start MIDIIN
WNDCLASSW wc4;

HWND hwnd5; //loadSave playlist
WNDCLASSW wc5;

int doStartMIDI = 0;
int doUpdate = 0;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    PWSTR pCmdLine, int nCmdShow) {
    
    MSG  msg;    
    HWND hwnd;
    WNDCLASSW wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.lpszClassName = L"PyrrosSynthHost";
    wc.hInstance     = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName  = NULL;
    wc.lpfnWndProc   = WndProc;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassW(&wc);

    hwnd = CreateWindowW(wc.lpszClassName, L"Pyrros Synth Host",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                100, 100, WINDOWWIDTH, WINDOWHEIGHT, NULL, NULL, hInstance, NULL);  


    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, 
    WPARAM wParam, LPARAM lParam) {

    if ((doStartMIDI > 0)&&(midiStarted < 1)){
        //midiDeviceNumber = 1;  //this we get from the startMIDI-window

        //open the given device
        flag = midiInOpen(&device, midiDeviceNumber, (DWORD_PTR)hwnd,0, CALLBACK_WINDOW);
        if (!flag == MMSYSERR_NOERROR){
            MessageBoxW(NULL, L"Error opening MIDI Input.", L"Error", MB_OK);
        }
        else{
            flag = midiInStart(device);
            if (!flag == MMSYSERR_NOERROR){
                MessageBoxW(NULL, L"Error starting MIDI recording.", L"Error", MB_OK);
                midiInReset(device);
                midiInClose(device);
            }
            else{
                midiStarted = 1;
                //MessageBoxW(NULL, L"MIDI IN device started.", L"Success!", MB_OK);
                InvalidateRect( hwnd, NULL, TRUE );
            }
        }
        doStartMIDI = 0;
    }

    if (doUpdate > 0){
        InvalidateRect( hwnd, NULL, TRUE );
        doUpdate = 0;
    }

    switch(msg) {        
        case WM_CREATE:
            visualisationArea.top = 60;
            visualisationArea.bottom = 200;
            visualisationArea.left = 910;
            visualisationArea.right = 1110;


            //initialize linked list-playlist
            playlistHead = (struct plNode *)malloc(sizeof *playlistHead);
            playlistZero = (struct plNode *)malloc(sizeof *playlistZero);


            playlistHead->next = playlistZero;
            playlistHead->tone = -1;
            playlistHead->timeMark = -1;
            playlistHead->length = -1;

            playlistZero->tone = -1;
            playlistZero->timeMark = -1;
            playlistZero->length = -1;
            playlistZero->next = playlistZero;


            soundFactoryInitialize(&theFactory);

            areaRect.top = 0;
            areaRect.bottom = 10;
            areaRect.left = 0;
            areaRect.right = 10;

            fillBuffer(buffer1);
            fillBuffer(buffer2);

            waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)hwnd, 0, CALLBACK_WINDOW);

            header.lpData = (LPSTR)buffer1;
            header.dwBufferLength = sizeof(buffer1);
            header.dwBytesRecorded = 0;
            header.dwUser = 0;
            header.dwFlags = 0;
            header.dwLoops = 0;
            header.lpNext = 0;
            header.reserved = 0;

            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));

            header2.lpData = (LPSTR)buffer2;
            header2.dwBufferLength = sizeof(buffer2);
            header2.dwBytesRecorded = 0;
            header2.dwUser = 0;
            header2.dwFlags = 0;
            header2.dwLoops = 0;
            header2.lpNext = 0;
            header2.reserved = 0;

            waveOutPrepareHeader(hWaveOut, &header2, sizeof(WAVEHDR));

            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header2, sizeof(WAVEHDR));

            //Open midi synth

            //midiDeviceNumber = 1;

            //open the given device
            //flag = midiInOpen(&device, midiDeviceNumber, (DWORD_PTR)hwnd,0, CALLBACK_WINDOW);
            //if (!flag == MMSYSERR_NOERROR){
            //    MessageBoxW(NULL, L"Error opening MIDI Input.", L"Error", MB_OK);
            //}
            //else{
            //    flag = midiInStart(device);
            //    if (!flag == MMSYSERR_NOERROR){
            //        MessageBoxW(NULL, L"Error starting MIDI recording.", L"Error", MB_OK);
            //        midiInReset(device);
            //        midiInClose(device);
            //    }
            //    else{
           //         midiStarted = 1;
            //        InvalidateRect( hwnd, NULL, TRUE );
            //    }
            //}

            wc2.style         = CS_HREDRAW | CS_VREDRAW;
            wc2.cbClsExtra    = 0;
            wc2.cbWndExtra    = 0;
            wc2.lpszClassName = L"PyrrosSynthSoundFactory";
            wc2.hInstance     = thisInstance;
            wc2.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
            wc2.lpszMenuName  = NULL;
            wc2.lpfnWndProc   = soundFactoryCallbackWindow;
            wc2.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wc2.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

            RegisterClassW(&wc2);

            hwnd2 = CreateWindowW(wc2.lpszClassName, L"Sound Factory",
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        820, 100, 400, 500, NULL, NULL, thisInstance, NULL);  


            ShowWindow(hwnd2, SW_SHOW);
            UpdateWindow(hwnd2);


            break;


        case MM_WOM_DONE:
            if (currentBuffer == 1){
                fillBuffer(buffer1);
                currentBuffer=2;
                waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            }
            else if (currentBuffer == 2){
                fillBuffer(buffer2);
                currentBuffer=1;
                waveOutWrite(hWaveOut, &header2, sizeof(WAVEHDR));
            }
            if (((ticker - timeShift*8)>=0) && ((ticker - timeShift*8)<32)&&playOn){
                areaRect.top = 399;
                areaRect.bottom = 411;
                areaRect.left = 9 + (ticker - timeShift*8)*20;
                areaRect.right = 51 + (ticker - timeShift*8)*20;
                InvalidateRect( hwnd, &areaRect, TRUE );
            }
            else if ((ticker - timeShift*8)==32){
                areaRect.top = 399;
                areaRect.bottom = 411;
                areaRect.left = 9 + (ticker - timeShift*8-1)*20;
                areaRect.right = 51 + (ticker - timeShift*8-1)*20;
                InvalidateRect( hwnd, &areaRect, TRUE );
            }
            if ((timeShift > 5)&&(ticker < 2)){
                areaRect.top = 399;
                areaRect.bottom = 411;
                areaRect.left = 641;
                areaRect.right = 671;
                InvalidateRect( hwnd, &areaRect, TRUE );
            }
            break;

        case MM_MIM_DATA:
            if (LOBYTE(LOWORD(lParam)) == 0x90) {
                soundFactoryModify((int)HIBYTE(LOWORD(lParam))-9, SF_TURN_ON, &theFactory);
            }
            else if(LOBYTE(LOWORD(lParam)) == 0x80){
                soundFactoryModify((int)HIBYTE(LOWORD(lParam))-9, SF_TURN_OFF, &theFactory);
            }

            break;

        case WM_KEYUP:
            if (wParam == VK_UP){
                if (toneShift < 21){
                    toneShift += 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
            }
            else if (wParam == VK_DOWN){
                if (toneShift > 0){
                    toneShift -= 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
            }
            else if (wParam == VK_LEFT){
                if (timeShift > 0){
                    timeShift -= 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    for (i = 0; i < 4; i++){
                        uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                    }
                }
            }
            else if (wParam == VK_RIGHT){
                if (timeShift < 12){
                    timeShift += 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    for (i = 0; i < 4; i++){
                        uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                    }
                }
            }
            else if (wParam == VK_SPACE){
                if (((ticker - timeShift*8)>0) && ((ticker - timeShift*8)<32)&&(playOn)){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 9 + (ticker - timeShift*8)*20;
                    areaRect.right = 51 + (ticker - timeShift*8)*20;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
                if (playOn){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 51 ;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    savedTicker = ticker;
                }
                playOn = 1 - playOn%2;
                
                ticker = savedTicker;
                smallTicker = 0;
                
                soundFactoryModify(0, SF_EMPTY, &theFactory);

                areaRect.top = 49;
                areaRect.bottom = 71;
                areaRect.left = 59;
                areaRect.right = 81;
                InvalidateRect(hwnd, &areaRect, TRUE );
            }
            else if (wParam == VK_BACK){
                soundFactoryModify(0, SF_EMPTY, &theFactory);
                savedTicker = 0;
                ticker = 0;
                timeShift = 0;
                areaRect.top = 99;
                areaRect.bottom = 411;
                areaRect.left = 29;
                areaRect.right = 671;
                InvalidateRect(hwnd, &areaRect, TRUE );
                for (i = 0; i < 4; i++){
                    uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                }
            }
            //else if (wParam == 'S'){
            //    savePlaylist();
            //}
            //else if (wParam == 'L'){
            //    loadPlaylist(0);
            //    InvalidateRect( hwnd, NULL, TRUE );
            //}
            //else if (wParam == 'A'){
            //    loadPlaylist(1);
            //    InvalidateRect( hwnd, NULL, TRUE );
            //}

            break;
        case WM_MOUSEMOVE:
               markedButtonOld = markedButton;
               if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 30,50,50,70)){
                   markedButton = ID_BUTTON_REWIND;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 30,50,50,70,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 60,50,80,70)){
                   markedButton = ID_BUTTON_PLAY;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 60,50,80,70,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 90, 50, 110, 70)){
                   markedButton = ID_BUTTON_NAVLEFT;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 90, 50, 130, 70,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 130, 50, 150, 70)){
                   markedButton = ID_BUTTON_NAVRIGHT;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 110, 50, 150, 70,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 110, 30, 130, 50)){
                   markedButton = ID_BUTTON_NAVUP;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 110, 30, 130, 70,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 110, 70, 130, 90)){
                   markedButton = ID_BUTTON_NAVDOWN;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 110, 50, 130, 90,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 110, 50, 130, 70)){
                   markedButton = ID_BUTTON_CLEARPLAYLIST;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd,  90, 30, 150, 90,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 30,75,100,95)){
                   markedButton = ID_BUTTON_EXPORT;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 30,75,100,95,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 30,25,100,45)){
                   markedButton = ID_BUTTON_STARTMIDIIN;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 30,25,100,45,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 270,45,290,60)){
                   markedButton = ID_BUTTON_TEMPOUP;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 270,45,290,75,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 270,60,290,75)){
                   markedButton = ID_BUTTON_TEMPODOWN;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 270,45,290,75,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 405,45,425,60)){
                   markedButton = ID_BUTTON_LENGTHUP;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 405,45,425,75,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 405,60,425,75)){
                   markedButton = ID_BUTTON_LENGTHDOWN;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 405,45,425,75,1);
                   }
               }
               else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 360,77,450,97)){
                   markedButton = ID_BUTTON_LOADSAVE;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 360,77,450,97,1);
                   }
               }
               else{
                   markedButton = 0;
                   if (markedButtonOld != markedButton){
                       uppdateArea(hwnd, 0,0,WINDOWWIDTH,160,0);
                       //InvalidateRect( hwnd, NULL, TRUE );
                   }
               }       
            break;

        case WM_LBUTTONUP:
            if ((LOWORD(lParam)>30)&&(LOWORD(lParam)<670)&&(HIWORD(lParam)>100)&&(HIWORD(lParam)<400)){

                areaRect.left = ((LOWORD(lParam)-30)/20)*20 + 29;
                areaRect.right = ((LOWORD(lParam)-30)/20+currentNoteLength-1)*20 + 51;
                areaRect.top = 389 - ((400-HIWORD(lParam))/10)*10;
                areaRect.bottom = 401 - ((400-HIWORD(lParam))/10)*10;
                InvalidateRect( hwnd, &areaRect, TRUE );

                if ((400-HIWORD(lParam))>=10){

                    loopPointer = playlistHead;
                    if (loopPointer->next->next == loopPointer->next){ 
                        playlistInsertAfter((400-HIWORD(lParam)+ 40*toneShift)/10, (LOWORD(lParam)-30)/20+timeShift*8, currentNoteLength, loopPointer);
                    }
                    else{
                        while(loopPointer->next != loopPointer){
                            if ((loopPointer->next->timeMark > (LOWORD(lParam)-30)/20+timeShift*8)||(loopPointer->next->next == loopPointer->next)){
                                playlistInsertAfter((400-HIWORD(lParam)+ 40*toneShift)/10, (LOWORD(lParam)-30)/20+timeShift*8, currentNoteLength, loopPointer);
                                break;
                            }
                            loopPointer = loopPointer->next;
                        }
                    }

                }
            }
            else if ((LOWORD(lParam)>=30)&&(LOWORD(lParam)<670)&&(HIWORD(lParam)>400)&&(HIWORD(lParam)<410)){

                if (((ticker - timeShift*8)>=0) && ((ticker - timeShift*8)<32)&&(playOn)){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 9 + (ticker - timeShift*8)*20;
                    areaRect.right = 51 + (ticker - timeShift*8)*20;
                    InvalidateRect(hwnd, &areaRect, TRUE );
                    soundFactoryModify(0, SF_EMPTY, &theFactory);  
                }
                if (((savedTicker - timeShift*8)>=0) && ((savedTicker - timeShift*8)<32)&&(!(playOn))){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 9 + (savedTicker - timeShift*8)*20;
                    areaRect.right = 51 + (savedTicker - timeShift*8)*20;
                    InvalidateRect(hwnd, &areaRect, TRUE );
                }

                areaRect.top = 399;
                areaRect.bottom = 411; 
                areaRect.left = ((LOWORD(lParam)-30)/20)*20 + 29;
                areaRect.right = ((LOWORD(lParam)-30)/20)*20 + 51;
                InvalidateRect(hwnd, &areaRect, TRUE );

                savedTicker = (LOWORD(lParam)-30)/20 + timeShift*8;
                ticker = savedTicker;
                smallTicker = 0;
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 30,50,50,70)){                   //rewind
                
                soundFactoryModify(0, SF_EMPTY, &theFactory);
                savedTicker = 0;
                ticker = 0;
                timeShift = 0;
                areaRect.top = 99;
                areaRect.bottom = 411;
                areaRect.left = 29;
                areaRect.right = 671;
                InvalidateRect(hwnd, &areaRect, TRUE );
                for (i = 0; i < 4; i++){
                    uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 60,50,80,70)){                //play-pause
                if (((ticker - timeShift*8)>0) && ((ticker - timeShift*8)<32)&&(playOn)){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 9 + (ticker - timeShift*8)*20;
                    areaRect.right = 51 + (ticker - timeShift*8)*20;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
                if (playOn){
                    areaRect.top = 399;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 51 ;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    savedTicker = ticker;
                }
                playOn = 1 - playOn%2;
                
                ticker = savedTicker;
                smallTicker = 0;
                
                soundFactoryModify(0, SF_EMPTY, &theFactory);

                areaRect.top = 49;
                areaRect.bottom = 71;
                areaRect.left = 59;
                areaRect.right = 81;
                InvalidateRect(hwnd, &areaRect, TRUE );
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 90, 50, 110, 70)){            //navigate left
                if (timeShift > 0){
                    timeShift -= 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    for (i = 0; i < 4; i++){
                        uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                    }
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 130, 50, 150, 70)){           //navigate right
                if (timeShift < 12){
                    timeShift += 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                    for (i = 0; i < 4; i++){
                        uppdateArea(hwnd, 30+i*160, 415, 50+i*160, 440, 1);
                    }
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 110, 30, 130, 50)){            //navigate up
                if (toneShift < 21){
                    toneShift += 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam), 110, 70, 130, 90)){            //navigate down
                if (toneShift > 0){
                    toneShift -= 1;
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29 ;
                    areaRect.right = 30+20*32+1;
                    InvalidateRect( hwnd, &areaRect, TRUE );
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam),110, 50, 130, 70)){            //clear playlist
                if (MessageBox(hwnd, L"Are you sure you want to empty the playlist?",L"Question",MB_YESNO) == IDYES){

                    emptyPlaylist();
                    areaRect.top = 99;
                    areaRect.bottom = 411;
                    areaRect.left = 29;
                    areaRect.right = 671;
                    InvalidateRect(hwnd, &areaRect, TRUE );
                }
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam),270,45,290,60)){         //tempo up
                    bpm = bpm + 1;

                    InvalidateRect(hwnd, NULL, TRUE );
            }
            else if (mouseInRect(LOWORD(lParam),HIWORD(lParam),270,60,290,75)){         //tempo down
                    bpm = bpm - 1;

                    InvalidateRect(hwnd, NULL, TRUE );
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),190,79,350,99)){   
                for(i=0;i<8;i++){
                    if (mouseInRect(LOWORD(lParam),HIWORD(lParam),192+20*i,81,208+20*i,97)){
                        currentNoteLength = i+1;
                        InvalidateRect(hwnd, NULL, TRUE );
                    }
                }
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 405,45,425,60)){ //note length up
                currentNoteLength += 1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 405,60,425,75)){ //note length down
                if(currentNoteLength>1){
                    currentNoteLength -= 1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),30,75,100,95)){   //export
                if (exportWindowActive<1){
                    wc3.style         = CS_HREDRAW | CS_VREDRAW;
                    wc3.cbClsExtra    = 0;
                    wc3.cbWndExtra    = 0;
                    wc3.lpszClassName = L"PyrrosSynthExport";
                    wc3.hInstance     = thisInstance;
                    wc3.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
                    wc3.lpszMenuName  = NULL;
                    wc3.lpfnWndProc   = WndExportProc;
                    wc3.hCursor       = LoadCursor(NULL, IDC_ARROW);
                    wc3.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

                    RegisterClassW(&wc3);

                    hwnd3 = CreateWindowW(wc3.lpszClassName, L"Export Song",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                200, 200, 250, 130, NULL, NULL, thisInstance, NULL);  


                    ShowWindow(hwnd3, SW_SHOW);
                    UpdateWindow(hwnd3);
                    exportWindowActive = 1;
                }
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),30,25,100,45)){ //start MIDI IN device
                if (midiStarted > 0){
                    MessageBoxW(NULL, L"A MIDI IN device is already active.", L"Error", MB_OK);
                }
                if ((startMIDIINWindowActive<1)&&(midiStarted < 1)){
                    wc4.style         = CS_HREDRAW | CS_VREDRAW;
                    wc4.cbClsExtra    = 0;
                    wc4.cbWndExtra    = 0;
                    wc4.lpszClassName = L"MIDI_IN_device_selector";
                    wc4.hInstance     = thisInstance;
                    wc4.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
                    wc4.lpszMenuName  = NULL;
                    wc4.lpfnWndProc   = WndStartMIDIINProc;
                    wc4.hCursor       = LoadCursor(NULL, IDC_ARROW);
                    wc4.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

                    RegisterClassW(&wc4);

                    hwnd4 = CreateWindowW(wc4.lpszClassName, L"MIDI IN device selector",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                200, 160, START_MIDI_WINDOW_WIDTH, START_MIDI_WINDOW_HEIGHT, NULL, NULL, thisInstance, NULL);  


                    ShowWindow(hwnd4, SW_SHOW);
                    UpdateWindow(hwnd4);
                    startMIDIINWindowActive = 1;
                }
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),360,77,450,97)){   //export
                if (loadSaveWindowActive<1){
                    wc5.style         = CS_HREDRAW | CS_VREDRAW;
                    wc5.cbClsExtra    = 0;
                    wc5.cbWndExtra    = 0;
                    wc5.lpszClassName = L"PyrrosSynthLoadSave";
                    wc5.hInstance     = thisInstance;
                    wc5.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
                    wc5.lpszMenuName  = NULL;
                    wc5.lpfnWndProc   = WndLoadSaveProc;
                    wc5.hCursor       = LoadCursor(NULL, IDC_ARROW);
                    wc5.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

                    RegisterClassW(&wc5);

                    hwnd5 = CreateWindowW(wc5.lpszClassName, L"Load/Save",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                450, 230, 250, 110, NULL, NULL, thisInstance, NULL);  


                    ShowWindow(hwnd5, SW_SHOW);
                    UpdateWindow(hwnd5);
                    loadSaveWindowActive = 1;
                    InvalidateRect( hwnd, NULL, TRUE );
                }
            }
            break;
        case WM_RBUTTONUP:
            loopPointer = playlistHead;
            while(loopPointer->next->next != loopPointer->next){
                if ((loopPointer->next->tone == (400-HIWORD(lParam)+ 40*toneShift)/10)&&(loopPointer->next->timeMark <= (LOWORD(lParam)-30)/20+timeShift*8)&&(loopPointer->next->timeMark + loopPointer->next->length - 1 >= (LOWORD(lParam)-30)/20+timeShift*8)){


                    areaRect.left = (loopPointer->next->timeMark  - timeShift*8)*20 + 29;
                    areaRect.right = (loopPointer->next->timeMark + loopPointer->next->length -1 - timeShift*8)*20 + 51;

                    areaRect.top = 389 - ((400-HIWORD(lParam))/10)*10;
                    areaRect.bottom = 401 - ((400-HIWORD(lParam))/10)*10;
                    InvalidateRect( hwnd, &areaRect, TRUE );

                    playlistDeleteNext(loopPointer);


                    break;
                }
                else{
                    loopPointer = loopPointer->next;
                }
            }
            break;

        case WM_PAINT:
            theGraphics(hwnd);
            break;

        case WM_DESTROY:
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutUnprepareHeader(hWaveOut, &header2, sizeof(WAVEHDR));

            waveOutClose(hWaveOut);

            PostQuitMessage(0);
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}



void theGraphics(HWND hwnd){

    PAINTSTRUCT ps;
    double y = 0;
    RECT r;

    int yUpper;
    int yLower;

    HDC hdc = BeginPaint(hwnd, &ps);

    HPEN hPenBlack = CreatePen(PS_SOLID, 1, RGB(0,0,0));
    HPEN hPenRed = CreatePen(PS_SOLID, 1, RGB(255,0,0));
    HPEN hPenLightBlue = CreatePen(PS_SOLID, 1, RGB(100,100,255));
    HPEN hPenDarkPink = CreatePen(PS_SOLID, 1, RGB(0,0,250));
    HPEN hPenInvisible = CreatePen(PS_NULL, 1, RGB(0,0,0));
    HPEN holdPen = SelectObject(hdc, hPenBlack);

    HBRUSH hBrush1 = CreateSolidBrush(RGB(255, 0, 255));
    HBRUSH hBrushGreen = CreateSolidBrush(RGB(150, 200, 150));
    HBRUSH hBrushLightBlue = CreateSolidBrush(RGB(150, 150, 255));
    HBRUSH hBrushLighterBlue = CreateSolidBrush(RGB(200, 200, 255));
    HBRUSH hBrushBlue = CreateSolidBrush(RGB(0, 0, 255));
    HBRUSH hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
//    HBRUSH hBrushPink = CreateSolidBrush(RGB(250,50,250));
//    HBRUSH hBrushDarkPink = CreateSolidBrush(RGB(100,0,150));
    HBRUSH hBrushPink = CreateSolidBrush(RGB(0,200,250));
    HBRUSH hBrushDarkPink = CreateSolidBrush(RGB(0,0,250));

    HBRUSH hBrushLightPink = CreateSolidBrush(RGB(200,150,250));
//    HBRUSH hBrushBackground = CreateSolidBrush(RGB(255,105,0));
    HBRUSH hBrushBackground = CreateSolidBrush(RGB(255,255,0));

    HBRUSH hBrushButtonMarked = CreateSolidBrush(RGB(220,220,255));

    HGDIOBJ original = NULL;



    HBRUSH holdBrush = SelectObject(hdc, hBrush1);


    HFONT hfont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"NSimSun");
    HFONT hfont2 = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Cambria");
    HFONT hfont3 = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"NSimSun");

    HFONT oldHFont;

  
    //Drawing -------------------------------------

    SelectObject(hdc, hPenInvisible);
    //SelectObject(hdc, hBrushBackground);
    SelectObject(hdc, hBrushWhite);
    Rectangle(hdc, 0, 0, WINDOWWIDTH, WINDOWHEIGHT);

    //SelectObject(hdc, hPenBlack);
    //MoveToEx(hdc, 0, 0, NULL);
    //LineTo(hdc, WINDOWWIDTH, 0);   
    //SelectObject(hdc, hPenInvisible);


    //Brand name area

    original = SelectObject(hdc,GetStockObject(DC_PEN));
/*
    for(i=0;i<50;i++){
        //SetDCPenColor(hdc, RGB(255-2*i,255-5*i,0));
        SetDCPenColor(hdc, RGB(255,255-3*i,0));

        MoveToEx(hdc, 1, 1+i, NULL);
        LineTo(hdc, 1100, 1+i);    
    }
    for(i=0;i<50;i++){
        //SetDCPenColor(hdc, RGB(255-2*i,255-5*i,0));
        SetDCPenColor(hdc, RGB(255,255-3*i,0));
        MoveToEx(hdc, 1, 100-i, NULL);
        LineTo(hdc, 1100, 100-i);    
    }
*/
 
    SelectObject(hdc, hfont2);
    SetTextColor(hdc,RGB(255,0,0));
    SetBkMode(hdc,TRANSPARENT);

    r.top = 1;
    r.left = 10;
    r.bottom = 25;
    r.right = 250;
    //DrawText(hdc, L"Pyrros Synth Host", 17, &r,DT_LEFT);

    //-------------------------------------

    SelectObject(hdc, hPenInvisible);
    SelectObject(hdc, hBrushLightBlue);

    //for (i = 0; i < 14; i++){
    //    Rectangle(hdc, 30, 381-i*20, 641+30, 360-i*20);
    //}

    for (i = 0; i < 10; i++){
        yUpper = 371 - 120*i + toneShift*40 + 20;
        yLower = 360 - 120*i + toneShift*40 + 20;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 40;
        yLower = 360 - 120*i + toneShift*40 + 40;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 60;
        yLower = 360 - 120*i + toneShift*40 + 60;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 90;
        yLower = 360 - 120*i + toneShift*40 + 90;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 110;
        yLower = 360 - 120*i + toneShift*40 + 110;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
    }


    SelectObject(hdc, hBrushLighterBlue);
    //for (i = 0; i < 15; i++){
    //    Rectangle(hdc, 30, 391-i*20, 641+30, 380-i*20);
    //}

    for (i = 0; i < 10; i++){
        yUpper = 371 - 120*i + toneShift*40 + 10;
        yLower = 360 - 120*i + toneShift*40 + 10;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 30;
        yLower = 360 - 120*i + toneShift*40 + 30;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 50;
        yLower = 360 - 120*i + toneShift*40 + 50;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 70;
        yLower = 360 - 120*i + toneShift*40 + 70;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 80;
        yLower = 360 - 120*i + toneShift*40 + 80;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
        yUpper = 371 - 120*i + toneShift*40 + 100;
        yLower = 360 - 120*i + toneShift*40 + 100;
        if ((yUpper<400)&&(yLower>99)){
            Rectangle(hdc, 30, yUpper, 641+30, yLower);
        }
    }

    SelectObject(hdc, hBrushLightPink);
    Rectangle(hdc, 30, 401, 641+30, 390);

    SelectObject(hdc, hBrushWhite);
    Rectangle(hdc, 30, 400, 641+30, 411);


    SelectObject(hdc, hBrushGreen);
    for (i = 0; i < 10; i++){
        if (((371 - 120*i + toneShift*40)<400)&&((361 - 120*i + toneShift*40)>100)){
            Rectangle(hdc, 30, 371 - 120*i + toneShift*40, 641+30, 360 - 120*i + toneShift*40);
        }
    }


    SelectObject(hdc, hPenLightBlue);
    for (i = 0; i < 17; i++){
        MoveToEx(hdc, 30 + i*40, 100, NULL);
        LineTo(hdc, 30 + i*40, 390);
    }


    //SelectObject(hdc, hPenInvisible);
    SelectObject(hdc, hPenDarkPink);
    SelectObject(hdc, hBrushPink);


    SelectObject(hdc, hPenBlack);
    for (i = 0; i < 5; i++){
        MoveToEx(hdc, 30 + i*160, 100, NULL);
        LineTo(hdc, 30 + i*160, 399);
    }
    SelectObject(hdc, hPenLightBlue);

    //Draw the tones in the playlist
    loopPointer = playlistHead;
    while(loopPointer->next != loopPointer){
        if (loopPointer != playlistHead){
            if ((loopPointer->timeMark - timeShift*8 < 32) && (loopPointer->timeMark - timeShift*8 >= 0)){
                if (((400-(loopPointer->tone)*10 + 40*toneShift)<391)&&((390-(loopPointer->tone)*10 + 40*toneShift)>99)){
                    if (loopPointer->timeMark + loopPointer->length - timeShift*8 > 31){
                        Rectangle(hdc, 30+(loopPointer->timeMark - timeShift*8)*20, 390-(loopPointer->tone)*10 + 40*toneShift, 51 + 31*20, 401-(loopPointer->tone)*10 + 40*toneShift);
                    }
                    else{
                        Rectangle(hdc, 30+(loopPointer->timeMark - timeShift*8)*20, 390-(loopPointer->tone)*10 + 40*toneShift, 51+(loopPointer->timeMark + loopPointer->length -1 - timeShift*8)*20, 401-(loopPointer->tone)*10 + 40*toneShift);
                    }
                    SelectObject(hdc, hBrushDarkPink);
                    SelectObject(hdc, hPenInvisible);
                    Rectangle(hdc, 30+(loopPointer->timeMark - timeShift*8)*20, 390-(loopPointer->tone)*10 + 40*toneShift, 35+(loopPointer->timeMark - timeShift*8)*20, 401-(loopPointer->tone)*10 + 40*toneShift);
                    SelectObject(hdc, hBrushPink);
                    SelectObject(hdc, hPenDarkPink);
                }

            }
        }
        loopPointer = loopPointer->next;
    }
    SelectObject(hdc, hPenInvisible);

    SelectObject(hdc, hBrushBlue);
    if (((ticker - timeShift*8)>=0) && ((ticker - timeShift*8)<32)&&playOn){
        Rectangle(hdc, 30 + (ticker - timeShift*8)*20, 400, 50 + (ticker - timeShift*8)*20, 411);
    }
    else if((!(playOn))&&((savedTicker - timeShift*8)>=0) && ((savedTicker - timeShift*8)<32)){
        Rectangle(hdc, 30 + (savedTicker - timeShift*8)*20, 400, 50 + (savedTicker - timeShift*8)*20, 411);
    }

    SelectObject(hdc, hPenBlack);
//    for (i = 0; i < 5; i++){
//        MoveToEx(hdc, 30 + i*160, 100, NULL);
//        LineTo(hdc, 30 + i*160, 399);
//    }


    MoveToEx(hdc, 30, 100, NULL);
    LineTo(hdc, 30 , 410);
    LineTo(hdc, 30+8*80 , 410);
    LineTo(hdc, 30+8*80 , 100);
    LineTo(hdc, 30, 100);





    //rewind button
    SelectObject(hdc, hPenBlack);
    
    
    if (markedButton == ID_BUTTON_REWIND){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }

    Rectangle(hdc, 30,50,50,70);

    for(i = 0; i < 6; i++){
        MoveToEx(hdc, 38-i, 55+i, NULL);
        LineTo(hdc, 38-i, 65-i);
        MoveToEx(hdc, 44-i, 55+i, NULL);
        LineTo(hdc, 44-i, 65-i);
    }
 
    //play pause-button
    if (markedButton == ID_BUTTON_PLAY){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 60,50,80,70);

    if(!playOn){
        for(i = 0; i < 8; i++){
            MoveToEx(hdc, 67+i, 53+i, NULL);
            LineTo(hdc, 67+i, 67-i);
        }
    }
    else{
        SelectObject(hdc, hBrushBlack);
        Rectangle(hdc, 64,54,68,66);
        Rectangle(hdc, 72,54,76,66);
    }

    //navigation
    if (markedButton == ID_BUTTON_NAVLEFT){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 90, 50, 110, 70);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 100-i, 55+i, NULL);
        LineTo(hdc, 100-i, 65-i);    
    }
    

    if (markedButton == ID_BUTTON_NAVRIGHT){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 130, 50, 150, 70);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 139+i, 55+i, NULL);
        LineTo(hdc, 139+i, 65-i);    
    }

    if (markedButton == ID_BUTTON_NAVUP){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 110, 30, 130, 50);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 115+i, 41-i, NULL);
        LineTo(hdc, 125-i, 41-i);    
    }

    if (markedButton == ID_BUTTON_NAVDOWN){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 110, 70, 130, 90);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 115+i, 79+i, NULL);
        LineTo(hdc, 125-i, 79+i);    
    }

    //Clear playlist
    if (markedButton == ID_BUTTON_CLEARPLAYLIST){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 110, 50, 130, 70);
    SelectObject(hdc, hPenRed);
        MoveToEx(hdc, 114, 54, NULL);
        LineTo(hdc, 126, 66); 
        MoveToEx(hdc, 115, 54, NULL);
        LineTo(hdc, 126, 65); 
        MoveToEx(hdc, 114, 55, NULL);
        LineTo(hdc, 125, 66); 

        MoveToEx(hdc, 126, 54, NULL);
        LineTo(hdc, 114, 66); 

        MoveToEx(hdc, 125, 54, NULL);
        LineTo(hdc, 114, 67); 
        MoveToEx(hdc, 126, 55, NULL);
        LineTo(hdc, 115, 66); 



    //Tempo, show and up-down
    SelectObject(hdc, hBrushWhite);
    SelectObject(hdc, hPenBlack);
    Rectangle(hdc, 165,45,270,75);

    SetBkMode(hdc,TRANSPARENT);
    SetTextColor(hdc,RGB(0,0,255));

    r.top = 50;
    r.left = 170;
    r.bottom = 75;
    r.right = 230;

    oldHFont = (HFONT)SelectObject(hdc, hfont);

    DrawText(hdc, L"Tempo:", 6, &r,DT_LEFT);

    r.top = 50;
    r.left = 230;
    r.bottom = 75;
    r.right = 270;

    _itow(bpm, charBuffer,10);
    DrawText(hdc,charBuffer, 10, &r, DT_LEFT);


    SelectObject(hdc, oldHFont);

    if (markedButton == ID_BUTTON_TEMPOUP){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 270,45,290,60);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 275+i, 55-i, NULL);
        LineTo(hdc, 285-i, 55-i);    
    }

    if (markedButton == ID_BUTTON_TEMPODOWN){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 270,60,290,75);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 275+i, 65+i, NULL);
        LineTo(hdc, 285-i, 65+i);    
    }



    //Tone-length
    SelectObject(hdc, hBrushWhite);
    SelectObject(hdc, hPenBlack);
    for(i=0;i<8;i++){
        Rectangle(hdc, 192+20*i, 81, 208+20*i, 97);
    }

    SelectObject(hdc, hPenInvisible);
    SelectObject(hdc, hBrushBlack);

    if (currentNoteLength < 9){
        Rectangle(hdc, 192+20*(currentNoteLength-1), 81, 208+20*(currentNoteLength-1), 97);
    }


    SelectObject(hdc, hPenBlack);
    SelectObject(hdc, hBrushWhite);
    Rectangle(hdc, 300,45,405,75);

    SetBkMode(hdc,TRANSPARENT);
    SetTextColor(hdc,RGB(0,0,255));

    r.top = 50;
    r.left = 305;
    r.bottom = 75;
    r.right = 375;

    oldHFont = (HFONT)SelectObject(hdc, hfont);

    DrawText(hdc, L"Length:", 7, &r,DT_LEFT);

    r.top = 50;
    r.left = 380;
    r.bottom = 75;
    r.right = 400;

    _itow(currentNoteLength, charBuffer,10);
    DrawText(hdc,charBuffer, 10, &r, DT_LEFT);


    if (markedButton == ID_BUTTON_LENGTHUP){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 405,45,425,60);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 410+i, 55-i, NULL);
        LineTo(hdc, 420-i, 55-i);    
    }

    if (markedButton == ID_BUTTON_LENGTHDOWN){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 405,60,425,75);
    for(i=0;i<5;i++){
        MoveToEx(hdc, 410+i, 65+i, NULL);
        LineTo(hdc, 420-i, 65+i);    
    }


    //Export button
    if (markedButton == ID_BUTTON_EXPORT){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 30,75,100,95);

    SelectObject(hdc, hfont3);

    r.top = 76;
    r.left = 31;
    r.bottom = 94;
    r.right = 99;

    DrawText(hdc, L"Export", 6, &r,DT_CENTER);


    //Start MIDI IN button
    if (markedButton == ID_BUTTON_STARTMIDIIN){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 30,25,100,45);

    SelectObject(hdc, hfont3);

    r.top = 26;
    r.left = 31;
    r.bottom = 44;
    r.right = 99;

    DrawText(hdc, L"MIDI IN", 7, &r,DT_CENTER);


    //LoadSave button
    if (markedButton == ID_BUTTON_LOADSAVE){
        SelectObject(hdc, hBrushButtonMarked);
    }
    else{
        SelectObject(hdc, hBrushWhite);
    }
    Rectangle(hdc, 360,77,450,97);

    SelectObject(hdc, hfont3);

    r.top = 78;
    r.left = 361;
    r.bottom = 96;
    r.right = 449;

    DrawText(hdc, L"Load/Save", 9, &r,DT_CENTER);



    SelectObject(hdc, hBrushWhite);

    //Number for beats
    SelectObject(hdc, hfont);

    for (i = 0; i < 4;i++){
        r.top = 415;
        r.left = 30+i*160;  //160 per takt
        r.bottom = 440;
        r.right = 50+i*160;

        _itow((timeShift+i+1), charBuffer,10);
        DrawText(hdc,charBuffer, 10, &r, DT_LEFT);
    }


    SelectObject(hdc, oldHFont);

    //Cleaning --------------------------------------
    SelectObject(hdc, holdPen);  //MEMORY LEAK?????  ta bort dessa också eller? Uppdatering: tror fixat detta nu.
    SelectObject(hdc, holdBrush);


    DeleteObject(hPenBlack);
    DeleteObject(hPenRed);
    DeleteObject(hPenLightBlue);
    DeleteObject(hPenDarkPink);
    DeleteObject(hPenInvisible);
    DeleteObject(holdPen);

    DeleteObject(hBrush1);
    DeleteObject(hBrushGreen);
    DeleteObject(hBrushLightBlue);
    DeleteObject(hBrushLighterBlue);
    DeleteObject(hBrushBlue);
    DeleteObject(hBrushWhite);
    DeleteObject(hBrushBlack);
    DeleteObject(hBrushPink);
    DeleteObject(hBrushDarkPink);
    DeleteObject(hBrushLightPink);
    DeleteObject(hBrushBackground);
    DeleteObject(holdBrush);
    DeleteObject(hBrushButtonMarked);


    DeleteObject(hfont);
    DeleteObject(hfont2);
    DeleteObject(oldHFont);
    DeleteObject(hfont3);


    DeleteObject(original);

    EndPaint(hwnd, &ps);
}



void fillBuffer(short *bf){
    int li = 0;
    static int doPlay = 0;

    for (li = 0; li < BUFFERSIZE; li++){
        if (playOn){
            smallTicker += 1;
            if (smallTicker == (int)(44100*60/((double)bpm*beatres))){
 
                loopPointer = playlistHead;
                while(loopPointer->next != loopPointer){
                    if (loopPointer->timeMark == ticker){                                                                  
                       soundFactoryModify(loopPointer->tone, SF_TURN_ON, &theFactory);
                    }
                    else if(loopPointer->timeMark + loopPointer->length == ticker){
                       soundFactoryModify(loopPointer->tone, SF_TURN_OFF, &theFactory);
                    } 
                    loopPointer = loopPointer->next;
                }
            
                ticker += 1;

                smallTicker = 0;
                if (ticker == playlistLength){
                    ticker = 0;
                }
            }
        }
        bf[li] = 0;
        bf[li] += soundFactoryProduce(&theFactory);
    }
}



int mouseInRect(int mouseX, int mouseY, int rectX1, int rectY1, int rectX2, int rectY2){
    if ((mouseX > rectX1)&&(mouseX < rectX2)&&(mouseY < rectY2)&&(mouseY > rectY1)){
        return 1;
    }
    else{
        return 0;
    }
}

void exportToWav(int nBeats, char * fileName){
    unsigned int chunkSize;  // = (BUFFER_SIZE*2 + 36)
    unsigned int subchunk1size = 16;
    unsigned short audioFormat = 1;     //PCM
    unsigned short nChannels = 1;       //Mono
    unsigned int sampleRate = 44100;
    unsigned int byteRate = 88200;
    unsigned short blockAlign = 2;
    unsigned short bitsPerSample = 16;
    unsigned int subchunk2size; // 2*BUFFER_SIZE

    FILE * pFile;
    int li = 0;
    short buffer16;

    chunkSize = (44100*60/bpm*nBeats)*2 + 36;
    subchunk2size = (44100*60/bpm*nBeats)*2;

    pFile = fopen(fileName, "wb");


    fwrite("RIFF", 1, 4, pFile);
    fwrite(&chunkSize, 4, 1, pFile);      
    fwrite("WAVE", 1, 4, pFile);
    fwrite("fmt ", 1, 4, pFile);
    fwrite(&subchunk1size, 4, 1, pFile);      
    fwrite(&audioFormat, 2, 1, pFile);
    fwrite(&nChannels, 2, 1, pFile);
    fwrite(&sampleRate, 4, 1, pFile);
    fwrite(&byteRate, 4, 1, pFile);
    fwrite(&blockAlign, 2, 1, pFile);
    fwrite(&bitsPerSample, 2, 1, pFile);
    fwrite("data", 1, 4, pFile);
    fwrite(&subchunk2size, 4, 1, pFile);

    soundFactoryModify(0, SF_EMPTY, &theFactory);
    smallTicker = 0;
    ticker = 0;
    playOn = 0;



    for (li = 0; li < (44100*60/bpm*nBeats); li++){
            smallTicker += 1;
            if (smallTicker == (int)(44100*60/((double)bpm*beatres))){
 
                loopPointer = playlistHead;
                while(loopPointer->next != loopPointer){
                    if (loopPointer->timeMark == ticker){                                                                  
                       soundFactoryModify(loopPointer->tone, SF_TURN_ON, &theFactory);
                    }
                    else if(loopPointer->timeMark + loopPointer->length == ticker){
                       soundFactoryModify(loopPointer->tone, SF_TURN_OFF, &theFactory);
                    } 
                    loopPointer = loopPointer->next;
                }
            
                ticker += 1;

                smallTicker = 0;
                if (ticker == playlistLength){
                    ticker = 0;
                }
            }
        buffer16 = 0;
        buffer16 += soundFactoryProduce(&theFactory);
        fwrite(&buffer16, 2, 1, pFile);
    }


    fclose(pFile);
    soundFactoryModify(0, SF_EMPTY, &theFactory);
}


LRESULT CALLBACK WndExportProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    WCHAR editInputBuffer[20]; 
    char editCharInputBuffer[20];
    WORD cchText;

    static HWND hwndEditFileName;
    static HWND hwndEditNBeats;

    int nBeats;

    switch(uMsg){

        case WM_CREATE:
            hwndEditFileName = CreateWindowEx(0, L"EDIT",L"fileName.wav", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOVSCROLL, 120, 10, 100, 20, 
                                      hwnd,(HMENU) ID_EDIT_FILENAME,(HINSTANCE) GetWindowLong(hwnd3, GWL_HINSTANCE), NULL); 

            hwndEditNBeats = CreateWindowEx(0, L"EDIT",L"4", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOVSCROLL | ES_NUMBER, 120, 35, 50, 20, 
                                      hwnd,(HMENU) ID_EDIT_NBEATS,(HINSTANCE) GetWindowLong(hwnd3, GWL_HINSTANCE), NULL); 

            CreateWindowW(L"Static", L"File Name: ", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          10, 10, 100, 20, hwnd, (HMENU) 1, NULL, NULL);
            CreateWindowW(L"Static", L"No. Beats:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          10, 35, 100, 20, hwnd, (HMENU) 1, NULL, NULL);

 

            CreateWindowW(L"Button", L"Export", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          10, 60, 210, 25, hwnd, (HMENU)ID_BUTTON_EXPORTNOW, NULL, NULL);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_EXPORTNOW){
                //get number of beats from inputbox
                cchText = (WORD)SendDlgItemMessage(hwnd3, ID_EDIT_NBEATS,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
                *((LPWORD)editInputBuffer) = cchText;
                SendDlgItemMessage(hwnd3, ID_EDIT_NBEATS, EM_GETLINE, (WPARAM)0, (LPARAM)editInputBuffer);
                editInputBuffer[cchText] = 0;
                nBeats = _wtoi(editInputBuffer);

                if ((nBeats<17)&&(nBeats > 0)){
                    //get filename from inputbox
                    cchText = (WORD)SendDlgItemMessage(hwnd3, ID_EDIT_FILENAME,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
                    *((LPWORD)editInputBuffer) = cchText;
                    SendDlgItemMessage(hwnd3, ID_EDIT_FILENAME, EM_GETLINE, (WPARAM)0, (LPARAM)editInputBuffer);
                    editInputBuffer[cchText] = 0;

                    for (i = 0; i<20; i++){
                        editCharInputBuffer[i] = (char)editInputBuffer[i];
                    }

                    exportToWav(nBeats, editCharInputBuffer);
                    InvalidateRect(hwnd, NULL, TRUE );
                    //MessageBoxW(NULL, L"Song exported!", L"Success!!!!!", MB_OK);
                    exportWindowActive = 0;
                    ShowWindow(hwnd3, SW_HIDE);
                    UpdateWindow(hwnd3);
                }
                else{
                    MessageBoxW(NULL, L"Number of beats must be a number between 1 and 16.", L"Failure", MB_OK);
                }
            }
            break;

        case WM_DESTROY:
                exportWindowActive = 0;
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndStartMIDIINProc(HWND hwnd, UINT msg, 
    WPARAM wParam, LPARAM lParam) {

    switch(msg) {
        case WM_CREATE:

            break;

        case WM_MOUSEMOVE:
               markedButtonOld2 = markedButton2;
               if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 10, 10, 160, 30)){
                   markedButton2 = ID_BUTTON_STARTMIDI;
               }
               else{
                   markedButton2 = 0;
               }
               if (markedButtonOld2 != markedButton2){
                   uppdateArea(hwnd,  9, 9, 161, 31, 1);
               }
            break;
        case WM_LBUTTONUP:
            activeMIDIDevices = midiInGetNumDevs();
            for (i = 0; i < activeMIDIDevices; i++){
                if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 0, 40+i*16, START_MIDI_WINDOW_WIDTH, 56+i*16)){
                    currentItem = i;
                    uppdateArea(hwnd,  0, 0, 30, START_MIDI_WINDOW_HEIGHT, 1);
                }
            }
            if (markedButton2 == ID_BUTTON_STARTMIDI){
                //open the given device
                //MessageBoxW(NULL, L"blablabla", L"blabla...", MB_OK);
                if (activeMIDIDevices > 0){
                    midiDeviceNumber = currentItem;
                    doStartMIDI = 1;
                }
                else{
                    MessageBoxW(NULL, L"No active MIDI IN devices found.", L"Error", MB_OK);
                }
                startMIDIINWindowActive = 0;
                ShowWindow(hwnd4, SW_HIDE);
                UpdateWindow(hwnd4);
            }
            break;
        case WM_PAINT:
            theStartMIDIGraphics(hwnd);
            break;   
        case WM_DESTROY:
            startMIDIINWindowActive = 0;
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}  


void theStartMIDIGraphics(HWND hwnd){

    PAINTSTRUCT ps;
    double y = 0;
    RECT r;
    int j = 0;


    HDC hdc = BeginPaint(hwnd, &ps);

    HBRUSH hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH hBrushLightBlue = CreateSolidBrush(RGB(200, 200, 255));


    HPEN hPenBlack = CreatePen(PS_SOLID, 1, RGB(0,0,0));
    HPEN hPenInvisible = CreatePen(PS_NULL, 1, RGB(0,0,0));


    HFONT hfont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"NSimSun");
    HFONT hfont2 = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Cambria");

    HFONT oldHFont;

    SelectObject(hdc, hPenInvisible);
    SelectObject(hdc, hBrushWhite);

    Rectangle(hdc, 0, 0, START_MIDI_WINDOW_WIDTH, START_MIDI_WINDOW_HEIGHT);

    SetTextColor(hdc,RGB(0,0,255));
    SetBkMode(hdc,TRANSPARENT);

    oldHFont = (HFONT)SelectObject(hdc, hfont2);
    

    activeMIDIDevices = midiInGetNumDevs();
    for (j = 0; j < activeMIDIDevices; j++){
        midiInGetDevCaps(j, &theInfo, sizeof(MIDIINCAPS));
        theString = theInfo.szPname;
        r.top = 40+j*16;
        r.left = 30;
        r.bottom = 60+j*16;
        r.right = START_MIDI_WINDOW_WIDTH;
        DrawText(hdc, theString, -1, &r,DT_LEFT);
    }
    
    if (activeMIDIDevices < 1){
        r.top = 40;
        r.left = 30;
        r.bottom = 60;
        r.right = START_MIDI_WINDOW_WIDTH;
        DrawText(hdc, L"No MIDI IN devices detected", 27, &r,DT_LEFT);
    }  


    SelectObject(hdc, hPenBlack);
    SelectObject(hdc, hBrushWhite);
    if (markedButton2 == ID_BUTTON_STARTMIDI){
        SelectObject(hdc, hBrushLightBlue);
    }

    Rectangle(hdc, 10, 10, 160, 30);

    r.top = 11;
    r.left = 20;
    r.bottom = 24;
    r.right = 159;
    SetTextColor(hdc,RGB(0,0,0));
    DrawText(hdc, L"Start MIDI IN device", 20, &r,DT_LEFT);
  


    SelectObject(hdc, hBrushBlack);
    if (activeMIDIDevices > 0){
        Rectangle(hdc, 16, 44+currentItem*16, 24, 52+currentItem*16);
    }

    DeleteObject(hBrushWhite);
    DeleteObject(hBrushBlack);
    DeleteObject(hBrushLightBlue);

    DeleteObject(hPenBlack);
    DeleteObject(hPenInvisible);

    DeleteObject(hfont);
    DeleteObject(hfont2);
    DeleteObject(oldHFont);

    EndPaint(hwnd, &ps);
}

int savePlaylist(char * fileName){

    FILE * pFile;
    pFile = fopen(fileName, "wb");
    loopPointer = playlistHead;
    while(loopPointer->next != loopPointer){
        fwrite(&(loopPointer->next->tone), 1, sizeof(int), pFile);
        fwrite(&(loopPointer->next->timeMark), 1, sizeof(int), pFile);
        fwrite(&(loopPointer->next->length), 1, sizeof(int), pFile);

        loopPointer = loopPointer->next;
    }

    fclose(pFile);
    return(0);
}

int loadPlaylist(char * fileName, int option){

    FILE * pFile;
    size_t isEOF = 0;
    int endReached = 0;

    int bufferTone = 0;
    int bufferTimeMark = 0;
    int bufferLength = 0;

    pFile = fopen(fileName, "rb");
    if (pFile == NULL){ //Error opening file, then just quit 
        MessageBoxW(NULL, L"File not found.", L"Error", MB_OK);
        return(1);
    }

    if (option < 1){
        emptyPlaylist();
    }

    while(endReached < 1){
        isEOF = fread(&bufferTone, 1, sizeof(int), pFile);
        if (isEOF != sizeof(int)){
            endReached = 1;
        }
        isEOF = fread(&bufferTimeMark, 1, sizeof(int), pFile);
        if (isEOF != sizeof(int)){
            endReached = 1;
        }
        isEOF = fread(&bufferLength, 1, sizeof(int), pFile);
        if (isEOF != sizeof(int)){
            endReached = 1;
        }
        if (endReached < 1){
            playlistInsertAfter(bufferTone, bufferTimeMark, bufferLength, playlistHead);
        }
    }

    fclose(pFile);
    return(0);
}

LRESULT CALLBACK WndLoadSaveProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    WCHAR editInputBuffer[20]; 
    char editCharInputBuffer[20];
    WORD cchText;

    static HWND hwndEditFileName;

    switch(uMsg){

        case WM_CREATE:
            hwndEditFileName = CreateWindowEx(0, L"EDIT",L"fileName.tne", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOVSCROLL, 120, 10, 100, 20, 
                                      hwnd,(HMENU) ID_EDIT_FILENAME2,(HINSTANCE) GetWindowLong(hwnd5, GWL_HINSTANCE), NULL); 

            CreateWindowW(L"Static", L"File Name: ", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          10, 10, 100, 20, hwnd, (HMENU) 1, NULL, NULL);


            CreateWindowW(L"Button", L"Save", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          10, 40, 70, 25, hwnd, (HMENU)ID_BUTTON_SAVE, NULL, NULL);
            CreateWindowW(L"Button", L"Load", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          80, 40, 70, 25, hwnd, (HMENU)ID_BUTTON_LOAD, NULL, NULL);
            CreateWindowW(L"Button", L"Add", WS_CHILD | WS_VISIBLE | SS_LEFT,
                          150, 40, 70, 25, hwnd, (HMENU)ID_BUTTON_ADD, NULL, NULL);

            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_SAVE){
                //get filename from inputbox
                cchText = (WORD)SendDlgItemMessage(hwnd5, ID_EDIT_FILENAME2,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
                *((LPWORD)editInputBuffer) = cchText;
                SendDlgItemMessage(hwnd5, ID_EDIT_FILENAME2, EM_GETLINE, (WPARAM)0, (LPARAM)editInputBuffer);
                editInputBuffer[cchText] = 0;

                for (i = 0; i<20; i++){
                    editCharInputBuffer[i] = (char)editInputBuffer[i];
                }

                savePlaylist(editCharInputBuffer);
                //MessageBoxW(NULL, L"Song saved.", L"Success!!!!!", MB_OK);
                loadSaveWindowActive = 0;
                ShowWindow(hwnd5, SW_HIDE);
                UpdateWindow(hwnd5);                
            }
            else if ((LOWORD(wParam) == ID_BUTTON_LOAD)||(LOWORD(wParam) == ID_BUTTON_ADD)){
                //get filename from inputbox
                cchText = (WORD)SendDlgItemMessage(hwnd5, ID_EDIT_FILENAME2,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
                *((LPWORD)editInputBuffer) = cchText;
                SendDlgItemMessage(hwnd5, ID_EDIT_FILENAME2, EM_GETLINE, (WPARAM)0, (LPARAM)editInputBuffer);
                editInputBuffer[cchText] = 0;

                for (i = 0; i<20; i++){
                    editCharInputBuffer[i] = (char)editInputBuffer[i];
                }
                
                if (LOWORD(wParam) == ID_BUTTON_LOAD){
                    loadPlaylist(editCharInputBuffer, 0);
                }
                else{
                    loadPlaylist(editCharInputBuffer, 1);
                }
                doUpdate = 1;
                //MessageBoxW(NULL, L"Song saved.", L"Success!!!!!", MB_OK);
                loadSaveWindowActive = 0;
                ShowWindow(hwnd5, SW_HIDE);
                UpdateWindow(hwnd5);                
            }

            break;

        case WM_DESTROY:
                loadSaveWindowActive = 0;
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void uppdateArea(HWND hwnd, int x1, int y1, int x2, int y2, int margin){
    RECT theRect;

    theRect.top = y1-margin;
    theRect.bottom = y2+margin;
    theRect.left = x1-margin;
    theRect.right = x2+margin;
    InvalidateRect( hwnd, &theRect, TRUE );
}



//linked-list functions---------------------------------------------------------------------

void playlistInsertAfter(int v1, int v2, int v3, struct plNode *t){
    struct plNode *x;
    x = (struct plNode *)malloc(sizeof *x);
    x->tone = v1;
    x->timeMark = v2;
    x->length = v3;
    x->next = t->next;

    t->next = x;
}

void playlistDeleteNext(struct plNode *t){
    struct plNode *x;
    x = t->next;
    t->next = t->next->next;
    free(x);
}


void emptyPlaylist(void){
    loopPointer = playlistHead;
    while(loopPointer->next->next != loopPointer->next){
        playlistDeleteNext(loopPointer);
    }
}


