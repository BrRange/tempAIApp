#ifndef WINFRAMEGUIH
#define WINFRAMEGUIH

#define PAINTBUFFERSTART(DeviceContext, WinRect)\
GetClientRect(window, &(WinRect));\
int clientW = (WinRect).right - (WinRect).left;\
int clientH = (WinRect).bottom - (WinRect).top;\
if(clientW <= 0 || clientH <= 0) return;\
struct paintHandler *paintH = &frame->paintH;\
HDC hdc = BeginPaint(window, &paintH->painter);\
HDC (DeviceContext) = CreateCompatibleDC(hdc);\
HBITMAP refBitmap = (HBITMAP)SelectObject((DeviceContext), frame->bitmap)

#define PAINTBUFFEREND(DeviceContext)\
BitBlt(hdc, 0, 0,\
paintH->painter.rcPaint.right - paintH->painter.rcPaint.left, paintH->painter.rcPaint.bottom - paintH->painter.rcPaint.top,\
DeviceContext, 0, 0, SRCCOPY);\
SelectObject(DeviceContext, refBitmap);\
DeleteDC(DeviceContext);\
EndPaint(window, &paintH->painter)

#include "winframe.h"

Register strLen(const char *str){
  const char *end = str;
  while(*end) ++end;
  return (Register)(end - str);
}

void strCopy(const char *src, char *dest, Register maxLen){
  while(*src && maxLen > 0){
    *dest = *src;
    ++src;
    ++dest;
    --maxLen;
  }
}

struct GUIRect{
  int x, y, w, h;
  unsigned penStyle, penWidth, penColor;
  unsigned brushColor;
};
typedef struct GUIRect GUIRect;

void setRectPos(GUIRect *rect, int x, int y){
  rect->x = x;
  rect->y = y;
}

void setRectDim(GUIRect *rect, int w, int h){
  rect->w = w;
  rect->h = h;
}

void setRectPen(GUIRect *rect, unsigned style, unsigned width, unsigned color){
  rect->penStyle = style;
  rect->penWidth = width;
  rect->penColor = color;
}

int setRectBrush(GUIRect *rect, unsigned color){
  int diff = rect->brushColor != color;
  rect->brushColor = color;
  return diff;
}

void drawRect(GUIRect *rect, struct paintHandler *paintH, HDC hdc){
  setPaintPen(paintH, rect->penStyle, rect->penWidth, rect->penColor);
  setPaintBrush(paintH, rect->brushColor);

  SelectObject(hdc, paintH->pen);
  SelectObject(hdc, paintH->brush);

  Rectangle(hdc, rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
}

void convertRectToWinRect(GUIRect *rect, RECT *winRect){
  *winRect = (RECT){rect->x - rect->penWidth, rect->y - rect->penWidth, rect->x + rect->w + rect->penWidth, rect->y + rect->h + rect->penWidth};
}

void getRectCenter(GUIRect *rect, int *x, int *y){
  *x = rect->x + rect->w / 2;
  *y = rect->y + rect->h / 2;
}

int getCollidingRect(const GUIRect *rect, int x, int y){
  x -= rect->x;
  y -= rect->y;
  return x >= 0 && x <= rect->w && y >= 0 && y <= rect->h;
}

struct GUITextBox{
  char *text;
  Register textLen;
  Register maxLen;
  unsigned fontColor;
  int padding;
  GUIRect rect;
};
typedef struct GUITextBox GUITextBox;

void setTextStatic(GUITextBox *txtbox, const char *text){
  txtbox->text = (char*)text;
  txtbox->textLen = strLen(text);
  txtbox->maxLen = -1;
}

void setTextBuffer(GUITextBox *txtbox, char *text, Register maxLen){
  txtbox->text = text;
  txtbox->textLen = strLen(text);
  txtbox->maxLen = maxLen;
}

void setTextContent(GUITextBox *txtbox, const char *text){
  strCopy(text, txtbox->text, txtbox->maxLen);
  txtbox->textLen = strLen(text);
}

void setTextFontColor(GUITextBox *txtbox, unsigned color){
  txtbox->fontColor = color;
}

void setTextPadding(GUITextBox *txtbox, int padding){
  txtbox->padding = padding;
}

void setTextRectToFit(GUITextBox *txtbox, Window window){
  SIZE txtdim;
  HDC context = GetDC(window);
  GetTextExtentPoint32(context, txtbox->text, txtbox->textLen, &txtdim);
  setRectDim(&txtbox->rect, txtdim.cx + 2 * txtbox->padding, txtdim.cy + 2 * txtbox->padding);
  ReleaseDC(window, context);
}

void drawTextBox(GUITextBox *txtbox, struct paintHandler *paintH, HDC hdc){
  drawRect(&txtbox->rect, paintH, hdc);
  SetTextColor(hdc, txtbox->fontColor);
  TextOut(hdc, txtbox->rect.x + txtbox->padding, txtbox->rect.y + txtbox->padding, txtbox->text, txtbox->textLen);
}

struct GUIClickBox{
  GUIRect *rect;
  int clicked;
};
typedef struct GUIClickBox GUIClickBox;

int tickClickBox(GUIClickBox *clickBox, int mouseBtn, int x, int y){
  if (!mouseBtn){
    clickBox->clicked = 0;
    return 0;
  }
  if(getCollidingRect(clickBox->rect, x, y))
    clickBox->clicked |= (clickBox->clicked & mouseBtn ? mouseBtn | 0x80000000 : mouseBtn);
  else
    clickBox->clicked &= ~(mouseBtn | 0x80000000);
  return clickBox->clicked & (mouseBtn | 0x80000000);
} 

#endif