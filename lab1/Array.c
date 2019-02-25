#include "Array.h"
#include <stdlib.h>
#include <stdio.h>

//---------------------------STATIC-------------------------

char sArray[10000][10000];

int sSize()
{
	return sizeof(sArray) / sizeof(sArray[0]);
}

int sSizeOfBlock()
{
	return sizeof(sArray[0]);
}

int sFindEmpty()
{
	int i = 0;
	while(sArray[i][0] != '\0' && i < sSize())
	{
		i++;
	}
	if(i == sSize())
		return -1;
	return i;
}

void sInsertBlock(char* chars, int sizeOfChars)
{
	int i = sFindEmpty();
	if(i < 0 || sizeOfChars + 1 > sSizeOfBlock())
		return;
	
	int j = 0;
	while(j < sizeOfChars)
	{
		sArray[i][j] = chars[j];
		j++;
	}
	sArray[i][j] = '\0';
}

void sDeleteBlock(int pos)
{
	if(pos < 0 || pos >= sSize())
		return;
	
	int i = 0;
	while(sArray[pos][i] != '\0')
	{
		sArray[pos][i] = '\0';
		i++;
	}
}

int sSumOfBlock(int pos)
{
	if(pos < 0 || pos >= sSize() || sArray[pos][0] == '\0')
		return -1;
		
	int sum = 0;
	int i = 0;
	while(sArray[pos][i] != '\0')
	{
		int ascii = sArray[pos][i];
		sum += ascii;
		i++;
	}
	return sum;
}

int sFindSimilarSum(int pos)
{
	if(pos < 0 || pos >= sSize() || sArray[pos] == '\0')
		return -1;
	
	int index = -1;
	int difference = -1;
	int i = 0;
	while(i < sSize() && difference < 0)
	{
		if(i != pos && sArray[i][0] != '\0')
		{
			difference = abs(sSumOfBlock(i) - sSumOfBlock(pos));
			index = i;
		}
			
		i++;
	}
	while(i < sSize())
	{
		if(i != pos && sArray[i][0] != '\0')
		{
			int newDifference = abs(sSumOfBlock(i) - sSumOfBlock(pos));
			if(newDifference < difference)
			{
				difference = newDifference;
				index = i;
			}
		}
		i++;
	}
	if(difference < 0)
		return -1;
	return index;
}

//----------------------DYNAMIC-----------------------------

int dFindEmpty(char **array, int size)
{
	int i = 0;
	while(array[i] != NULL && i < size)
	{
		i++;
	}
	if(i == size)
		return -1;
	return i;
}

void dInsertBlock(char **array, int size, char* chars, int sizeOfChars)
{
	int i = dFindEmpty(array, size);
	if(i < 0)
		return;
	
	array[i] = malloc(sizeOfChars + 1);
	int j = 0;
	while(j < sizeOfChars)
	{
		array[i][j] = chars[j];
		j++;
	}
	array[i][j] = '\0';
}

void dDeleteBlock(char **array, int size, int pos)
{
	if(pos < 0 || pos >= size)
		return;
	free(array[pos]);
	array[pos] = NULL;
}

char **dMakeArray(int size)
{
	char **array = malloc(sizeof(char **) * size);
	
	int i = 0;
	while(i < size)
	{
		array[i] = NULL;
		i++;
	}
	
	return array;
}

char **dDeleteArray(char **array, int size)
{
	int i = 0;
	while(i < size)
	{
		dDeleteBlock(array, size, i);
		i++;
	}
	free(array);
	return NULL;
}

int dSumOfBlock(char **array, int size, int pos)
{
	if(pos < 0 || pos >= size || array[pos] == NULL)
		return -1;
		
	int sum = 0;
	int i = 0;
	while(array[pos][i] != '\0')
	{
		int ascii = array[pos][i];
		sum += ascii;
		i++;
	}
	return sum;
}

int dFindSimilarSum(char **array, int size, int pos)
{
	if(pos < 0 || pos >= size || array[pos] == NULL)
		return -1;
	
	int index = -1;
	int difference = -1;
	int i = 0;
	while(i < size && difference < 0)
	{
		if(i != pos && array[i] != NULL)
		{
			difference = abs(dSumOfBlock(array, size, i) - dSumOfBlock(array, size, pos));
			index = i;
		}
			
		i++;
	}
	while(i < size)
	{
		if(i != pos && array[i] != NULL)
		{
			int newDifference = abs(dSumOfBlock(array, size, i) - dSumOfBlock(array, size, pos));
			if(newDifference < difference)
			{
				difference = newDifference;
				index = i;
			}
		}
		i++;
	}
	if(difference < 0)
		return -1;
	return index;
}

