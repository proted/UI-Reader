#include "Structures.h"

#include <stdio.h>
#include <malloc.h>
#include <math.h>

#define MAX_SCROLL_RANGE (0xffff)

int local_MakeWrapOFF(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, SCROLLING_T* scroll);
int local_MakeWrapON(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, SCROLLING_T* scroll);

/*
 Функция читает текст из файла
 @param     buffer (in/out) - структура хранения буфера
 @return    int - 0 при успехе, -1 при ошибке
*/
int LoadBuffer(BUFFER_T* buffer)
{
    FILE* f = fopen(buffer->filename, "rb+");
    if(f == NULL)
    {
        buffer->bufferLength = 0;
        buffer->buffer = NULL;
        buffer->filename = NULL;
        buffer->strCount = 0;
        return -1;
    }

    fseek(f, 0, SEEK_END);
    buffer->bufferLength = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer->buffer = (char *) calloc(buffer->bufferLength, sizeof(char));
    if(buffer->buffer == NULL)
    {
        return -1;
    }
    fread(buffer->buffer, sizeof(char), buffer->bufferLength, f);
    buffer->strCount = 1;
    for(int i = 0; i < buffer->bufferLength; i++)
    {
        if(buffer->buffer[i] == '\n')
        {
            buffer->strCount++;
        }
    }
    return 0;
}

/*
 Функция инициализирует структуру VIEW_T для отображения текста в режимах верстка/без_верстки
 @param     hwnd (in)
 @param     view (in/out) - структура представления текста, отображаемого на экране
 @param     buffer (in) - структура хранения текстового буфера
 @param     scroll (in/out) - структура представления полос прокрутки
 @return    int - 0 при успехе, -1 при ошибке
*/
int MakeView(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, WNDPARAMS_T* wndparams, SCROLLING_T* scroll)
{
    view->ClientHBuffer = (wndparams->HClient / wndparams->HChar) - 1;
    view->ClientWBuffer = (wndparams->WClient / wndparams->WChar) - 1;
    switch (view->mode)
    {
        case WRAP_OFF:
        {
            return local_MakeWrapOFF(hwnd, view, buffer, scroll);
        }
        case WRAP_ON:
        {
            return local_MakeWrapON(hwnd, view, buffer, scroll);
        }
    }
    return -1;
}

/*
 Функция осуществляет обновление параметров полос прокрутки
 @param     hwnd (in)
 @param     scroll (in/out) - структура представления полос прокрутки
 @param     view (in) - структура представления текста, отображаемого на экране
 @param     buffer (in) - структура хранения текстового буфера
 @return    int - 0 при успехе, -1 при ошибке
*/
int UpdateScroll(HWND hwnd, SCROLLING_T* scroll, VIEW_T* view, BUFFER_T* buffer)
{
    if(buffer->filename == NULL)
    {
        ShowScrollBar(hwnd, SB_VERT, FALSE);
        ShowScrollBar(hwnd, SB_HORZ, FALSE);
        return 0;
    }
    switch (view->mode)
    {
        case WRAP_OFF:
        {
            if (view->ClientWBuffer <  view->MaxStrLength)
            {
                ShowScrollBar(hwnd, SB_HORZ, TRUE);
                scroll->HScrollMax = max(0, view->MaxStrLength - view->ClientWBuffer);
                scroll->HScrollPos = min(scroll->HScrollPos, scroll->HScrollMax);
                SetScrollRange(hwnd, SB_HORZ, 0, scroll->HScrollMax, TRUE);
                SetScrollPos(hwnd, SB_HORZ, scroll->HScrollPos, TRUE);
            }
            else
            {
                scroll->HScrollPos = scroll->HScrollMax = 0;
                SetScrollRange(hwnd, SB_HORZ, 0, scroll->HScrollMax, TRUE);
                SetScrollPos(hwnd, SB_HORZ, scroll->HScrollPos, TRUE);
                ShowScrollBar(hwnd, SB_HORZ, FALSE);
            }
            if (view->ClientHBuffer < view->offsetCount)
            {
                ShowScrollBar(hwnd, SB_VERT, TRUE);
                if ((view->offsetCount - view->ClientHBuffer) >= MAX_SCROLL_RANGE)
                {
                    scroll->VScrollMax     = MAX_SCROLL_RANGE - 1;
                    scroll->coeffScrolling = (long double)(view->offsetCount - view->ClientHBuffer) / MAX_SCROLL_RANGE;
                }
                else
                {
                    scroll->VScrollMax     = view->offsetCount - view->ClientHBuffer;
                    scroll->coeffScrolling = 1;
                }
                scroll->VScrollPos = max(0, min(scroll->VScrollPos, view->offsetCount - view->ClientHBuffer));
                SetScrollRange(hwnd, SB_VERT, 0, scroll->VScrollMax, TRUE);
                SetScrollPos(hwnd, SB_VERT, scroll->VScrollPos / scroll->coeffScrolling, TRUE);
            }
            else
            {
                ShowScrollBar(hwnd, SB_VERT, FALSE);
            }
            break;
        }
        case WRAP_ON:
        {
            ShowScrollBar(hwnd, SB_HORZ, FALSE);
            if (view->ClientHBuffer < view->offsetCount)
            {
                ShowScrollBar(hwnd, SB_VERT, TRUE);
                if ((view->offsetCount - view->ClientHBuffer) >= MAX_SCROLL_RANGE)
                {
                    scroll->VScrollMax     = MAX_SCROLL_RANGE - 1;
                    scroll->coeffScrolling = (long double)(view->offsetCount - view->ClientHBuffer) / MAX_SCROLL_RANGE;
                }
                else
                {
                    scroll->VScrollMax     = view->offsetCount - view->ClientHBuffer;
                    scroll->coeffScrolling = 1;
                }
                scroll->VScrollPos = max(0, min(scroll->VScrollPos, view->offsetCount - view->ClientHBuffer));
                SetScrollRange(hwnd, SB_VERT, 0, scroll->VScrollMax, TRUE);
                SetScrollPos(hwnd, SB_VERT, scroll->VScrollPos / scroll->coeffScrolling, TRUE);
            }
            else
            {
                ShowScrollBar(hwnd, SB_VERT, FALSE);
            }
            break;
        }
        default:
            break;
    }
    view->offsetToCurrentLine = view->offsetsToNewLine[scroll->VScrollPos];
}

