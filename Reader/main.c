#include "Windows.h"

#include "Structures.h"
#include "DUI_Menu.h"
#include <malloc.h>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int iCmdShow)
{
    static char szClassName[] = "DUI_Reader";
    HWND        hwnd;
    MSG         messages;
    WNDCLASSEX  wincl;

    wincl.cbSize        = sizeof(WNDCLASSEX);
    wincl.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wincl.lpfnWndProc   = WindowProcedure;
    wincl.cbClsExtra    = 0;
    wincl.cbWndExtra    = 0;
    wincl.hInstance     = hThisInstance;
    wincl.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wincl.lpszClassName = szClassName;
    wincl.lpszMenuName  = "DUI_Menu";

    if (!RegisterClassEx(&wincl))
    {
        return 0;
    }

    hwnd = CreateWindowEx(0,                   /* Extended possibilites for variation */
                          szClassName,         /* Classname */
                          "SSRV Reader",       /* Title Text */
                          WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, /* default window */
                          CW_USEDEFAULT,       /* Windows decides the position */
                          CW_USEDEFAULT,       /* where the window ends up on the screen */
                          544,                 /* The programs width */
                          375,                 /* and height in pixels */
                          HWND_DESKTOP,        /* The window is a child-window to desktop */
                          NULL,                /* No menu */
                          hThisInstance,       /* Program Instance handler */
                          lpszArgument         /* Window Creation data */
                         );

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC                 hdc;
    PAINTSTRUCT         ps;
    RECT                rect;
    TEXTMETRIC          tm;
    CREATESTRUCT        *cs;
    HMENU               hMenu;
    OPENFILENAME        ofn;
    static HFONT        hfont;

    char                bufferToNewFilename[260];

    static WNDPARAMS_T  wndParams;
    static BUFFER_T     buffer;
    static VIEW_T       view;
    static SCROLLING_T  scrolling;

    int                 error;

    switch (message)
    {
        case WM_CREATE:
        {
            hdc = GetDC(hwnd);
            hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
            SelectObject(hdc, hfont);
            GetTextMetrics(hdc, &tm);
            wndParams.WChar = tm.tmMaxCharWidth;
            wndParams.HChar = tm.tmHeight + tm.tmExternalLeading;
            GetClientRect(hwnd, &rect);
            wndParams.WClient = rect.right;
            wndParams.HClient = rect.bottom;

            buffer.buffer = NULL;
            view.offsetsToNewLine = NULL;

            cs = (CREATESTRUCT*)lParam;
            buffer.filename = (char*)(cs->lpCreateParams);
            if(strlen(buffer.filename) != 0)
            {
                error = LoadBuffer(&buffer);
                if (error == -1)
                {
                    MessageBox(hwnd, "No such file or file is too big to load", "Error", MB_ICONERROR);
                }
            }
            view.mode = WRAP_OFF;
            MakeView(hwnd, &view, &buffer, &wndParams, &scrolling);

            ReleaseDC(hwnd, hdc);
            break;
        }
        case WM_PAINT:
        {
            hdc = BeginPaint(hwnd, &ps);
            if (buffer.filename != NULL)
            {
                int horizontalOffset = (view.mode == WRAP_OFF)
                                        ? scrolling.HScrollPos
                                        : 0;
                for (long i = scrolling.VScrollPos; (i <= scrolling.VScrollPos + view.ClientHBuffer) && (i < view.offsetCount); i++)
                {
                    long y   = wndParams.HChar * (i - scrolling.VScrollPos);
                    long len = max(view.offsetsToNewLine[i + 1] - view.offsetsToNewLine[i] - horizontalOffset, 0);
                    TextOut(hdc, 0, y, buffer.buffer + view.offsetsToNewLine[i] + horizontalOffset, len);
                }
            }
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            int oldClientWBuffer = view.ClientWBuffer;
            wndParams.WClient = LOWORD(lParam);
            wndParams.HClient = HIWORD(lParam);
            view.ClientWBuffer = (wndParams.WClient / wndParams.WChar) - 1;
            view.ClientHBuffer = (wndParams.HClient / wndParams.HChar) - 1;
            if(buffer.filename != NULL)
            {
                if (view.mode == WRAP_ON && oldClientWBuffer != view.ClientWBuffer)
                {
                    ClearView(&view);
                    MakeView(hwnd, &view, &buffer, &wndParams, &scrolling);
                }
                UpdateScroll(hwnd, &scrolling, &view, &buffer);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_VSCROLL:
        {
            switch(LOWORD(wParam))
            {
                case SB_TOP:
                    scrolling.VScrollPos = 0;
                    break;
                case SB_BOTTOM:
                    scrolling.VScrollPos += scrolling.VScrollMax * scrolling.coeffScrolling - scrolling.VScrollPos;
                    break;
                case SB_LINEUP:
                    scrolling.VScrollPos -= 1;
                    break;
                case SB_LINEDOWN:
                    scrolling.VScrollPos += 1;
                    break;
                case SB_PAGEUP:
                    scrolling.VScrollPos += min(-1, - view.ClientHBuffer);
                    break;
                case SB_PAGEDOWN:
                    scrolling.VScrollPos += max(1, view.ClientHBuffer);
                    break;
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    scrolling.VScrollPos = (HIWORD(wParam)) * scrolling.coeffScrolling;
                    if(HIWORD(wParam) == scrolling.VScrollMax)
                    {
                        scrolling.VScrollPos = view.offsetCount - view.ClientHBuffer;
                    }
                    break;
                default :
                    break;
            }
            scrolling.VScrollPos = max(0, min(scrolling.VScrollPos, view.offsetCount - view.ClientHBuffer));
            if(scrolling.VScrollPos != GetScrollPos(hwnd, SB_VERT) / scrolling.coeffScrolling)
            {
                SetScrollPos(hwnd, SB_VERT, scrolling.VScrollPos / scrolling.coeffScrolling, TRUE);
                UpdateScroll(hwnd, &scrolling, &view, &buffer);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_HSCROLL:
        {
            if(view.mode == WRAP_ON || buffer.buffer == NULL)
            {
                break;
            }
            switch(LOWORD(wParam))
            {
                case SB_TOP:
                    scrolling.HScrollPos = 0;
                    break;
                case SB_BOTTOM:
                    scrolling.HScrollPos += scrolling.HScrollMax - scrolling.HScrollPos;
                    break;
                case SB_LINEUP:
                    scrolling.HScrollPos -= 1;
                    break;
                case SB_LINEDOWN:
                    scrolling.HScrollPos += 1;
                    break;
                case SB_PAGEUP:
                    scrolling.HScrollPos += min(-1, - view.ClientWBuffer);
                    break;
                case SB_PAGEDOWN:
                    scrolling.HScrollPos += max(1, view.ClientWBuffer);
                    break;
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    scrolling.HScrollPos = (HIWORD(wParam));
                    if(HIWORD(wParam) == scrolling.HScrollMax)
                    {
                        scrolling.HScrollPos = view.MaxStrLength - view.ClientWBuffer;
                    }
                    break;
                default :
                    break;
            }
            scrolling.HScrollPos = max(0, min(scrolling.HScrollPos, view.MaxStrLength - view.ClientWBuffer));
            if(scrolling.HScrollPos != GetScrollPos(hwnd, SB_HORZ))
            {
                SetScrollPos(hwnd, SB_HORZ, scrolling.HScrollPos, TRUE);
                UpdateScroll(hwnd, &scrolling, &view, &buffer);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case VK_UP:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
                    break;
                }
                case VK_DOWN:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
                    break;
                }
                case VK_LEFT:
                {
                    SendMessage(hwnd, WM_HSCROLL, SB_LINEUP, 0L);
                    break;
                }
                case VK_RIGHT:
                {
                    SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0L);
                    break;
                }
                case VK_HOME:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0L);
                    break;
                }
                case VK_END:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0L);
                    break;
                }
                case VK_NEXT:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
                    break;
                }
                case VK_PRIOR:
                {
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
                    break;
                }
            }
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDM_OPEN:
                {
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.lpstrFile = bufferToNewFilename;
                    ofn.nMaxFile = sizeof(bufferToNewFilename);
                    ofn.lpstrFile[0] = '\0';
                    ofn.lpstrFilter = "Text\0*.TXT\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 0;
                    ofn.lpstrInitialDir = NULL;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    ofn.hwndOwner = hwnd;
                    if(GetOpenFileName(&ofn) == TRUE)
                    {
                        ClearView(&view);
                        ClearBuffer(&buffer);
                        ClearScroll(hwnd, &scrolling);
                        view.offsetToCurrentLine = 0;
                        buffer.filename = ofn.lpstrFile;
                        LoadBuffer(&buffer);

                        view.mode = WRAP_OFF;
                        hMenu = GetMenu(hwnd);
                        CheckMenuItem(hMenu, IDM_WRAPOFF, MF_CHECKED);
                        CheckMenuItem(hMenu, IDM_WRAPON, MF_UNCHECKED);
                        MakeView(hwnd, &view, &buffer, &wndParams, &scrolling);
                        UpdateScroll(hwnd, &scrolling, &view, &buffer);
                        InvalidateRect(hwnd, NULL, TRUE);
                        UpdateWindow(hwnd);
                    }
                    break;
                }
                case IDM_CLOSE:
                {
                    ClearBuffer(&buffer);
                    ClearView(&view);
                    ClearScroll(hwnd, &scrolling);
                    ShowScrollBar(hwnd, SB_VERT, FALSE);
                    ShowScrollBar(hwnd, SB_HORZ, FALSE);
                    InvalidateRect(hwnd, NULL, TRUE);
                    ZeroMemory(&ofn, sizeof(ofn));
                    break;
                }
                case IDM_EXIT:
                {
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    break;
                }
                case IDM_WRAPOFF:
                {
                    if(view.mode == WRAP_OFF || buffer.buffer == NULL)
                    {
                        break;
                    }
                    view.mode = WRAP_OFF;
                    ClearView(&view);
                    MakeView(hwnd, &view, &buffer, &wndParams, &scrolling);
                    InvalidateRect(hwnd, NULL, TRUE);
                    hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_WRAPOFF, MF_CHECKED);
                    CheckMenuItem(hMenu, IDM_WRAPON, MF_UNCHECKED);
                    break;
                }
                case IDM_WRAPON:
                {
                    if(view.mode == WRAP_ON || buffer.buffer == NULL)
                    {
                        break;
                    }
                    view.mode = WRAP_ON;
                    ClearView(&view);
                    MakeView(hwnd, &view, &buffer, &wndParams, &scrolling);
                    InvalidateRect(hwnd, NULL, TRUE);
                    hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_WRAPON, MF_CHECKED);
                    CheckMenuItem(hMenu, IDM_WRAPOFF, MF_UNCHECKED);
                    break;
                }
            }
            break;
        }
        case WM_CLOSE:
        case WM_DESTROY:
        {
            ClearView(&view);
            ClearBuffer(&buffer);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
