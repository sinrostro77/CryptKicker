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

#define TRUE  1
#define FALSE 0

/* Dictionary "public" members */
typedef struct _Dictionary Dictionary;
Dictionary *Dictionary_Create();
Dictionary *Dictionary_Destroy(Dictionary *this);
void        Dictionary_ResolveSentence(Dictionary *this, char *encryptedSentence);

/*******************************/

int main(int args, char **argv)
{
	Dictionary *theDictionary;
	char  aLine[90];

	REDIRECT_STDIN()

	theDictionary = Dictionary_Create();

	while(fgets(aLine, sizeof(aLine), stdin) != NULL)
	{
		if(aLine[0] != '\0')
		{
			Dictionary_ResolveSentence(theDictionary, aLine);
			printf(aLine);
		}
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
	int          length;
} Word;

struct _Dictionary
{
	int   length;
	Word  words[1];
};

void Dictionary_MakePattern(const char *word, char *pattern);
void Dictionary_AddWord(Dictionary *this, const char *word);
int Dictionary_MatchLetters(const char *word, const char *partialWord);
char *Dictionary_DecryptWord(Dictionary *this, const char *encryptedWord, const char *resolvedLetters, int *key);

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

	scanf("%c", currentWord);

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
	Dictionary_ResolveSentence() - Resolves an encrypted sentence.
	NOTE: encryptedSentence argument is both input and output.
*/
typedef struct 
{
	int   length;
	Word  words[40];  /* Ideally I should allocate this dynamically */
} Sentence;

void Dictionary_ResolveSentence(Dictionary *this, char *encryptedSentence)
{
	char *tokenString;
	char *currentToken;
	char *decryptedWord;
	int   index;
	int   key;
	int   resolved = FALSE;
	static struct _Sentence
	{
		int length;
		struct
		{
			char *pSourceText;  /* This ain't a NULL-terminated string */
			int   length;
			char  text[MAX_WORD_SIZE];
			char  resolvedLetters[MAX_WORD_SIZE];
			int   key;
		} words[40];
	} sentence;
	
	memset(&sentence, 0, sizeof(sentence));

	tokenString = (char *)malloc(strlen(encryptedSentence) + 1);
	strcpy(tokenString, encryptedSentence);

	/* Tokenize the sentence, the output should be sorted by length
	   in descending order. */
	currentToken = strtok(tokenString, " ");
	while (currentToken != NULL)
	{
		index = 0;

		/* Calculate the index where this new token should be inserted */
		if (sentence.length > 0)
		{
			/* NOTE: Is it worth it to implement a binary search for the 40
			         elements array? */
			for ( ; index < sentence.length; index++ )
			{
				if (strlen(currentToken) >= sentence.words[index].length)
				{
					break;
				}
			}
		}

		/* If there are entries that need to be moved... do so */
		if (index < sentence.length)
		{
			memmove(&sentence.words[index + 1], &sentence.words[index], (sentence.length - index) * sizeof(sentence.words[0]) );
		}

		/* Keep a pointer to original string as well */
		sentence.words[index].pSourceText = encryptedSentence + (currentToken - tokenString);
		if( currentToken[strlen(currentToken) - 1] == '\n' )
		{
			currentToken[strlen(currentToken) - 1] = '\0';
		}
		strcpy(sentence.words[index].text, currentToken);
		sentence.words[index].length = strlen(currentToken);

		sentence.length++;

		/* Next token */
		currentToken = strtok(NULL, " ");
	}

	index = 0;
	while(TRUE)
	{
		decryptedWord = Dictionary_DecryptWord(this, sentence.words[index].text, sentence.words[index].resolvedLetters, &sentence.words[index].key);

		if (decryptedWord)
		{
			int i = 0;
			int letter, tLetter;
			strcpy(sentence.words[index].text, decryptedWord);
			for(i = index + 1; i < sentence.length; i++)
			{
				for(letter = 0; sentence.words[index].text[letter] != '\0'; letter++)
				{
					for(tLetter = 0; sentence.words[i].text[tLetter] != '\0'; tLetter++)
					{
						if(sentence.words[index].pSourceText[letter] == sentence.words[i].pSourceText[tLetter])
						{
							if(sentence.words[i].resolvedLetters[0] == '\0')
							{
								memset(sentence.words[i].resolvedLetters, ' ', sentence.words[i].length);
							}
							sentence.words[i].resolvedLetters[tLetter] = sentence.words[index].text[letter];
						}	
					}
				}
			}

			index++;
			if (index == sentence.length)
			{
				resolved = TRUE;
				break;
			}
		}
		else
		{
			/* We need to back up if needed or exit if already in the first word*/
			if (index != 0)
			{
				memcpy(sentence.words[index].text, sentence.words[index].pSourceText, sentence.words[index].length);
				sentence.words[index].resolvedLetters[0] = '\0';
				index--;
				continue;
			}
			else
			{
				break;
			}
		}
	}

	/* As last thing, if we were able to decrypt the sentence copy the result, if
	   not fill it with '*' */
	if (resolved)
	{
		for(index = 0; index < sentence.length; index++)
		{
			memcpy(sentence.words[index].pSourceText, sentence.words[index].text, sentence.words[index].length);
		}
	}
	else
	{
		for(index = 0; index < sentence.length; index++)
		{
			memset(sentence.words[index].pSourceText, '*', sentence.words[index].length);
		}
	}

	free(tokenString);
	return;
}

/*
	Dictionary_DecryptWord() - It will decrypt an ecnrypted word, if
	  there's no match in the dictionary it will return a NULL pointer.
	NOTE: As dictionary grows this function will have a poor performance,
		  for big dictionaries a hash map will probably work best.
*/
char *Dictionary_DecryptWord(Dictionary *this, const char *encryptedWord, const char *resolvedLetters, int *key)
{
	char *word = NULL;
	char  wordPattern[MAX_WORD_SIZE];
	int   i;
	int   nSolution = 0;

	Dictionary_MakePattern(encryptedWord, wordPattern);

	for(i = 0; i < this->length; i++)
	{
		/* If lenght is the same with same pattern with same letters as the
		 resolved letters and it is the key we're looking for... we got a hit! */
		if( (strlen(encryptedWord) == this->words[i].length) &&
			(!strcmp(wordPattern, this->words[i].pattern))   &&
		    (Dictionary_MatchLetters(this->words[i].text, resolvedLetters)) &&
		    (++nSolution > *key) )
		{
			word = this->words[i].text;
			(*key)++;
			break;
		}
	}

	return word;
}

/*
   Dictionary_MatchLetters() - It will return TRUEN when letters specified
     in "partialWord" are found in "word", FALSE otherwise.
*/
int Dictionary_MatchLetters(const char *word, const char *partialWord)
{
	int rc = TRUE;
	int i;

	if(partialWord[0] != '\0')
	{
		for (i = 0; word[i] != '\0'; i++)
		{
			if(partialWord[i] != ' ')
			{
				if(partialWord[i] != word[i])
				{
					rc = FALSE;
					break;
				}
			}
		}
	}

	return rc;
}

/*
	Dictionary_AddWord() - Adds a word in a dictionary.
*/
void Dictionary_AddWord(Dictionary *this, const char *word)
{
	strcpy(&this->words[this->length].text, word);
	Dictionary_MakePattern(word, &this->words[this->length].pattern);
	this->words[this->length].length = strlen(word);
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