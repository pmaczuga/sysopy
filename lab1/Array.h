#ifndef Array
#define Array

//----------------------STATIC-----------------------

void sInsertBlock(char* chars, int sizeOfChars);

void sDeleteBlock(int pos);

int sFindSimilarSum(int pos);

//--------------------------DYNAMIC----------------------

char **dMakeArray(int size);

char **dDeleteArray(char **array, int size);

void dInsertBlock(char **array, int size, char* chars, int sizeOfChars);

void dDeleteBlock(char **array, int size, int pos);

int dFindSimilarSum(char **array, int size, int pos);

#endif
