#ifndef WINFRAMEH
#define WINFRAMEH

#include <windows.h>

#define NOCALL static inline

#define MOUSE_LEFT_BTN (1 << 0)
#define MOUSE_MIDDLE_BTN (1 << 1)
#define MOUSE_RIGHT_BTN (1 << 2)
#define MOUSE_EXTRA_BTN(x) (1 << (2 + (x)))

#define CURSOR_INDEX_IDLE 0
#define CURSOR_INDEX_POINT 1
#define CURSOR_INDEX_WAIT 2
#define CURSOR_INDEX_HORIZONTAL 3
#define CURSOR_INDEX_VERTICAL 4
#define CURSOR_INDEX_DOWNSLOPE 5
#define CURSOR_INDEX_UPSLOPE 6

typedef HWND Window;
typedef LPARAM Register;
typedef MSG Event;

int sysPressed(int key) { return GetAsyncKeyState(key) & 0x8000; }

int sysTapped(int key) { return GetAsyncKeyState(key) & 0x0001; }

struct keyboardHandler {
  char key[6];
};

void pressKey(struct keyboardHandler *keyH, char key) {
  for (int i = 0; i < 6; ++i) {
    if (!keyH->key[i]) {
      keyH->key[i] = key;
      break;
    }
  }
}

void releaseKey(struct keyboardHandler *keyH, char key) {
  for (int i = 0; i < 6; ++i) {
    if (keyH->key[i] == key) {
      keyH->key[i] = 0;
      break;
    }
  }
}

int hasKey(struct keyboardHandler *keyH, char key) {
  for (int i = 0; i < 6; ++i)
    if (keyH->key[i] == key)
      return 1;
  return 0;
}

void clearKeyboardHandler(struct keyboardHandler *keyH) {
  for (int i = 0; i < 6; ++i)
    keyH->key[i] = 0;
}

struct mouseHandler {
  int button;
  union {
    struct {
      short x, y;
    };
    int pos;
  };
};

void setMousePos(struct mouseHandler *mouseH, int pos) { mouseH->pos = pos; }

void getMousePos(struct mouseHandler *mouseH, int *x, int *y) {
  *x = mouseH->x;
  *y = mouseH->y;
}

void clickButton(struct mouseHandler *mouseH, int btn) {
  mouseH->button |= btn;
}

void releaseButton(struct mouseHandler *mouseH, int btn) {
  mouseH->button &= ~btn;
}

int hasButton(struct mouseHandler *mouseH, int btn) {
  return mouseH->button & btn;
}

void clearMouseHandler(struct mouseHandler *mouseH) { mouseH->button = 0; }

struct cursorHandler {
  int currentCursor;
  union {
    HCURSOR cursor[7];
    struct {
      HCURSOR idleCursor, pointCursor, waitCursor, borderCursor[4];
    };
  };
};

void updateCursor(struct cursorHandler *curH) {
  SetCursor(curH->cursor[curH->currentCursor]);
}

int setCurrentCursor(struct cursorHandler *curH, int cursor) {
  if (curH->currentCursor == cursor) {
    return 0;
  }
  curH->currentCursor = cursor;
  return 1;
}

struct paintHandler {
  PAINTSTRUCT painter;
  unsigned penStyle, penWidth, penColor;
  HPEN pen;
  unsigned brushColor;
  HBRUSH brush;
};

void setPaintPen(struct paintHandler *paintH, unsigned style, unsigned width,
                 unsigned color) {
  if ((paintH->penStyle != style) || (paintH->penWidth != width) ||
      (paintH->penColor != color)) {
    if (paintH->pen)
      DeleteObject(paintH->pen);
    paintH->pen = CreatePen(style, width, color);
    paintH->penStyle = style;
    paintH->penWidth = width;
    paintH->penColor = color;
  }
}

void setPaintBrush(struct paintHandler *paintH, unsigned color) {
  if (paintH->brushColor != color) {
    if (paintH->brush)
      DeleteObject(paintH->brush);
    paintH->brush = CreateSolidBrush(color);
    paintH->brushColor = color;
  }
}

Register CALLBACK procedure(Window, unsigned code, size_t flag, Register);

struct WindowFrame {
  WNDCLASS class;
  struct cursorHandler curH;
  struct keyboardHandler keyH;
  struct mouseHandler mouseH;
  struct paintHandler paintH;
  HBITMAP bitmap;
  void **GUI;
  void (*paintProc)(struct WindowFrame *, Window, void **GUI);
};
typedef struct WindowFrame WindowFrame;

WindowFrame newWindowFrame() {
  WindowFrame frame = {};
  frame.class.lpfnWndProc = procedure;
  frame.class.hInstance = GetModuleHandle(NULL);
  frame.class.lpszClassName = "Window Frame";

  frame.curH.idleCursor = LoadCursor(0, IDC_ARROW);
  frame.curH.pointCursor = LoadCursor(0, IDC_HAND);
  frame.curH.waitCursor = LoadCursor(0, IDC_WAIT);
  frame.curH.borderCursor[0] = LoadCursor(0, IDC_SIZEWE);
  frame.curH.borderCursor[1] = LoadCursor(0, IDC_SIZENS);
  frame.curH.borderCursor[2] = LoadCursor(0, IDC_SIZENWSE);
  frame.curH.borderCursor[3] = LoadCursor(0, IDC_SIZENESW);

  RegisterClass(&frame.class);

  return frame;
}

