#ifndef STRUCTURES_S_INCLUDED
#define STRUCTURES_S_INCLUDED

#include "Windows.h"

typedef enum    MODE_T
{
    WRAP_OFF,   //режим "без верстки"
    WRAP_ON     //режим "с версткой"
} MODE_T;
typedef struct  BUFFER_T
{
    char*   buffer;         //массив, в которой хранится содержимое файла
    char*   filename;       //наименование файла
    long    bufferLength;   //количество символов в содержимом файла
    long    strCount;       //количество строк в файле
} BUFFER_T;
typedef struct  VIEW_T
{
    MODE_T  mode;                   //текущий режим
    long*   offsetsToNewLine;       //массив отступов в буфере, означающих начало новой строки
    long    offsetCount;            //количество отступов (фактически, строк при отображении)
    int     ClientWBuffer;          //количество символов, помещающихся в одну строку в окне
    int     ClientHBuffer;          //количество строк, помещающихся в окне
    long    MaxStrLength;           //Максимальная длина строки
    long    offsetToCurrentLine;    //Индекс элемента текстового буфера, с которого начинается верхняя строка
} VIEW_T;
typedef struct  WNDPARAMS_T
{
    int     WChar;      //ширина одного символа
    int     HChar;      //высота одного символа
    int     WClient;    //ширина окна
    int     HClient;    //высота окна
} WNDPARAMS_T;
typedef struct  SCROLLING_T
{
    long            VScrollPos;         //текущая позиция ползунка вертикальной полосы прокрутки
    long            HScrollPos;         //текущая позиция ползунка горизонтальной полосы прокрутки
    long            VScrollMax;         //максимальная позиция ползунка вертикальной полосы прокрутки
    long            HScrollMax;         //максимальная позиция ползунка горизонтальной полосы прокрутки
    long double     coeffScrolling;     //коэффициент "сжатия" ползунка (для случаев, когда максимальная позиция ползунка превышает допустимую для полосы прокрутки)
} SCROLLING_T;

/*
 Функция читает текст из файла
 @param buffer (in/out) - структура хранения буфера
 @out   int - 0 при успехе, -1 при ошибке
*/
int LoadBuffer(BUFFER_T* buffer);

/*
 Функция инициализирует структуру VIEW_T для отображения текста в режимах верстка/без_верстки
 @param     hwnd (in)
 @param     view (in/out) - структура представления текста, отображаемого на экране
 @param     buffer (in) - структура хранения текстового буфера
 @param     wndparams (in) - структура хранения параметров окна и шрифта
 @param     scroll (in/out) - структура представления полос прокрутки
 @return    int - 0 при успехе, -1 при ошибке
*/
int MakeView(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, WNDPARAMS_T* wndparams, SCROLLING_T* scroll);

/*
 Функция осуществляет обновление параметров полос прокрутки
 @param     scroll (in/out) - структура представления полос прокрутки
 @param     view (in) - структура представления текста, отображаемого на экране
 @param     buffer (in) - структура хранения текстового буфера
 @return    int - 0 при успехе, -1 при ошибке
*/
int UpdateScroll(HWND hwnd, SCROLLING_T* scroll, VIEW_T* view, BUFFER_T* buffer);

/*
 Функция очищает память в структуре представления VIEW_T
 @param     view (in/out) - структура представления текста, отображаемого на экране
 @return    none
*/
void ClearView(VIEW_T* view);

/*
 Функция очищает память в структуре хранения текстового буфера BUFFER_T
 @param     buffer (in/out) - структура хранения текстового буфера
 @return    none
*/
void ClearBuffer(BUFFER_T* buffer);

/*
 Функция приводит к дефолтному состоянию структуру представления полос прокрутки SCROLLING_T
 @param     scroll (in/out) - структура представления полос прокрутки SCROLLING_T
 @return    none
*/
void ClearScroll(HWND hwnd, SCROLLING_T* scroll);

#endif //STRUCTURES_H_INCLUDED
