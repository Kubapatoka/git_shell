#include "shell.h"

void strapp(char **dstp, const char *src) {
  assert(dstp != NULL);

  if (*dstp == NULL) {
    *dstp = strdup(src);
  } else {
    size_t s = strlen(*dstp) + strlen(src) + 1;
    *dstp = realloc(*dstp, s);
    strcat(*dstp, src);
  }
}

token_t *tokenize(char *s, int *tokc_p) {
  int capacity = 10;
  int ntoks = 0;

  token_t *tokvec = malloc(sizeof(token_t) * (capacity + 1));

  while (*s != 0) {
    /* Consume whitespace characters. */
    if (isspace(*s)) {
      *s++ = 0;
      continue;
    }

    /* Make sure there's enough space to add new token. */
    if (ntoks == capacity) {
      capacity *= 2;
      tokvec = realloc(tokvec, sizeof(token_t) * (capacity + 1));
    }

    size_t l = strcspn(s, " |&<>;!"); //szuka znaku z " " w s
    if (l > 0) {
      tokvec[ntoks++] = s;
      s += l;
      continue;
    }

    token_t tok;

    if (s[0] == '|') {
      if (s[1] == '|') {
        *s++ = 0;
        tok = T_OR;
      } else {
        tok = T_PIPE;
      }
    } else if (s[0] == '&') {
      if (s[1] == '&') {
        *s++ = 0;
        tok = T_AND;
      } else {
        tok = T_BGJOB;
      }
    } else if (s[0] == '<') {
      tok = T_INPUT;
    } else if (s[0] == '>') {
      tok = T_OUTPUT;
    } else if (s[0] == ';') {
      tok = T_COLON;
    } else if (s[0] == '!') {
      tok = T_BANG;
    } else {
      continue;
    }

    *s++ = 0;
    tokvec[ntoks++] = tok;
  }

  tokvec[ntoks] = NULL;
  *tokc_p = ntoks;
  
  int i = 0;
  while(tokvec[i] != NULL)
  {
  	char* a = tokvec[i];
  	
  	if(tokvec[i] == T_NULL) printf("T_NULL");
  	else if(tokvec[i] == T_OR) printf("T_OR");
  	else if(tokvec[i] == T_PIPE) printf("T_PIPE");
  	else if(tokvec[i] == T_AND) printf("T_AND");
  	else if(tokvec[i] == T_BGJOB) printf("T_BGJOB");
  	else if(tokvec[i] == T_INPUT) printf("T_INPUT");
  	else if(tokvec[i] == T_OUTPUT) printf("T_OUTPUT");
  	else if(tokvec[i] == T_COLON) printf("T_COLON");
  	else if(tokvec[i] == T_APPEND) printf("T_APPEND");
  	else if(tokvec[i] == T_BANG) printf("T_BANG");
  	else while(*a != '\0'){ printf("%c", *a); a++;} 
	
	printf("\n"); 
	
	i++;	
  }
  
  return tokvec;
}
