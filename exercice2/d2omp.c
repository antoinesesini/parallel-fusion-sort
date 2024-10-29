#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <semaphore.h>

#define MAX_THREADS 48  // nombre maximal de threads

sem_t semaphore;  // Déclaration du sémaphore

void tri_tableau(int *tableau, long int n);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Utilisation : %s <fichier_test> <resultat>\n", argv[0]);
        return 1;
    }
    FILE *fichier_entree = fopen(argv[1], "r");
    if (fichier_entree == NULL) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        return 1;
    }

    long int n = 10000;
    //fscanf(fichier_entree, "%ld", &n);

    int *tableau = (int *)malloc(n * sizeof(int));
    if (tableau == NULL) {
        perror("Erreur lors de l'allocation mémoire");
        fclose(fichier_entree);
        return 1;
    }

    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        //fscanf(fichier_entree, "%d", &tableau[i]);
        tableau[i] = rand() % 10000 + 1;
    }


    fclose(fichier_entree);

    double debut = omp_get_wtime();

    // Initialiser le sémaphore avec une valeur de MAX_THREADS
    sem_init(&semaphore, 0, MAX_THREADS);

    tri_tableau(tableau, n);

    double fin = omp_get_wtime();
    double temps_execution = fin - debut;

    FILE *fichier_sortie = fopen(argv[2], "w");
    if (fichier_sortie == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(tableau);
        return 1;
    }

    if (n <= 1000) {
        for (int i = 0; i < n; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
    } else {
        for (int i = 0; i < 100; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
        fprintf(fichier_sortie, "--------------------\n");
        for (int i = n - 100; i < n; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
    }

    fclose(fichier_sortie);
    free(tableau);
    printf("Temps d'exécution du tri : %.6f secondes\n", temps_execution);

    // Détruire le sémaphore
    sem_destroy(&semaphore);

    return 0;
}

void tri_tableau(int *tableau, long int n) {
    if (n <= 1) {
        return;
    }

    long int milieu = n / 2;
    int *gauche = (int *)malloc(milieu * sizeof(int));
    int *droite = (int *)malloc((n - milieu) * sizeof(int));

    for (int i = 0; i < milieu; i++) {
        gauche[i] = tableau[i];
    }
    for (int i = milieu; i < n; i++) {
        droite[i - milieu] = tableau[i];
    }

    int executer_en_parallele = 0;

    // Attendre un slot disponible dans le sémaphore pour exécuter en parallèle
    if (sem_trywait(&semaphore) == 0) {  // Si sémaphore décrémenté avec succès, on peut lancer en parallèle
        executer_en_parallele = 1;
    }

    if (executer_en_parallele) {
        #pragma omp task
        {
            tri_tableau(gauche, milieu);
            sem_post(&semaphore);  // Libérer le sémaphore lorsque le thread termine
        }

        tri_tableau(droite, n - milieu);
        #pragma omp taskwait
    } else {
        tri_tableau(gauche, milieu);
        tri_tableau(droite, n - milieu);
    }

    int i = 0, j = 0, k = 0;
    while (i < milieu && j < n - milieu) {
        if (gauche[i] < droite[j]) {
            tableau[k] = gauche[i];
            i++;
        } else {
            tableau[k] = droite[j];
            j++;
        }
        k++;
    }
    while (i < milieu) {
        tableau[k] = gauche[i];
        i++;
        k++;
    }
    while (j < n - milieu) {
        tableau[k] = droite[j];
        j++;
        k++;
    }

    free(gauche);
    free(droite);
}
