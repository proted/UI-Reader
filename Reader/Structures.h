#ifndef STRUCTURES_S_INCLUDED
#define STRUCTURES_S_INCLUDED

#include "Windows.h"

typedef enum    MODE_T
{
    WRAP_OFF,   //����� "��� �������"
    WRAP_ON     //����� "� ��������"
} MODE_T;
typedef struct  BUFFER_T
{
    char*   buffer;         //������, � ������� �������� ���������� �����
    char*   filename;       //������������ �����
    long    bufferLength;   //���������� �������� � ���������� �����
    long    strCount;       //���������� ����� � �����
} BUFFER_T;
typedef struct  VIEW_T
{
    MODE_T  mode;                   //������� �����
    long*   offsetsToNewLine;       //������ �������� � ������, ���������� ������ ����� ������
    long    offsetCount;            //���������� �������� (����������, ����� ��� �����������)
    int     ClientWBuffer;          //���������� ��������, ������������ � ���� ������ � ����
    int     ClientHBuffer;          //���������� �����, ������������ � ����
    long    MaxStrLength;           //������������ ����� ������
    long    offsetToCurrentLine;    //������ �������� ���������� ������, � �������� ���������� ������� ������
} VIEW_T;
typedef struct  WNDPARAMS_T
{
    int     WChar;      //������ ������ �������
    int     HChar;      //������ ������ �������
    int     WClient;    //������ ����
    int     HClient;    //������ ����
} WNDPARAMS_T;
typedef struct  SCROLLING_T
{
    long            VScrollPos;         //������� ������� �������� ������������ ������ ���������
    long            HScrollPos;         //������� ������� �������� �������������� ������ ���������
    long            VScrollMax;         //������������ ������� �������� ������������ ������ ���������
    long            HScrollMax;         //������������ ������� �������� �������������� ������ ���������
    long double     coeffScrolling;     //����������� "������" �������� (��� �������, ����� ������������ ������� �������� ��������� ���������� ��� ������ ���������)
} SCROLLING_T;

/*
 ������� ������ ����� �� �����
 @param buffer (in/out) - ��������� �������� ������
 @out   int - 0 ��� ������, -1 ��� ������
*/
int LoadBuffer(BUFFER_T* buffer);

/*
 ������� �������������� ��������� VIEW_T ��� ����������� ������ � ������� �������/���_�������
 @param     hwnd (in)
 @param     view (in/out) - ��������� ������������� ������, ������������� �� ������
 @param     buffer (in) - ��������� �������� ���������� ������
 @param     wndparams (in) - ��������� �������� ���������� ���� � ������
 @param     scroll (in/out) - ��������� ������������� ����� ���������
 @return    int - 0 ��� ������, -1 ��� ������
*/
int MakeView(HWND hwnd, VIEW_T* view, BUFFER_T* buffer, WNDPARAMS_T* wndparams, SCROLLING_T* scroll);

/*
 ������� ������������ ���������� ���������� ����� ���������
 @param     scroll (in/out) - ��������� ������������� ����� ���������
 @param     view (in) - ��������� ������������� ������, ������������� �� ������
 @param     buffer (in) - ��������� �������� ���������� ������
 @return    int - 0 ��� ������, -1 ��� ������
*/
int UpdateScroll(HWND hwnd, SCROLLING_T* scroll, VIEW_T* view, BUFFER_T* buffer);

/*
 ������� ������� ������ � ��������� ������������� VIEW_T
 @param     view (in/out) - ��������� ������������� ������, ������������� �� ������
 @return    none
*/
void ClearView(VIEW_T* view);

/*
 ������� ������� ������ � ��������� �������� ���������� ������ BUFFER_T
 @param     buffer (in/out) - ��������� �������� ���������� ������
 @return    none
*/
void ClearBuffer(BUFFER_T* buffer);

/*
 ������� �������� � ���������� ��������� ��������� ������������� ����� ��������� SCROLLING_T
 @param     scroll (in/out) - ��������� ������������� ����� ��������� SCROLLING_T
 @return    none
*/
void ClearScroll(HWND hwnd, SCROLLING_T* scroll);

#endif //STRUCTURES_H_INCLUDED
