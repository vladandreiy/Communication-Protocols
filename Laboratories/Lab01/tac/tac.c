#include <unistd.h> /* pentru open(), exit() */
#include <fcntl.h> /* O_RDWR */
#include <errno.h> /* perror() */
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_MAX 1024
#define LINES_MAX 100

void fatal(char * mesaj_eroare)
{
    perror(mesaj_eroare);
    exit(-1);
}

int main(int argc, char *argv[]) {
  int source;
  int copied;
  char buf[BUFFER_MAX];

  if(argc < 2)
    fatal("Nu s-a oferit niciun fisier");
  else if(argc > 2)
    fatal("Prea multe fisiere");

  source = open(argv[1], O_RDONLY);
  
  if (source < 0)
    fatal("Nu se poate deschide fisierul");

  lseek(source, 0, SEEK_SET);


  /*
  * Read from source and get the index of each line in an array
  * Output to stdout from the source file using the given indexes
  */

  int indexes[LINES_MAX];
  int indexesNo = 1;
  indexes[0] = 0;
  int i = 1;

  for(i=0; (copied = read(source, buf + i, 1)) ; i++) {
    if (copied < 0)
      fatal("Eroare la citire");
    if(buf[i] == '\n') {
      indexes[indexesNo++] = i+1;
    }
  }
  indexes[indexesNo++] = i+1;

  char buff[BUFFER_MAX];
  i = indexesNo - 1;
  lseek(source, indexes[i - 1], SEEK_SET);

  char newline[2];
  *newline = '\n';

  while((copied = read(source, buff, indexes[i]-indexes[i-1]-1)) && i > 0) {
    i--;
    lseek(source, indexes[i-1], SEEK_SET);
    if (copied < 0)
      fatal("Eroare la citire");
    copied = write(1, buff, copied);
    if (copied < 0)
      fatal("Eroare la scriere");
    copied = write(1, newline, 1);
    if (copied < 0)
      fatal("Eroare la scriere");
  }
  close(source);
  return 0;
}