#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int wish_komennonSelvitys(char **a);
int wish_kaynnistys(char **a);
char *wish_rivinluku(void);
char **wish_kaskyjen_halkaisu(char *rivi);
void build_in_exit();
void build_in_path();
void build_in_cd(char **a);

int main(int argc, char *argv[]) {
	char error_message[] = "Cannot open file.\n";
	char *rivi;
	int tila;
	char **syotteet;
	FILE *tiedosto;
	char muisti[256];
	if (argc > 1) { // Jos komentorivi parametrinä annetaan jotain eli tiedoston nimi
		tiedosto = fopen(argv[1], "r");
		if (tiedosto == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));	
			exit(1);
		}
		while (fgets(muisti, 20, tiedosto) != NULL) {
			printf("%s", muisti);
			int len = strlen(muisti);
			if (len > 0 && muisti[len-1] == '\n') {	// poistetaan rivinvaihto 
				muisti[len-1] = 0;
			}
			syotteet = wish_kaskyjen_halkaisu(muisti);
			tila = wish_komennonSelvitys(syotteet);

		}
		fclose(tiedosto);
	} else if (argc == 1) {
		do {
			printf("wish> ");
			rivi = wish_rivinluku();
			syotteet = wish_kaskyjen_halkaisu(rivi);
			if (strcmp(rivi,"\n") == 0) { // Jos annetaan vain rivinvaihto mennään loopin alkuun
				continue;
			} else {
				tila = wish_komennonSelvitys(syotteet);
			}
			free(rivi);
		} while (tila);
	}
	return 0;
}

int wish_komennonSelvitys(char **a) {
	char *b_i[] = {"exit","path","cd"}; // build in lista, joita vertaillaan annettuun käskyyn
	if (a[0] == NULL) {
		return 1;
	}
	if (strcmp(a[0],b_i[0]) == 0) {
		build_in_exit();
		return 1;
	} else if (strcmp(a[0],b_i[1]) == 0) {
		build_in_path();
		return 1;
	} else if (strcmp(a[0],b_i[2]) == 0) {
		build_in_cd(a);
		return 1;
	}
	return wish_kaynnistys(a); // jos kasky ei ole build in
}

int wish_kaynnistys(char **args) {
	char error_message[] = "No such command.\n";
	char error_message2[] = "Forking has errored.\n";
	char **a = malloc(64 * sizeof(char));
	pid_t pid;
	int status;
	char polku[20] = "/bin/";
	strcat(polku, args[0]); //lisätään polkuun annettu komentorivikäsky (esim. polku = /bin/ls)

	int i = 0;
	while (args[i] != NULL) { // selvitetään löytyykö redirectaus merkki
		if (strcmp(args[i],">") == 0) {
			args[i] = NULL;
			break;
		}
		i++;
	}
	pid = fork(); // Lähde: [https://brennan.io/2015/01/16/write-a-shell-in-c/]
	if (pid == 0) {
		int tiedosto = open(args[i+1], O_WRONLY|O_CREAT|O_TRUNC, 0600);
		dup2(tiedosto,STDOUT_FILENO);
		// Lapsiprosessi
		if (execv(polku, args) == -1) {
			write(STDERR_FILENO, error_message, strlen(error_message));	
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Katsotaan ettei erroria forkin kanssa
		write(STDERR_FILENO, error_message2, strlen(error_message));
	} else {
		// Vanhempi prosessi
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

char *wish_rivinluku(void) {
	char error_message[] = "Cannot read line.\n";
	size_t kaskySize = 1024;
	char *kasky = malloc(kaskySize * sizeof(char));
	
	if(kasky == NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	getline(&kasky, &kaskySize, stdin);
	strtok(kasky, "\n");
	return kasky;
}

char **wish_kaskyjen_halkaisu(char *rivi) { //luodaan lista, jossa kaikki syötteen "sanat" erikseen
	size_t palaSize = 64;
	char **palat = malloc(palaSize * sizeof(char)); 
	char vali[2] = " ";
	char *pala, **v_pala;
	int i = 0;
	pala = strtok(rivi,vali);
	while (pala != NULL) {
		palat[i] = pala;
		i++;
		if (sizeof(palat) <= palaSize) {
			v_pala = palat;
			palaSize += 64;
			palat = realloc(palat,palaSize * sizeof(char)); //allokoidaan tarpeen vaatiessa lisää tilaa
		}
		pala = strtok(NULL,vali);
	}
	return palat;
}

void build_in_exit(){
	exit(0);
}

void build_in_path() {
	char error_message[] = "path: Cannot find path.\n";
	char polku[256];
	if (getcwd(polku,sizeof(polku)) == NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
	} else {
		printf("%s\n", polku);
	}
}

void build_in_cd(char **a) {
	char error_message[] = "cd: No such file or directory.\n";
	if (a[1] == NULL) {
		printf("cd: cd needs atleast one argument.\n");
	}
	if (chdir(a[1]) != 0) {
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}

