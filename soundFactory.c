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

#define PI 3.14159265
#define WINDOWWIDTHSF 900
#define WINDOWHEIGHTSF 500

struct soundNode{
  double frequency;
  double phaseAngle;
  int id;

  double opA;
  double opB;
  double opC;
  double opD;
  double opE;
  double opF;

  double opAPhase;
  double opBPhase;
  double opCPhase;
  double opDPhase;
  double opEPhase;
  double opFPhase;

  double attack[7];
  double decay[7];
  double release[7];

  int releasedTrue;

  struct soundNode *next;
};

int opMatrix[36]; 
int opRatio[6]; 
int opWaveForm[6]; 
int opOut[6];

double initPhaseA = 0.0;
double initPhaseB = 0.2;
double initPhaseC = 0.4;
double initPhaseD = 0.6;
double initPhaseE = 0.8;
double initPhaseF = 1.0;

int paramRelease[7] = {100,100,100,100,100,100,400};
int paramDecay[7] = {100,100,100,100,100,100,300};
int paramSustain[7] = {100,100,100,100,100,100,70};
int paramAttack[7] = {100,100,100,100,100,100,200};
int envAactive[6] ={0,0,0,0,0,0};
int envRactive[6] ={0,0,0,0,0,0};

int i;
int j;

int convNum(int number, WCHAR buf[]);

int opSelectSource = 0;
int opSelectDest = 0;
int activeElement = 0;
int activeNumberPos = 0;
int activeRatioNumberPos = 0;
int activeADSRNumberPos = 0;


double waveFunction(int type, double input);
double sawtooth(double phaseAngle);
double squarewave(double phaseAngle);
double trianglewave(double phaseAngle);

double envelopeADSR(int activeA, int activeR, double a, double d, double r);

void loadPreset(int id);

void theGraphicsSF(HWND hwnd);
int mouseInRectSF(int mouseX, int mouseY, int rectX1, int rectY1, int rectX2, int rectY2);

double getFrequency(int tone);
void soundlistInsertAfter(double v1, double v2a, int v3, struct soundNode *t);
void soundlistDeleteNext(struct soundNode *t);

//EXTERNAL Window callback function -------------------------------------------------------