/*
 Функция очищает память в структуре представления VIEW_T
 @param     view (in/out) - структура представления текста, отображаемого на экране
 @return    none
*/
void ClearView(VIEW_T* view)
{
    if(view->offsetsToNewLine != NULL)
    {
        free(view->offsetsToNewLine);
        view->offsetsToNewLine = NULL;
    }
}

/*
 Функция очищает память в структуре хранения текстового буфера BUFFER_T
 @param     buffer (in/out) - структура хранения текстового буфера
 @return    none
*/
void ClearBuffer(BUFFER_T* buffer)
{
    if(buffer->buffer != NULL)
    {
        free(buffer->buffer);
        buffer->buffer = NULL;
    }
    if(buffer->filename != NULL)
    {
        buffer->filename = NULL;
    }
}

/*
 Функция приводит к дефолтному состоянию структуру представления полос прокрутки SCROLLING_T
 @param     scroll (in/out) - структура представления полос прокрутки SCROLLING_T
 @return    none
*/
void ClearScroll(HWND hwnd, SCROLLING_T* scroll)
{
    scroll->HScrollPos = 0;
    scroll->HScrollMax = 0;
    scroll->VScrollPos = 0;
    scroll->VScrollMax = 0;
    scroll->coeffScrolling = 1.0;
    SetScrollPos(hwnd, SB_VERT, 0, TRUE);
    SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
    SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
    SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
}

/*
 Функция инициализирует структуру VIEW_T для отображения текста в режиме без верстки
 @param     hwnd (in)
 @param     view (in/out) - структура представления текста
 @param     buffer (in) - структура хранения текстового буфера
 @return    int - 0 при успехе, -1 при ошибке
*/
int local_MakeWrapOFF(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, SCROLLING_T* scroll)
{
    int switcingModeOffset = (view->offsetToCurrentLine != 0)
                             ? view->offsetToCurrentLine
                             : 0;
    view->offsetCount = buffer->strCount + 1;
    view->offsetsToNewLine = (long*)calloc(view->offsetCount, sizeof(long));
    view->offsetsToNewLine[0] = 0;
    view->MaxStrLength = 0;
    for(int i = 0, j = 1; i < buffer->bufferLength; i++)
    {
        if(buffer->buffer[i] == '\n')
        {
            view->offsetsToNewLine[j] = i + 1;
            view->MaxStrLength = max(view->MaxStrLength, view->offsetsToNewLine[j] - view->offsetsToNewLine[j - 1]);
            j++;
        }
        if(switcingModeOffset != 0 && i == switcingModeOffset)
        {
            scroll->VScrollPos = j - 1;
        }
    }
    view->offsetsToNewLine[buffer->strCount] = buffer->bufferLength;
    UpdateScroll(hwnd, scroll, view, buffer);
    return 0;
}

/*
 Функция инициализирует структуру VIEW_T для отображения текста в режиме с версткой
 @param     hwnd (in)
 @param     view (in/out) - структура представления текста
 @param     buffer (in) - структура хранения текстового буфера
 @return    int - 0 при успехе, -1 при ошибке
*/
int local_MakeWrapON(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, SCROLLING_T* scroll)
{
    int switcingModeOffset = (view->offsetToCurrentLine != 0)
                                ? view->offsetToCurrentLine
                                : 0;
    view->offsetCount = 1;
    long currentLength = 0;
    for(int i = 0; i < buffer->bufferLength; i++)
    {
        if(buffer->buffer[i] == '\n')
        {
            view->offsetCount++;
            currentLength = 0;
        }
        else
        {
            currentLength++;
        }
        if(currentLength == view->ClientWBuffer)
        {
            view->offsetCount++;
            currentLength = 0;
        }
    }
    view->offsetCount++;
    view->offsetsToNewLine = (long*)calloc(view->offsetCount, sizeof(long));
    view->offsetsToNewLine[0] = 0;
    currentLength = 0;
    int j = 1;
    for(int i = 0; i < buffer->bufferLength && j < view->offsetCount; i++)
    {
        if(buffer->buffer[i] == '\n')
        {
            view->offsetsToNewLine[j] = i;
            j++;
            currentLength = 0;
        }
        else
        {
            currentLength++;
        }
        if(currentLength == view->ClientWBuffer)
        {
            view->offsetsToNewLine[j] = i;
            j++;
            currentLength = 0;
        }
        if(switcingModeOffset != 0 && i == switcingModeOffset)
        {
            scroll->VScrollPos = j - 1;
        }
    }
    view->offsetsToNewLine[j] = buffer->bufferLength;
    UpdateScroll(hwnd, scroll, view, buffer);
    return 0;
}
