#include <stdio.h>
#include "winframeGUI.h"

void mainRender(WindowFrame *frame, Window window, void **GUI){
  RECT rectBuf;
  GetClientRect(window, &rectBuf);
  int clientW = rectBuf.right - rectBuf.left;
  int clientH = rectBuf.bottom - rectBuf.top;

  if(clientW <= 0 || clientH <= 0) return;

  struct paintHandler *paintH = &frame->paintH;
  HDC hdc = BeginPaint(window, &paintH->painter);

  HDC contx = CreateCompatibleDC(hdc);
  HBITMAP refBitmap = (HBITMAP)SelectObject(contx, frame->bitmap);

  SetBkMode(contx, TRANSPARENT);
  setPaintPen(paintH, PS_SOLID, 1, 0x20a5da);
  setPaintBrush(paintH, RGB(0, 0, 20));
  SelectObject(contx, paintH->pen);
  SelectObject(contx, paintH->brush);

  Rectangle(contx, 0, 0, clientW, clientH);

  drawTextBox(GUI[0], paintH, contx);
  
  BitBlt(
    hdc,
    0,
    0,
    paintH->painter.rcPaint.right - paintH->painter.rcPaint.left,
    paintH->painter.rcPaint.bottom - paintH->painter.rcPaint.top,
    contx,
    0,
    0,
    SRCCOPY
  );

  SelectObject(contx, refBitmap);
  DeleteDC(contx);
  EndPaint(window, &paintH->painter);
}

void mainTick(WindowFrame *frame, Window window){
  RECT rectBuf;
  GetClientRect(window, &rectBuf);

  GUITextBox *txtbox = frame->GUI[0];
  int wincx = (rectBuf.left + rectBuf.right) >> 1;
  int wincy = (rectBuf.top + rectBuf.bottom) >> 1;

  int mouseState = tickClickBox(frame->GUI[1], frame->mouseH.button, frame->mouseH.x, frame->mouseH.y);

  if(mouseState > 0){
    int *count = frame->GUI[2];
    switch(mouseState){
    case MOUSE_LEFT_BTN:{
      *count += 1;
      break;
    }
    case MOUSE_MIDDLE_BTN:{
      *count = 0;
      break;
    }
    case MOUSE_RIGHT_BTN:{
      *count -= 1;
      break;
    }
    }
    snprintf(txtbox->text, txtbox->maxLen, "Count is at %i", *count);
    txtbox->textLen = strlen(txtbox->text);
  }

  setTextRectToFit(frame->GUI[0], window);
  setRectPos(&txtbox->rect, wincx - txtbox->rect.w / 2, wincy - txtbox->rect.h / 2);

  InvalidateRect(window, 0, 0);
}

int main() {
  WindowFrame frame = newWindowFrame();
  frame.paintProc = mainRender;
  int count = 0;
  
  char mutableStr[20] = {0};
  GUITextBox txtbox = {0};
  setTextBuffer(&txtbox, mutableStr, 20);
  setTextContent(&txtbox, "Count is 0");
  setTextFontColor(&txtbox, 0x0000ff);
  setTextPadding(&txtbox, 20);
  setRectPen(&txtbox.rect, PS_SOLID, 1, 0x0000ff);
  setRectBrush(&txtbox.rect, 0xdddddd);

  GUIClickBox countClick = {0};
  countClick.rect = &txtbox.rect;
  
  void *GUIElements[] = {&txtbox, &countClick, &count};
  frame.GUI = GUIElements;
  
  Window window = newWindow(&frame, "AI manager", 0x80000000, 0x80000000, 0x80000000, 0x80000000);
  ShowWindow(window, 1);
  updateCursor(&frame.curH);
  
  Event event;
  while (IsWindow(window)) {
    if (sysTapped(VK_ESCAPE)) {
      DestroyWindow(window);
    }
    
    handleEvents(&event, 0);
    if(frame.bitmap){
      mainTick(&frame, window);
      UpdateWindow(window);
    }

    Sleep(1);
  }

  closeWindowFrame(&frame);

  puts("Program exit");
}