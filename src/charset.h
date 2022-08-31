#ifndef CHARSET
#define CHARSET

#define FONT_SIZE 36
#define N_CHAR 33

char characters[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'D', 'G', 'L', 'P', 
    'c', 'e', 'g', 'h', 'i', 'm', 'n', 'o', 'p', 'r', 's', 't', 'u', 
    ':', ',', '(', ')', '.', '-' 
};

wchar_t w_characters[] = {
    L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', 
    L'D', L'G', L'L', L'P', 
    L'c', L'e', L'g', L'h', L'i', L'm', L'n', L'o', L'p', L'r', L's', L't', L'u', 
    L':', L',', L'(', L')', L'.', L'-' 
};

#endif