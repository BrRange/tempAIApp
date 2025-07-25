#define ACTIVATIONFNIMPL
#define LAYERMODELIMPL
#include <stdio.h>
#include <stdlib.h>
#include "winframeGUI.h"
#include "layerModel.h"

void printMat(Mat m){
  putchar('\n');
  for(unsigned i = 0; i < m.r; i++){
    putchar('|');
    for(unsigned j = 0; j < m.c; j++){
      printf("%1.2f\t", readMat(m, i, j));
    }
    puts("|");
  }
}

LayerModel loadLayerModel(FILE *file){
  LayerModel lm;
  fread(&lm.layerSize, sizeof(size_t), 1, file);
  fread(&lm.loss, sizeof(lm.loss), 1, file);
  lm.layer = malloc(sizeof(Layer) * lm.layerSize);
  for(size_t i = 0; i < lm.layerSize; i++){
    fread(&lm.layer[i].act, sizeof(enum LayerFunc), 1, file);
    fread(&lm.layer[i].weight.r, sizeof(unsigned), 1, file);
    fread(&lm.layer[i].weight.c, sizeof(unsigned), 1, file);
    lm.layer[i].weight.data = malloc(lm.layer[i].weight.r * lm.layer[i].weight.c * sizeof(float));
    fread(lm.layer[i].weight.data, sizeof(float), lm.layer[i].weight.r * lm.layer[i].weight.c, file);
    fread(&lm.layer[i].bias.r, sizeof(unsigned), 1, file);
    fread(&lm.layer[i].bias.c, sizeof(unsigned), 1, file);
    lm.layer[i].bias.data = malloc(lm.layer[i].bias.r * lm.layer[i].bias.c * sizeof(float));
    fread(lm.layer[i].bias.data, sizeof(float), lm.layer[i].bias.r * lm.layer[i].bias.c, file);
  }
  return lm;
}

void mainRender(WindowFrame *frame, Window window, void **GUI){
  RECT rectBuf;
  PAINTBUFFERSTART(contx, rectBuf);

  SetBkMode(contx, TRANSPARENT);
  setPaintPen(paintH, PS_NULL, 0, 0);
  setPaintBrush(paintH, 0x140000);
  SelectObject(contx, paintH->pen);
  SelectObject(contx, paintH->brush);
  Rectangle(contx, 0, 0, clientW, clientH);

  GUIClickBox *hitboxes = GUI[0];
  for(int i = 0; i < 15; ++i){
    drawRect(hitboxes[i].rect, paintH, contx);
  }

  drawTextBox(GUI[1], paintH, contx);
  
  PAINTBUFFEREND(contx);
}

void mainTick(WindowFrame *frame, Window window){
  GUIClickBox *hitboxes = frame->GUI[0];
  GUITextBox *txtbox = frame->GUI[1];

  RECT rectBuf;
  GetClientRect(window, &rectBuf);
  int clientW = rectBuf.right - rectBuf.left;
  int clientH = rectBuf.bottom - rectBuf.top;
  for(int i = 0; i < 3; ++i){
    for(int j = 0; j < 5; ++j){
      setRectPos(hitboxes[i + j * 3].rect, clientW / 2 - 30 + 20 * i, clientH / 2 - 80 + 20 * j);
    }
  }

  int someUpdate = 0;
  for(int i = 0; i < 15; ++i){
    int mouseState = tickClickBox(&hitboxes[i], frame->mouseH.button, frame->mouseH.x, frame->mouseH.y);
    if(mouseState == MOUSE_LEFT_BTN) someUpdate = setRectBrush(hitboxes[i].rect, 0) ? 1 : someUpdate;
    else if(mouseState == MOUSE_RIGHT_BTN) someUpdate = setRectBrush(hitboxes[i].rect, 0xdddddd) ? 1 : someUpdate;
  }

  if(someUpdate){
    LayerModel *lm = frame->GUI[2];
    Mat *in = frame->GUI[3];
    Mat *out = frame->GUI[4];

    for(int i = 0; i < 3; ++i){
      for(int j = 0; j < 5; ++j){
        in->data[i + j * 3] = hitboxes[i + j * 3].rect->brushColor ? 0.f : 1.f;
      }
    }

    outputLayerModel(*lm, *in, out);

    int guess = 9;
    for(; guess >= 0; --guess) if(out->data[guess] > 0.5f) break;
    if(guess < 0) setTextContent(txtbox, "?");
    else{
      char digit[2] = {'0' + guess, 0};
      setTextContent(txtbox, digit);
    }

    destroyMat(out);
  }

  setTextRectToFit(txtbox, window);
  setRectPos(&txtbox->rect, clientW / 2 - txtbox->rect.w / 2, clientH / 2 + 40);

  InvalidateRect(window, 0, 0);
}

int main(){
  FILE *f = fopen("Model.bin", "rb");
  if(!f){
    puts("File could not be opened");
    return 1;
  }
  LayerModel lm = loadLayerModel(f);
  fclose(f);

  Mat input = newMat(1, 15), output = {};

  WindowFrame frame = newWindowFrame();
  frame.paintProc = mainRender;
  int count = 0;
  
  GUIRect image[15] = {};
  GUIClickBox hitboxes[15] = {};
  for(int i = 0; i < 3; ++i){
    for(int j = 0; j < 5; ++j){
        GUIRect *ref = image + (i + j * 3);
        setRectPos(ref, 20 + 20 * i, 20 + 20 * j);
        setRectDim(ref, 20, 20);
        setRectPen(ref, PS_SOLID, 0, 0x808080);
        setRectBrush(ref, 0xdddddd);
        hitboxes[i + j * 3].rect = ref;
    }
  }

  GUITextBox txtbox = {};
  char digit[2];
  setTextBuffer(&txtbox, digit, 1);
  setTextContent(&txtbox, "?");
  setTextFontColor(&txtbox, 0);
  setTextPadding(&txtbox, 12);
  setRectPen(&txtbox.rect, 1, 2, 0);
  setRectBrush(&txtbox.rect, 0xdddddd);
  
  void *GUIElements[] = {hitboxes, &txtbox, &lm, &input, &output};
  frame.GUI = GUIElements;
  
  Window window = newWindow(&frame, "AI manager", 0x80000000, 0x80000000, 0x80000000, 0x80000000);
  ShowWindow(window, 1);
  updateCursor(&frame.curH);
  
  Event event;
  while (IsWindow(window)) {
    if (sysTapped(VK_ESCAPE)) {
      DestroyWindow(window);
      break;
    }
    
    handleEvents(&event, 0);
    if(frame.bitmap){
      mainTick(&frame, window);
      UpdateWindow(window);
    }

    Sleep(10);
  }

  closeWindowFrame(&frame);

  freeMat(output);
  freeMat(input);
  freeLayerModel(lm);

  puts("Exitted normally");
}