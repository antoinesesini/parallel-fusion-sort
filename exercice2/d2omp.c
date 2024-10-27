#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_THREADS 48

// Compteur global pour le nombre de threads actifs
int threads_actifs = 0;

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

    long int n;
    fscanf(fichier_entree, "%ld", &n);

    // Allouer de la mémoire pour le tableau
    int *tableau = (int *)malloc(n * sizeof(int));
    if (tableau == NULL) {
        perror("Erreur lors de l'allocation mémoire");
        fclose(fichier_entree);
        return 1;
    }

    // Lire les n entiers dans le tableau
    for (int i = 0; i < n; i++) {
        fscanf(fichier_entree, "%d", &tableau[i]);
    }
    fclose(fichier_entree);

    // Calcul du temps de début
    double debut = omp_get_wtime();

    tri_tableau(tableau, n);

    // Calcul du temps de fin
    double fin = omp_get_wtime();
    double temps_execution = fin - debut; // Calculer le temps d'exécution

    // Ouvrir le fichier de sortie
    FILE *fichier_sortie = fopen(argv[2], "w");
    if (fichier_sortie == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(tableau);
        return 1;
    }

    // Si n <= 1000, écrire tout le tableau trié
    if (n <= 1000) {
        for (int i = 0; i < n; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
    } else {
        // Si n > 1000, écrire les 100 premiers, une séparation, et les 100 derniers
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

    // Zone critique pour vérifier et ajuster le nombre de threads
    #pragma omp critical
    {
        if (threads_actifs < MAX_THREADS) {
            threads_actifs++;
            executer_en_parallele = 1;
        }
    }


    if (executer_en_parallele) {
        // Lancer tri parallèle pour la partie gauche si des threads sont disponibles
        #pragma omp task
        {
            tri_tableau(gauche, milieu);
            #pragma omp critical
            {
                threads_actifs--;
            }
        }

        // Exécuter tri_tableau(droite) dans le programme principal en même temps
        tri_tableau(droite, n - milieu);
        // S'assurer que tri_tableau(gauche) a fini
        #pragma omp taskwait
    } else {
        // Si pas de threads disponibles, tri séquentiel pour les deux parties
        tri_tableau(gauche, milieu);
        tri_tableau(droite, n - milieu);
    }

    // Fusionner les deux parties triées
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