void setBitmap(WindowFrame *frame, Window window){
  RECT rectBuf;
  GetClientRect(window, &rectBuf);
  if(frame->bitmap){
    DeleteObject(frame->bitmap);
    frame->bitmap = NULL;
  }
  HDC context = GetDC(window);
  int w = rectBuf.right - rectBuf.left;
  int h = rectBuf.bottom - rectBuf.top;
  if(w <= 0 || h <= 0){
    frame->bitmap = 0;
    ReleaseDC(window, context);
    return;  
  }
  frame->bitmap = CreateCompatibleBitmap(context, w, h);
  ReleaseDC(window, context);
}

Window newWindow(WindowFrame *frame, const char *name, int x, int y, int w, int h) {
  Window window = CreateWindowEx(
    0, frame->class.lpszClassName,
    name, WS_TILEDWINDOW,
    x, y, w, h,
    0, 0,
    frame->class.hInstance, 0
  );
  if (!window) {
    DWORD err = GetLastError();
    printf("CreateWindowEx failed with error %lu\n", err);
    exit(1);
  }
  SetWindowLongPtr(window, GWLP_USERDATA, (size_t)frame);
  setBitmap(frame, window);
  return window;
}

void closeWindowFrame(WindowFrame *frame) {
  if (frame->paintH.pen)
    DeleteObject(frame->paintH.pen);
  if (frame->paintH.brush)
    DeleteObject(frame->paintH.brush);
  UnregisterClass(frame->class.lpszClassName, frame->class.hInstance);
  *frame = (WindowFrame){};
}

void handleEvents(Event *event, Window window) {
  while (PeekMessage(event, window, 0, 0, 1)) {
    TranslateMessage(event);
    DispatchMessage(event);
  }
}

Register CALLBACK procedure(Window window, unsigned code, size_t flag, Register data) {
  switch (code) {
  case WM_CLOSE: {
    DestroyWindow(window);
    break;
  }
  case WM_DESTROY: {
    PostQuitMessage(0);
    break;
  }
  case WM_KEYDOWN: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    pressKey(&frame->keyH, (char)flag);
    break;
  }
  case WM_KEYUP: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    releaseKey(&frame->keyH, (char)flag);
    break;
  }
  case WM_LBUTTONDOWN: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    clickButton(&frame->mouseH, MOUSE_LEFT_BTN);
    break;
  }
  case WM_LBUTTONUP: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    releaseButton(&frame->mouseH, MOUSE_LEFT_BTN);
    break;
  }
  case WM_MBUTTONDOWN: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    clickButton(&frame->mouseH, MOUSE_MIDDLE_BTN);
    break;
  }
  case WM_MBUTTONUP: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    releaseButton(&frame->mouseH, MOUSE_MIDDLE_BTN);
    break;
  }
  case WM_RBUTTONDOWN: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    clickButton(&frame->mouseH, MOUSE_RIGHT_BTN);
    break;
  }
  case WM_RBUTTONUP: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    releaseButton(&frame->mouseH, MOUSE_RIGHT_BTN);
    break;
  }
  case WM_XBUTTONDOWN: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    clickButton(&frame->mouseH, MOUSE_EXTRA_BTN(flag >> 16));
    break;
  }
  case WM_XBUTTONUP: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    releaseButton(&frame->mouseH, MOUSE_EXTRA_BTN(flag >> 16));
    break;
  }
  case WM_MOUSEMOVE: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    setMousePos(&frame->mouseH, data);
    break;
  }
  case WM_KILLFOCUS: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    clearMouseHandler(&frame->mouseH);
    clearKeyboardHandler(&frame->keyH);
    break;
  }
  case WM_SIZE: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    if(flag == SIZE_MINIMIZED){
      DeleteObject(frame->bitmap);
      frame->bitmap = 0;
    } else{
      setBitmap(frame, window);
      InvalidateRect(window, 0, 0);
    }
    break;
  }
  case WM_SETCURSOR: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    int cursorChanged = 0;
    switch (data & 0xffff) {
    case HTCAPTION: [[fallthrough]];
    case HTCLIENT:
      cursorChanged = setCurrentCursor(&frame->curH, CURSOR_INDEX_IDLE);
      break;
    case HTLEFT: [[fallthrough]];
    case HTRIGHT:
      cursorChanged = setCurrentCursor(&frame->curH, CURSOR_INDEX_HORIZONTAL);
      break;
    case HTBOTTOM: [[fallthrough]];
    case HTTOP:
      cursorChanged = setCurrentCursor(&frame->curH, CURSOR_INDEX_VERTICAL);
      break;
    case HTBOTTOMRIGHT: [[fallthrough]];
    case HTTOPLEFT:
      cursorChanged = setCurrentCursor(&frame->curH, CURSOR_INDEX_DOWNSLOPE);
      break;
    case HTTOPRIGHT: [[fallthrough]];
    case HTBOTTOMLEFT:
      cursorChanged = setCurrentCursor(&frame->curH, CURSOR_INDEX_UPSLOPE);
      break;
    }
    if (cursorChanged)
      updateCursor(&frame->curH);
    break;
  }
  case WM_PAINT: {
    WindowFrame *frame = (WindowFrame *)GetWindowLongPtr(window, GWLP_USERDATA);
    if (frame->paintProc)
      frame->paintProc(frame, window, frame->GUI);
    break;
  }
  default:
    return DefWindowProc(window, code, flag, data);
  }
  return 1;
}

#endif
