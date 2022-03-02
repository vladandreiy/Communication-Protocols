#include <unistd.h> /* pentru open(), exit() */
#include <fcntl.h> /* O_RDWR */
#include <errno.h> /* perror() */
#include <stdio.h>
#include <stdlib.h>

void fatal(char * mesaj_eroare)
{
    perror(mesaj_eroare);
    exit(-1);
}

int main(int argc, char *argv[]) {
  int source;
  int copied;
  char buf[1024];

  if(argc < 2)
    fatal("Nu s-a oferit niciun fisier");
  else if(argc > 2)
    fatal("Prea multe fisiere");

  source = open(argv[1], O_RDONLY);
  
  if (source < 0)
    fatal("Nu se poate deschide fisierul");

  lseek(source, 0, SEEK_SET);

  while ((copied = read(source, buf, sizeof(buf)))) {
    if (copied < 0)
      fatal("Eroare la citire");
    copied = write(1, buf, copied);
    if (copied < 0)
      fatal("Eroare la scriere");
    }
  close(source);
  return 0;
}