LRESULT CALLBACK soundFactoryCallbackWindow(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg){
        case WM_CREATE:
            SetWindowTextW(hwnd,L"Saccharin Synth");
            SetWindowPos(hwnd, NULL, 820, 100, WINDOWWIDTHSF, WINDOWHEIGHTSF, SWP_SHOWWINDOW);

            for (i=0; i<36; i++){
                opMatrix[i] = 0;
            }
            for (i=0; i<6; i++){
                opOut[i] = 0;
                opRatio[i] = 10000;
                opWaveForm[i] = 1;
            }
            opOut[5] = 100;
            break;
        case WM_KEYDOWN:
            if (!activeElement){
                if (wParam == VK_LEFT){
                    if ((opSelectSource == 8)&&(opSelectDest == 6)){
                        opSelectSource = 5;
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                    else if (opSelectSource > 0){
                        opSelectSource -= 1;
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                }
                else if (wParam == VK_RIGHT){
                    if ((opSelectSource == 5)&&(opSelectDest == 6)){
                        opSelectSource = 8;
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                    else if (opSelectSource < 11){
                        opSelectSource += 1;
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                }
                else if (wParam == VK_UP){
                    if (opSelectDest > 0){
                        opSelectDest -= 1;
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                }
                else if (wParam == VK_DOWN){
                    if (opSelectDest < 6){
                        if (!((opSelectDest == 5)&&(opSelectSource > 5 )&&(opSelectSource < 8))){
                            opSelectDest += 1;
                            InvalidateRect( hwnd, NULL, TRUE );
                        }
                    }
                }
            }
            else{
                if ((wParam+1 > '0')&&(wParam - 1 < '9')){               //use the fact that numbers 0-9 in ascii comes in the order 0,1,...,9
                    if (opSelectSource < 6){  //Matrix cases and Outputs cases
                        if (activeNumberPos == 0){
                            if (opSelectDest < 6){
                                opMatrix[6*opSelectDest + opSelectSource] = 0;
                            }
                            else{
                                opOut[opSelectSource] = 0;
                            }
                        }
                        if (opSelectDest < 6){
                            opMatrix[6*opSelectDest + opSelectSource] = 10*opMatrix[6*opSelectDest + opSelectSource];
                            opMatrix[6*opSelectDest + opSelectSource] += (int)wParam - '0';
                        }
                        else{
                            opOut[opSelectSource] = 10*opOut[opSelectSource];
                            opOut[opSelectSource] += (int)wParam - '0';
                        }

                        activeNumberPos += 1;
                        if (activeNumberPos > 3){
                            activeNumberPos = 0;
                            activeElement = 0;
                        }
                    }
                    else if ((opSelectDest < 6)&&(opSelectSource == 6)){  //Ratio cases
                        opRatio[opSelectDest] = opRatio[opSelectDest] - ((opRatio[opSelectDest]/((int)pow(10,4-activeRatioNumberPos)))%10)*pow(10,4-activeRatioNumberPos);
                        opRatio[opSelectDest] += ((int)wParam - '0')*pow(10,4-activeRatioNumberPos);
                        activeRatioNumberPos += 1;
                        if (activeRatioNumberPos > 4){
                            activeRatioNumberPos = 0;
                            activeElement = 0;
                        }
                    }
                    else if((opSelectDest < 7)&&(opSelectSource > 7)&&(opSelectSource < 12)){ //ADSR-env case
                        if (opSelectSource == 8){
                            if (activeADSRNumberPos == 0){
                                paramAttack[opSelectDest] = 0;
                            }
                            paramAttack[opSelectDest] = 10*paramAttack[opSelectDest];
                            paramAttack[opSelectDest] += (int)wParam - '0';
                        }
                        else if (opSelectSource == 9){
                            if (activeADSRNumberPos == 0){
                                paramDecay[opSelectDest] = 0;
                            }
                            paramDecay[opSelectDest] = 10*paramDecay[opSelectDest];
                            paramDecay[opSelectDest] += (int)wParam - '0';
                        }
                        else if (opSelectSource == 10){
                            if (activeADSRNumberPos == 0){
                                paramSustain[opSelectDest] = 0;
                            }
                            paramSustain[opSelectDest] = 10*paramSustain[opSelectDest];
                            paramSustain[opSelectDest] += (int)wParam - '0';
                            if (paramSustain[opSelectDest] > 100){
                                paramSustain[opSelectDest] = 100;
                            }
                        }
                        else if (opSelectSource == 11){
                            if (activeADSRNumberPos == 0){
                                paramRelease[opSelectDest] = 0;
                            }
                            paramRelease[opSelectDest] = 10*paramRelease[opSelectDest];
                            paramRelease[opSelectDest] += (int)wParam - '0';
                        }

                        activeADSRNumberPos += 1;
                        if (activeADSRNumberPos > 2){
                            activeADSRNumberPos = 0;
                            activeElement = 0;
                        }
                    }
                    InvalidateRect( hwnd, NULL, TRUE );
                }
                else if((opSelectSource == 7)&&(opSelectDest < 6)){
                    if (wParam == VK_RIGHT){
                        opWaveForm[opSelectDest] += 1;
                        if (opWaveForm[opSelectDest] > 4){
                            opWaveForm[opSelectDest] = 1;
                        }
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                    else if (wParam == VK_LEFT){
                        opWaveForm[opSelectDest] -= 1;
                        if (opWaveForm[opSelectDest] < 1){
                            opWaveForm[opSelectDest] = 4;
                        }
                        InvalidateRect( hwnd, NULL, TRUE );
                    }
                }
            }
            if (wParam == VK_RETURN){
                activeElement = (activeElement+1)%2;
                activeNumberPos = 0;
                activeRatioNumberPos = 0;
                activeADSRNumberPos = 0;
                if((opSelectSource == 8)&&(opSelectDest<6)){
                    envAactive[opSelectDest] = 1;
                }
                else if((opSelectSource == 11)&&(opSelectDest<6)){
                    envRactive[opSelectDest] = 1;
                }
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if (wParam == VK_DELETE){
                if ((opSelectDest < 6)&&(opSelectSource < 6)){
                    opMatrix[6*opSelectDest + opSelectSource] = 0;
                }
                else if (opSelectSource < 6){
                    opOut[opSelectSource] = 0;
                }

                if((opSelectSource == 8)&&(opSelectDest<6)){
                    envAactive[opSelectDest] = 0;
                }
                else if((opSelectSource == 11)&&(opSelectDest<6)){
                    envRactive[opSelectDest] = 0;
                }
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(wParam == VK_F1){
                loadPreset(1);
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(wParam == VK_F2){
                loadPreset(2);
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(wParam == VK_F3){
                loadPreset(3);
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(wParam == VK_F4){
                loadPreset(4);
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(wParam == VK_F5){
                loadPreset(5);
                InvalidateRect( hwnd, NULL, TRUE );
            }
            
            break;
        case WM_LBUTTONUP:
            if(mouseInRect(LOWORD(lParam),HIWORD(lParam),90,70,90+51+50*5,70+51+50*6)){
                if ((opSelectSource == (LOWORD(lParam)-90)/50)&&(opSelectDest == (HIWORD(lParam)-70)/50)){
                    activeElement = (activeElement+1)%2;
                }
                else{
                    opSelectSource = (LOWORD(lParam)-90)/50;
                    opSelectDest = (HIWORD(lParam)-70)/50;
                    activeElement = 0;
                }
                activeNumberPos = 0;
                activeRatioNumberPos = 0;
                activeADSRNumberPos = 0;
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),90+50*6+50,70,90+51+50*6+100,70+51+50*5)){
                if ((opSelectSource == 6)&&(opSelectDest == (HIWORD(lParam)-70)/50)){
                    activeElement = (activeElement+1)%2;
                }
                else{
                    opSelectSource = 6;
                    opSelectDest = (HIWORD(lParam)-70)/50;
                    activeElement = 0;
                }
                activeNumberPos = 0;
                activeRatioNumberPos = 0;
                activeADSRNumberPos = 0;
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam),90+50*8+50,70,90+51+50*8+100,70+51+50*5)){
                if ((opSelectSource == 7)&&(opSelectDest == (HIWORD(lParam)-70)/50)){
                    activeElement = (activeElement+1)%2;
                }
                else{
                    opSelectSource = 7;
                    opSelectDest = (HIWORD(lParam)-70)/50;
                    activeElement = 0;
                }
                activeNumberPos = 0;
                activeRatioNumberPos = 0;
                activeADSRNumberPos = 0;
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else if(mouseInRect(LOWORD(lParam),HIWORD(lParam), 90+50*10+50, 70, 90+50+50*12+100, 70+51+50*6)){
                if ((opSelectSource == 8 + (LOWORD(lParam)-(90+50*10+50))/50)&&(opSelectDest == (HIWORD(lParam)-70)/50)){
                    activeElement = (activeElement+1)%2;
                    if((opSelectSource == 8)&&(opSelectDest<6)){
                        envAactive[opSelectDest] = 1;
                    }
                    else if((opSelectSource == 11)&&(opSelectDest<6)){
                        envRactive[opSelectDest] = 1;
                    }
                }
                else{
                    opSelectSource = 8 + (LOWORD(lParam)-(90+50*10+50))/50;
                    opSelectDest = (HIWORD(lParam)-70)/50;
                    activeElement = 0;
                }
                activeNumberPos = 0;
                activeRatioNumberPos = 0;
                activeADSRNumberPos = 0;
                InvalidateRect( hwnd, NULL, TRUE );
            }
            else{
                 activeElement = 0;
                 InvalidateRect( hwnd, NULL, TRUE );
            }
            
            break;
        case WM_PAINT:
            theGraphicsSF(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


//EXTERNAL Sound Factory functions-----------------------------------------------------------------

double soundFactoryProduce(struct soundFactory *x){
    double output;
    int i;
    struct soundNode *lp;
    double opA;
    double opB;
    double opC;
    double opD;
    double opE;
    double opF;
    double opApost;
    double opBpost;
    double opCpost;
    double opDpost;
    double opEpost;
    double opFpost;
    double opAPhase;
    double opBPhase;
    double opCPhase;
    double opDPhase;
    double opEPhase;
    double opFPhase;

    double attack[7];
    double decay[7];
    double release[7]; 
 
    double frequency;
    double phaseAngle;

    output = 0.0;

    lp=x->soundlistHead;

    while(lp->next->next != lp->next){ 
        opA = lp->next->opA;
        opB = lp->next->opB;
        opC = lp->next->opC;
        opD = lp->next->opD;
        opE = lp->next->opE;
        opF = lp->next->opF;
        opAPhase = lp->next->opAPhase;
        opBPhase = lp->next->opBPhase;
        opCPhase = lp->next->opCPhase;
        opDPhase = lp->next->opDPhase;
        opEPhase = lp->next->opEPhase;
        opFPhase = lp->next->opFPhase;

        for(i=0;i<7;i++){
            attack[i] = lp->next->attack[i];
            decay[i] = lp->next->decay[i];
            release[i] = lp->next->release[i];
        }

        frequency = lp->next->frequency;
        phaseAngle = lp->next->phaseAngle;

        opApost = envelopeADSR(envAactive[0], envRactive[0], attack[0], decay[0], release[0])*waveFunction(opWaveForm[0], opAPhase + 0.01*opMatrix[6*0 + 0]*opA + 0.01*opMatrix[6*0 + 1]*opB + 0.01*opMatrix[6*0 + 2]*opC + 0.01*opMatrix[6*0 + 3]*opD + 0.01*opMatrix[6*0 + 4]*opE + 0.01*opMatrix[6*0 + 5]*opF);
        opBpost = envelopeADSR(envAactive[1], envRactive[1], attack[1], decay[1], release[1])*waveFunction(opWaveForm[1], opBPhase + 0.01*opMatrix[6*1 + 0]*opA + 0.01*opMatrix[6*1 + 1]*opB + 0.01*opMatrix[6*1 + 2]*opC + 0.01*opMatrix[6*1 + 3]*opD + 0.01*opMatrix[6*1 + 4]*opE + 0.01*opMatrix[6*1 + 5]*opF);
        opCpost = envelopeADSR(envAactive[2], envRactive[2], attack[2], decay[2], release[2])*waveFunction(opWaveForm[2], opCPhase + 0.01*opMatrix[6*2 + 0]*opA + 0.01*opMatrix[6*2 + 1]*opB + 0.01*opMatrix[6*2 + 2]*opC + 0.01*opMatrix[6*2 + 3]*opD + 0.01*opMatrix[6*2 + 4]*opE + 0.01*opMatrix[6*2 + 5]*opF);
        opDpost = envelopeADSR(envAactive[3], envRactive[3], attack[3], decay[3], release[3])*waveFunction(opWaveForm[3], opDPhase + 0.01*opMatrix[6*3 + 0]*opA + 0.01*opMatrix[6*3 + 1]*opB + 0.01*opMatrix[6*3 + 2]*opC + 0.01*opMatrix[6*3 + 3]*opD + 0.01*opMatrix[6*3 + 4]*opE + 0.01*opMatrix[6*3 + 5]*opF);
        opEpost = envelopeADSR(envAactive[4], envRactive[4], attack[4], decay[4], release[4])*waveFunction(opWaveForm[4], opEPhase + 0.01*opMatrix[6*4 + 0]*opA + 0.01*opMatrix[6*4 + 1]*opB + 0.01*opMatrix[6*4 + 2]*opC + 0.01*opMatrix[6*4 + 3]*opD + 0.01*opMatrix[6*4 + 4]*opE + 0.01*opMatrix[6*4 + 5]*opF);
        opFpost = envelopeADSR(envAactive[5], envRactive[5], attack[5], decay[5], release[5])*waveFunction(opWaveForm[5], opFPhase + 0.01*opMatrix[6*5 + 0]*opA + 0.01*opMatrix[6*5 + 1]*opB + 0.01*opMatrix[6*5 + 2]*opC + 0.01*opMatrix[6*5 + 3]*opD + 0.01*opMatrix[6*5 + 4]*opE + 0.01*opMatrix[6*5 + 5]*opF);


        //output += (exp(decay)-1)/(exp(1)-1)*(exp(release)-1)/(exp(1)-1)*(exp(attack)-1)/(exp(1)-1)*2000*(0.01*opOut[0]*(opApost) + 0.01*opOut[1]*(opBpost) + 0.01*opOut[2]*(opCpost) + 0.01*opOut[3]*(opDpost) + 0.01*opOut[4]*(opEpost) + 0.01*opOut[5]*(opFpost)); 
        output += envelopeADSR(1,1, attack[6], decay[6], release[6])*2000*(0.01*opOut[0]*(opApost) + 0.01*opOut[1]*(opBpost) + 0.01*opOut[2]*(opCpost) + 0.01*opOut[3]*(opDpost) + 0.01*opOut[4]*(opEpost) + 0.01*opOut[5]*(opFpost)); 
        

        phaseAngle += 2*PI*(frequency)/(44100);
        if (phaseAngle > (2*PI)){
            phaseAngle -= 2*PI;
        }

        opAPhase += 2*PI*0.0001*opRatio[0]*(frequency)/(44100);
        if (opAPhase > (2*PI)){
            opAPhase -= 2*PI;
        }

        opBPhase += 2*PI*0.0001*opRatio[1]*(frequency)/(44100);
        if (opBPhase > (2*PI)){
            opBPhase -= 2*PI;
        }

        opCPhase += 2*PI*0.0001*opRatio[2]*(frequency)/(44100);
        if (opCPhase > (2*PI)){
            opCPhase -= 2*PI;
        }

        opDPhase += 2*PI*0.0001*opRatio[3]*(frequency)/(44100);
        if (opDPhase > (2*PI)){
            opDPhase -= 2*PI;
        }

        opEPhase += 2*PI*0.0001*opRatio[4]*(frequency)/(44100);
        if (opEPhase > (2*PI)){
            opEPhase -= 2*PI;
        }

        opFPhase += 2*PI*0.0001*opRatio[5]*(frequency)/(44100);
        if (opFPhase > (2*PI)){
            opFPhase -= 2*PI;
        }


        for (i = 0; i < 7; i++){
            if (attack[i]<1){
                attack[i] += 1/pow(10,0.01*paramAttack[i]);
                if (attack[i]>1){
                    attack[i] = 1.0;
                }
            }
            else if ((decay[i]>0.01*paramSustain[i])&&(lp->next->releasedTrue == 0)){
                decay[i] -= 1/pow(10,0.01*paramDecay[i]);
                if (decay[i] < 0.01*paramSustain[i]){
                    decay[i] = 0.01*paramSustain[i];
                }
            }

            if (lp->next->releasedTrue > 0){
                release[i] -= 1.0/pow(10,0.01*paramRelease[i]);
                if (release[i] < 0){
                   if (i < 6){
                       release[i] = 0.0;
                   }
                   else{
                       release[i] = -50.0;
                   }
                }
            }
        }


        lp->next->opA = opApost;
        lp->next->opB = opBpost;
        lp->next->opC = opCpost;
        lp->next->opD = opDpost;
        lp->next->opE = opEpost;
        lp->next->opF = opFpost;

        lp->next->opAPhase = opAPhase;
        lp->next->opBPhase = opBPhase;
        lp->next->opCPhase = opCPhase;
        lp->next->opDPhase = opDPhase;
        lp->next->opEPhase = opEPhase;
        lp->next->opFPhase = opFPhase;

        lp->next->phaseAngle = phaseAngle;

        for (i = 0; i < 7; i++){
            lp->next->attack[i] = attack[i];
            lp->next->decay[i] = decay[i];
            lp->next->release[i] = release[i];
        }

        if (release[6] < 0.0){
            soundlistDeleteNext(lp);
        }
        else{
            lp = lp->next;
        }
    }

   return output;
}


void soundFactoryModify(int tone, int mode, struct soundFactory *x){
    struct soundNode *sLoopPointer;

    
    if (mode == SF_TURN_ON){         
        soundlistInsertAfter(getFrequency(tone), 0.0, tone, x->soundlistHead);
    } 
    else if (mode == SF_TURN_OFF){    
        sLoopPointer = x->soundlistHead;
        while(sLoopPointer->next->next != sLoopPointer->next){
            if (sLoopPointer->next->id == tone){
                  sLoopPointer->next->releasedTrue = 1;
                  break;
            }
            sLoopPointer = sLoopPointer->next;
        } 
    } 
    else if (mode == SF_EMPTY){      // <-----------------------------------------All tones stops playing
        sLoopPointer = x->soundlistHead;
        while(sLoopPointer->next->next != sLoopPointer->next){
            soundlistDeleteNext(sLoopPointer);
        }
    }
}

void soundFactoryInitialize(struct soundFactory *t){
            struct soundNode *tHead;
            struct soundNode *tZero;
            tHead = (struct soundNode *)malloc(sizeof *tHead);
            tZero = (struct soundNode *)malloc(sizeof *tZero);

            tHead->next = tZero;           
            tZero->next = tZero;

            t->soundlistHead = tHead;
            t->soundlistZero = tZero;
}



//INTERNAL functions--------------------------------------------------------------------

double waveFunction(int type, double input){
    double retvalue = 0.0;
    if (type == 1){
        retvalue = sin(input);
    }
    else if (type == 2){
        retvalue = sawtooth(input);
    }
    else if (type == 3){
        retvalue = squarewave(input);
    }
    else if (type == 4){
        retvalue = trianglewave(input);
    }

    return retvalue;
}

double sawtooth(double phaseAngle){
    double retvalue = -1.0;
    retvalue = phaseAngle/PI-1.0;

    return retvalue;
}

double squarewave(double phaseAngle){
    double retvalue = -1.0;
    if (phaseAngle>PI){
        retvalue = 1.0;
    }

    return retvalue;
}

double trianglewave(double phaseAngle){
    double retvalue = 0.0;
    if (phaseAngle < PI/2){
        retvalue = 2*phaseAngle/PI;
    }
    else if(phaseAngle < 3*PI/2){
        retvalue = -2*phaseAngle/PI+2.0;
    }
    else{
        retvalue = 2*phaseAngle/PI-4.0;
    }
    return retvalue*1.2;
}

double envelopeADSR(int activeA, int activeR, double a, double d, double r){
    double retvalue;

    //retvalue = (exp(d)-1)/(exp(1)-1)*(exp(r)-1)/(exp(1)-1)*(exp(a)-1)/(exp(1)-1);

    retvalue = (exp(d)-1)/(exp(1)-1);
    if (activeA){
        retvalue = retvalue*(exp(a)-1)/(exp(1)-1);
    }
    if (activeR){
        retvalue = retvalue*(exp(r)-1)/(exp(1)-1);
    }

    return retvalue;

}

double getFrequency(int tone){
  float temp = 1.0;
  int tonfactor = 0;
  int k;
  double res = 0.0;

  tonfactor = tone/12;

  k = tone%12;

  if (k==0)
    temp = 1.0;
  else if (k==1)
    temp = 1.0595;
  else if (k==2)
    temp = 1.1225;
  else if (k==3)
    temp = 1.1892;
  else if (k==4)
    temp = 1.2599;
  else if (k==5)
    temp = 1.3348;
  else if (k==6)
    temp = 1.4142;
  else if (k==7)
    temp = 1.4983;
  else if (k==8)
    temp = 1.5874;
  else if (k==9)
    temp = 1.6818;
  else if (k==10)
    temp = 1.7818;
  else if (k==11)
    temp = 1.8877;
  else if (k==12)
    temp = 2.0;

  res = 55*temp*pow(2.0,tonfactor)*0.25;
  return res;
}

int convNum(int number, WCHAR buf[]){
    buf[0] = '0' + number/10000;
    buf[1] = '.';
    buf[2] = '0' + (number - (number/10000)*10000)/1000;
    buf[3] = '0' + (number - (number/1000)*1000)/100;
    buf[4] = '0' + (number - (number/100)*100)/10;
    buf[5] = '0' + (number - (number/10)*10);
    return(0);
}

void theGraphicsSF(HWND hwnd){

    PAINTSTRUCT ps;
    double y = 0;
    RECT r;


    WCHAR charBuffer[10];

    int i;
    int j;
    int k;

    int yUpper;
    int yLower;

    HDC hdc = BeginPaint(hwnd, &ps);

    HPEN hPenBlack = CreatePen(PS_SOLID, 1, RGB(0,0,0));
    HPEN hPenRed = CreatePen(PS_SOLID, 1, RGB(255,0,0));
    HPEN hPenLightBlue = CreatePen(PS_SOLID, 1, RGB(150, 150, 255));
    HPEN hPenInvisible = CreatePen(PS_NULL, 1, RGB(0,0,0));


    HBRUSH hBrushLightBlue = CreateSolidBrush(RGB(150, 150, 255));
    HBRUSH hBrushLighterBlue = CreateSolidBrush(RGB(200, 200, 255));
    HBRUSH hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH hBrushGreen = CreateSolidBrush(RGB(150, 200, 150));


    HFONT hfont3 = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas");
    HFONT hfont2 = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Gabriola");

    SelectObject(hdc, hPenInvisible);

    for(i=0;i<10;i++){
        charBuffer[i] = 0;
    }

    //Start painting--------------------------------

   // MoveToEx(hdc, 30, 100, NULL);
   // LineTo(hdc, 30 , 410);

    SelectObject(hdc, hBrushWhite);
    Rectangle(hdc, 0, 0, WINDOWWIDTHSF, WINDOWHEIGHTSF);

    //Draw checkered matrix and fill in operator matrix
    SetBkMode(hdc,TRANSPARENT);
    SetTextColor(hdc,RGB(50,50,200));
    SelectObject(hdc, hfont3);
    for(i = 0; i < 6; i++){
        for(j = 0; j< 7; j++){
            if ((activeElement)&&(i == opSelectSource)&&(j == opSelectDest)){
                SelectObject(hdc, hBrushWhite);
            }
            else if (i==j){
                SelectObject(hdc, hBrushGreen);
            }
            else if (j>5){
                SelectObject(hdc, hBrushLighterBlue);
            }
            else if ((i+j)%2){
                SelectObject(hdc, hBrushLighterBlue);
            }
            else{
                SelectObject(hdc, hBrushLightBlue);
            }
            Rectangle(hdc, 90+50*i, 70+50*j, 90+51+50*i, 70+51+50*j);

            r.top = 70+15+50*j;
            r.left = 90+5+50*i;
            r.bottom = 70+51+50*j;
            r.right = 90+51+50*i;
            if (j < 6){
                if (opMatrix[6*j + i] > 0){
                    for(k=0;k<10;k++){
                        charBuffer[k] = 0;
                    }
                    _itow(opMatrix[6*j + i], charBuffer,10);
                    DrawText(hdc,charBuffer, 10, &r, DT_LEFT);
                }
            }
            else{
                if (opOut[i] > 0){
                    for(k=0;k<10;k++){
                        charBuffer[k] = 0;
                    }
                _itow(opOut[i], charBuffer,10);
                DrawText(hdc,charBuffer, 10, &r, DT_LEFT);
                }
            }
        }
    }

    //Ratios
    for (j=0; j<6;j++){
        if ((activeElement)&&(opSelectSource==6)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*6+50, 70+50*j, 90+51+50*6+100, 70+51+50*j);
    }
    //Ratio numbers
    for(j = 0; j< 6; j++){
        r.top = 70+15+50*j;
        r.left = 90+5+50*7;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*8;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        convNum(opRatio[j], charBuffer);
        DrawText(hdc,charBuffer, 10, &r, DT_LEFT);
    }

    //Wave forms
    for (j=0; j<6;j++){
        if ((activeElement)&&(opSelectSource == 7)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*8+50, 70+50*j, 90+51+50*8+100, 70+51+50*j);
    }
    //Waveforms text
    for(j = 0; j< 6; j++){
        r.top = 70+15+50*j;
        r.left = 90+5+50*9;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*10;
        if (opWaveForm[j] == 1){
            DrawText(hdc,L"Sine", 4, &r, DT_LEFT);
        }
        else if (opWaveForm[j] == 2){
            DrawText(hdc,L"Sawtooth", 8, &r, DT_LEFT);
        }
        else if (opWaveForm[j] == 3){
            DrawText(hdc,L"Square", 6, &r, DT_LEFT);
        }
        else if (opWaveForm[j] == 4){
            DrawText(hdc,L"Triangle", 8, &r, DT_LEFT);
        }
    }

    //ADSR envelopes
    //A
    for (j=0;j<7;j++){
        if ((activeElement)&&(opSelectSource == 8)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*11, 70+50*j, 90+51+50*11, 70+51+50*j);
        r.top = 70+15+50*j;
        r.left = 90+5+50*11;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*12;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        _itow(paramAttack[j], charBuffer,10);
        if (j<6){
            if(envAactive[j]){
                DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
            }
        }
        else{
            DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
        }
    }
    //D
    for (j=0;j<7;j++){
        if ((activeElement)&&(opSelectSource == 9)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*12, 70+50*j, 90+51+50*12, 70+51+50*j);
        r.top = 70+15+50*j;
        r.left = 90+5+50*12;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*13;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        _itow(paramDecay[j], charBuffer,10);
        DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
    }
    //S
    for (j=0;j<7;j++){
        if ((activeElement)&&(opSelectSource == 10)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*13, 70+50*j, 90+51+50*13, 70+51+50*j);
        r.top = 70+15+50*j;
        r.left = 90+5+50*13;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*14;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        _itow(paramSustain[j], charBuffer,10);
        DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
    }
    //R
    for (j=0;j<7;j++){
        if ((activeElement)&&(opSelectSource == 11)&&(j == opSelectDest)){
            SelectObject(hdc, hBrushWhite);
        }
        else{
            SelectObject(hdc, hBrushLighterBlue);
        }
        Rectangle(hdc, 90+50*14, 70+50*j, 90+51+50*14, 70+51+50*j);
        r.top = 70+15+50*j;
        r.left = 90+5+50*14;
        r.bottom = 70+51+50*j;
        r.right = 90+51+50*15;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        _itow(paramRelease[j], charBuffer,10);
        if (j<6){
            if(envRactive[j]){
                DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
            }
        }
        else{
                DrawText(hdc,charBuffer, 10, &r, DT_CENTER);
        }

    }


    //line between ratios, wave forms and ADSR
    SelectObject(hdc, hPenLightBlue);
    MoveToEx(hdc, 90+50*8+50, 70+50*0, NULL);
    LineTo(hdc, 90+50*8+50 , 70+50*6);

    MoveToEx(hdc, 90+50*10+50, 70+50*0, NULL);
    LineTo(hdc, 90+50*10+50 , 70+50*6);

    MoveToEx(hdc, 90+50*11+50, 70+50*0, NULL);
    LineTo(hdc, 90+50*11+50 , 70+50*7);

    MoveToEx(hdc, 90+50*12+50, 70+50*0, NULL);
    LineTo(hdc, 90+50*12+50 , 70+50*7);

    MoveToEx(hdc, 90+50*13+50, 70+50*0, NULL);
    LineTo(hdc, 90+50*13+50 , 70+50*7);

    MoveToEx(hdc, 90+50*6+50, 70+50*6, NULL);
    LineTo(hdc, 90+50+50*14 , 70+50*6);
    
    //Draw ABCDEF on colums and rows
    for (i = 0; i < 6; i++){
        r.top = 70-20;
        r.left = 90+20+50*i;
        r.bottom = 70+51-20;
        r.right = 90+51+50*i;
        for(k=0;k<10;k++){
            charBuffer[k] = 0;
        }
        charBuffer[0] = 65 + i;       //65 is ASCII for A, 66 for B etc...
        DrawText(hdc,charBuffer, 10, &r, DT_LEFT);   //columns
        r.top = 70+15+50*i;
        r.left = 90-20;
        r.bottom = 70+51+50*i;
        r.right = 90;
        DrawText(hdc,charBuffer, 10, &r, DT_LEFT);   //rows
    }

    r.top = 70+15+50*6;
    r.left = 90-35;
    r.bottom = 70+51+50*6;
    r.right = 90;
    DrawText(hdc,L"Out", 3, &r, DT_LEFT);

    SelectObject(hdc, hfont2);
    SetTextColor(hdc,RGB(0,0,0));
    r.top = 70-45;
    r.left = 90+20+100;
    r.bottom = 70+51-45;
    r.right = 90+51+130;
    DrawText(hdc,L"Source", 6, &r, DT_LEFT);

    r.top = 70+5+125;
    r.left = 90-70;
    r.bottom = 70+51+125;
    r.right = 90;
    DrawText(hdc,L"Dest.", 5, &r, DT_LEFT);

    r.top = 70-45+10;
    r.left = 90+50*6+75;
    r.bottom = 70+51-45+10;
    r.right = 90+50+50*6+100;
    DrawText(hdc,L"Ratio", 5, &r, DT_LEFT);

    r.top = 70-45+10;
    r.left = 90+50*8+60;
    r.bottom = 70+51-45+10;
    r.right = 90+50+50*8+100;
    DrawText(hdc,L"Waveform", 8, &r, DT_LEFT);

    r.top = 70-35;
    r.left = 90+20+50*11;
    r.bottom = 70+51-35;
    r.right = 90+51+50*12;
    DrawText(hdc,L"A", 1, &r, DT_LEFT);

    r.top = 70-35;
    r.left = 90+20+50*12;
    r.bottom = 70+51-35;
    r.right = 90+51+50*13;
    DrawText(hdc,L"D", 1, &r, DT_LEFT);

    r.top = 70-35;
    r.left = 90+20+50*13;
    r.bottom = 70+51-35;
    r.right = 90+51+50*14;
    DrawText(hdc,L"S", 1, &r, DT_LEFT);

    r.top = 70-35;
    r.left = 90+20+50*14;
    r.bottom = 70+51-35;
    r.right = 90+51+50*15;
    DrawText(hdc,L"R", 1, &r, DT_LEFT);



    //line between operators and output
    SelectObject(hdc, hPenLightBlue);
    MoveToEx(hdc, 90+50*0, 70+50*6, NULL);
    LineTo(hdc, 90+50+50*5 , 70+50*6);


    i = opSelectSource;
    j = opSelectDest;

    //Mark the selected matrix element with a red square
    SelectObject(hdc, hPenRed);
    if (i < 6){
        MoveToEx(hdc, 90+50*i, 70+50*j, NULL);
        LineTo(hdc, 90+50+50*i , 70+50*j);
        LineTo(hdc, 90+50+50*i , 70+50+50*j);
        LineTo(hdc, 90+50*i , 70+50+50*j);
        LineTo(hdc, 90+50*i , 70+50*j);
    }
    else if(i == 6){
        MoveToEx(hdc, 90+50*i+50, 70+50*j, NULL);
        LineTo(hdc, 90+50+50*i+100 , 70+50*j);
        LineTo(hdc, 90+50+50*i+100 , 70+50+50*j);
        LineTo(hdc, 90+50*i +50, 70+50+50*j);
        LineTo(hdc, 90+50*i +50, 70+50*j);
    }
    else if(i == 7){
        MoveToEx(hdc, 90+50*i+100, 70+50*j, NULL);
        LineTo(hdc, 90+50+50*i+150 , 70+50*j);
        LineTo(hdc, 90+50+50*i+150 , 70+50+50*j);
        LineTo(hdc, 90+50*i +100, 70+50+50*j);
        LineTo(hdc, 90+50*i +100, 70+50*j);
    }
    else if(i > 7){
        MoveToEx(hdc, 90+50*i+150, 70+50*j, NULL);
        LineTo(hdc, 90+50+50*i+150 , 70+50*j);
        LineTo(hdc, 90+50+50*i+150 , 70+50+50*j);
        LineTo(hdc, 90+50*i +150, 70+50+50*j);
        LineTo(hdc, 90+50*i +150, 70+50*j);
    }



    //Cleaning --------------------------------------

    DeleteObject(hPenBlack);
    DeleteObject(hPenRed);
    DeleteObject(hPenInvisible);
    DeleteObject(hPenLightBlue);

    DeleteObject(hBrushLightBlue);
    DeleteObject(hBrushLighterBlue);
    DeleteObject(hBrushWhite);
    DeleteObject(hBrushGreen);

    DeleteObject(hfont2);
    DeleteObject(hfont3);

 
    EndPaint(hwnd, &ps);
}


int mouseInRectSF(int mouseX, int mouseY, int rectX1, int rectY1, int rectX2, int rectY2){
    if ((mouseX > rectX1)&&(mouseX < rectX2)&&(mouseY < rectY2)&&(mouseY > rectY1)){
        return 1;
    }
    else{
        return 0;
    }
}

void loadPreset(int id){
    int opMatrix2[36] = {0,  0,  0,  0,  0,  0,
                         0, 60, 10, 10, 10, 10,
                         0, 10, 60, 10, 10, 10,
                         0, 10, 10, 60, 10, 10,
                         0, 10, 10, 10, 60, 10,
                         0, 10, 10, 10, 10, 60};
    int opOut2[6] = {0, 40, 40, 40, 40, 50};
    int opRatio2[6] = {10000,19940,20060,9970,10030,10000};
    int opWaveForm2[6] = {1, 3, 4, 2, 3, 1};

    int opMatrix3[36] = {0,  0,  0,  0,  0,  0,
                         0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0,
                         0, 0, 50, 120, 0, 0,
                         0, 0, 0, 40, 50, 0,
                         0, 0, 0, 0, 40, 50};
    int opOut3[6] = {0, 0, 0, 70, 70, 70};
    int opRatio3[6] = {10000,10000,10000,10000,9970,10030};
    int opWaveForm3[6] = {1, 1, 1, 1, 1, 1};

    int opMatrix4[36] = {70,  0, 0,  0,  0,  0,
                         70, 80, 0,  0,  0,  0,
                          0, 80, 0,  0,  0,  0,
                          0, 70, 0, 80,  0,  0,
                          0,  0, 0, 90, 50,  0,
                          0,  0, 0,  0, 90, 50};
    int opOut4[6] = {0, 0, 150, 40, 40, 40};
    int opRatio4[6] = {5000,5000,20000,10000,9940,10060};
    int opWaveForm4[6] = {4, 4, 4, 3, 3, 3};

    int opMatrix5[36] = {90, 0, 0, 0, 0, 0,
                         0, 90, 0, 0, 0, 0,
                         0, 0, 90, 0, 0, 0,
                         0, 0, 0, 90, 0, 0,
                         0, 0, 0, 0, 90, 0,
                         0, 0, 0, 0, 0, 90};
    int opOut5[6] = {40, 40, 30, 40, 50, 60};
    int opRatio5[6] = {20050,19950,80000,40000,20000,10000};
    int opWaveForm5[6] = {1, 1, 1, 1, 1, 1};


    int i;
    if (id==1){
        for(i = 0; i< 36; i++){
            opMatrix[i] = 0;
        }
        for(i=0;i<6;i++){
            opOut[i] = 0;
            opRatio[i] = 10000;
            opWaveForm[i] = 1;
        }
        opOut[5] = 100;
    }
    else if (id==2){
        for(i = 0; i< 36; i++){
            opMatrix[i] = opMatrix2[i];
        }
        for(i=0;i<6;i++){
            opOut[i] = opOut2[i];
            opRatio[i] = opRatio2[i];
            opWaveForm[i] = opWaveForm2[i];
        }
    }
    else if (id==3){
        for(i = 0; i< 36; i++){
            opMatrix[i] = opMatrix3[i];
        }
        for(i=0;i<6;i++){
            opOut[i] = opOut3[i];
            opRatio[i] = opRatio3[i];
            opWaveForm[i] = opWaveForm3[i];
        }
    }
    else if (id==4){
        for(i = 0; i< 36; i++){
            opMatrix[i] = opMatrix4[i];
        }
        for(i=0;i<6;i++){
            opOut[i] = opOut4[i];
            opRatio[i] = opRatio4[i];
            opWaveForm[i] = opWaveForm4[i];
        }
    }
    else if (id==5){
        for(i = 0; i< 36; i++){
            opMatrix[i] = opMatrix5[i];
        }
        for(i=0;i<6;i++){
            opOut[i] = opOut5[i];
            opRatio[i] = opRatio5[i];
            opWaveForm[i] = opWaveForm5[i];
        }
    }
}



//linked-list functions------------------------------------------------------
void soundlistInsertAfter(double v1, double v2a, int v3, struct soundNode *t){
    struct soundNode *x;
    int i;
    x = (struct soundNode *)malloc(sizeof *x);
    x->frequency = v1;
    x->phaseAngle = v2a;
    x->id = v3;

    x->opA = 0.0;
    x->opB = 0.0;
    x->opC = 0.0;
    x->opD = 0.0;
    x->opE = 0.0;
    x->opF = 0.0;

    x->opAPhase = initPhaseA;
    x->opBPhase = initPhaseB;
    x->opCPhase = initPhaseC;
    x->opDPhase = initPhaseD;
    x->opEPhase = initPhaseE;
    x->opFPhase = initPhaseF;

    for(i=0; i<7; i++){
        x->attack[i] = 0.0;
        x->decay[i] = 1.0;
        x->release[i] = 1.0;
    }

    x->releasedTrue = 0;
    
    x->next = t->next;

    t->next = x;
}

void soundlistDeleteNext(struct soundNode *t){
    struct soundNode *x;
    x = t->next;
    t->next = t->next->next;
    free(x);
}