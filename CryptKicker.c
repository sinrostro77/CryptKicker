#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ONLINE_JUDGE
	#define REDIRECT_STDIN()
#else
	#define REDIRECT_STDIN()                                \
		if (freopen("INPUT.TXT", "r", stdin) <= (FILE *)0L)  \
		{													\
			perror("Error redirecting file to stdin");		\
		}
#endif

/* Dictionary "public" members */
typedef struct _Dictionary Dictionary;
Dictionary *Dictionary_Create();
Dictionary *Dictionary_Destroy(Dictionary *this);
char       *Dictionary_DecryptWord(Dictionary *this, const char *encryptedWord);

/*******************************/

int main(int args, char **argv)
{
	Dictionary *theDictionary;
	char  aLine[90];
	char  lineCopy[90]; /* A copy to use with strtok */
	char *currentWord;
	int   wordLength;
	char *decryptedWord;
	char *paLine;
	int   newLine;

	REDIRECT_STDIN()

	theDictionary = Dictionary_Create();

	while(fgets(aLine, sizeof(aLine), stdin) != NULL)
	{
		strcpy(lineCopy, aLine);
		paLine = aLine;
		currentWord = strtok(lineCopy, " ");
		while (currentWord)
		{
			if( (wordLength = strlen(currentWord)) > 0 )
			{
				/* Remove the end of line */
				if (currentWord[wordLength - 1] == '\n')
				{
					currentWord[wordLength - 1] = '\0';
					wordLength--;
				}
				if ( (decryptedWord = Dictionary_DecryptWord(theDictionary, currentWord)) == NULL )
				{
					memset(currentWord, '*', wordLength);
					decryptedWord = currentWord;
				}

				memcpy(paLine, decryptedWord, wordLength);
				paLine += wordLength + 1;
			}
			currentWord = strtok(NULL, " ");
		}

		printf(aLine);
	}

	theDictionary = Dictionary_Destroy(theDictionary);


	return 0;
}

/***************************************************************/
/* Start of Dictionary 	                                       */

#define MAX_WORD_SIZE 17 /* Includes the NULL character */

typedef struct
{
	char 		 text[MAX_WORD_SIZE];
	char         pattern[MAX_WORD_SIZE];
} Word;

struct _Dictionary
{
	int   length;
	Word  words[1];
};

void Dictionary_MakePattern(const char *word, char *pattern);
void Dictionary_AddWord(Dictionary *this, const char *word);

/*
	Dictionary_Create() - Creates a Dictionary and initializes it.
*/
Dictionary *Dictionary_Create()
{
	int nWords;
	int dictionarySz;
	char currentWord[MAX_WORD_SIZE];
	Dictionary *this = NULL;

	/* Get number of words in dictionary */
	scanf("%d", &nWords);

	/* Reserve memory for the Dictionary object*/
	dictionarySz = sizeof(Dictionary) + (sizeof(Word) * (nWords - 1));
	this = (Dictionary *) malloc(dictionarySz);
	memset(this, 0, dictionarySz);

	/* Read each of the words for the dictionary */
	for ( ; nWords > 0; nWords--)
	{
		scanf("%s", currentWord);
		Dictionary_AddWord(this, currentWord);
	}

	return this;
}

Dictionary *Dictionary_Destroy(Dictionary *this)
{
	if(this != NULL)
	{
		free(this);
	}
	return NULL;
}

/*
	Dictionary_DecryptWord() - It will decrypt an ecnrypted word, if
	  there's no match in the dictionary it will return a NULL pointer.
*/
char *Dictionary_DecryptWord(Dictionary *this, const char *encryptedWord)
{
	char *word = NULL;
	char  wordPattern[MAX_WORD_SIZE];
	int   i;

	Dictionary_MakePattern(encryptedWord, wordPattern);

	for(i = 0; i < this->length; i++)
	{
		if (!strcmp(wordPattern, this->words[i].pattern))
		{
			word = &this->words[i].text;
			break;
		}
	}

	return word;
}
/*
	Dictionary_AddWord() - Adds a word in a dictionary.
*/
void Dictionary_AddWord(Dictionary *this, const char *word)
{
	strcpy(&this->words[this->length].text, word);
	Dictionary_MakePattern(word, &this->words[this->length].pattern);
	this->length++;
	return;
}

/*
	Dictionary_MakePattern() - Calculates the letter repetition 
	  pattern for the given "word" and returns it in "pattern".
*/
void Dictionary_MakePattern(const char *word, char *pattern)
{
	int   cIdx,
	      tIdx;
	char  letter = 'a';

	/* Initialize pattern to NULL characters. */
	memset(pattern, 0, strlen(word) + 1);

	/* For each character in word. */
	for(cIdx = 0; word[cIdx] != '\0'; cIdx++)
	{
		/* If not already processed this pattern character */
		if(pattern[cIdx] == '\0')
		{
			pattern[cIdx] = letter;
			for(tIdx = cIdx + 1; word[tIdx] != '\0'; tIdx++)
			{
				if(word[cIdx] == word[tIdx])
				{
					pattern[tIdx] = letter;
				}
			}
			letter++;
		}
	}

	return;
}
/*  End of Dictionary                                          */
/***************************************************************/