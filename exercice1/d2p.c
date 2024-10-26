#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Prototypes des fonctions
void tri_tableau(int *tableau, long int n);
void fusion(int *T, long int p1, long int r1, long int p2, long int r2, int *A, long int p3);
void p_tri_fusion(int *A, long int p, long int r, int *B, long int s);

// Fonction principale pour trier le tableau
void tri_tableau(int *tableau, long int n) {
    int *T = (int *)malloc(n * sizeof(int));  // Tableau temporaire
    if (T == NULL) {
        perror("Erreur lors de l'allocation mémoire pour le tableau temporaire");
        return;
    }

    p_tri_fusion(tableau, 0, n - 1, T, 0);
    free(T);  // Libérer le tableau temporaire
}

// Fonction de tri fusion parallèle
void p_tri_fusion(int *A, long int p, long int r, int *B, long int s) {
    if (p == r) {
        B[s] = A[p];
    } else {
        long int q = (p + r) / 2;
        long int q_prime = q - p + 1;

        // Lancer le tri fusion en parallèle pour la première moitié
        pthread_t thread;
        pthread_create(&thread, NULL, (void *(*)(void *))p_tri_fusion, (void *)(&(struct { int *A; long int p; long int q; int *B; long int s; }){A, p, q, B, 0}));

        // Tri fusion de la seconde moitié
        p_tri_fusion(A, q + 1, r, B, q_prime + s);

        pthread_join(thread, NULL);  // Attendre la fin du thread

        // Fusionner les deux parties
        fusion(B, s, q_prime - 1, q_prime + s, r - p, A, s);
    }
}

// Fonction de fusion
void fusion(int *T, long int p1, long int r1, long int p2, long int r2, int *A, long int p3) {
    long int n1 = r1 - p1 + 1;
    long int n2 = r2 - p2 + 1;

    if (n1 < n2) {
        // Échanger les paramètres si n1 < n2
        long int temp = n1;
        n1 = n2;
        n2 = temp;

        temp = p1;
        p1 = p2;
        p2 = temp;

        temp = r1;
        r1 = r2;
        r2 = temp;
    }

    if (n1 > 0) {
        long int q1 = (p1 + r1) / 2;
        long int q2 = p2;

        while (q2 <= r2 && T[q1] > T[q2]) {
            q2++;
        }

        long int q3 = p3 + (q1 - p1) + (q2 - p2);
        A[q3] = T[q1];

        pthread_t thread;
        pthread_create(&thread, NULL, (void *(*)(void *))fusion, (void *)(&(struct { int *T; long int p1; long int r1; long int p2; long int r2; int *A; long int p3; }){T, p1, q1 - 1, p2, q2 - 1, A, p3}));

        fusion(T, q1 + 1, r1, q2, r2, A, q3 + 1);
        pthread_join(thread, NULL);
    }
}

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
    fscanf(fichier_entree, "%ld", &n); // Lire le nombre d'entiers

    // Allouer de la mémoire pour le tableau
    int *tableau = (int *)malloc(n * sizeof(int));
    if (tableau == NULL) {
        perror("Erreur lors de l'allocation mémoire");
        fclose(fichier_entree);
        return 1;
    }

    // Lire les n entiers dans le tableau
    for (long int i = 0; i < n; i++) {
        fscanf(fichier_entree, "%d", &tableau[i]);
    }
    fclose(fichier_entree);

    // Calcul du temps de début
    clock_t debut = clock();

    // Appeler la fonction de tri
    tri_tableau(tableau, n);

    // Calcul du temps de fin
    clock_t fin = clock();
    double temps_execution = (double)(fin - debut) / CLOCKS_PER_SEC;

    // Ouvrir le fichier de sortie
    FILE *fichier_sortie = fopen(argv[2], "w");
    if (fichier_sortie == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(tableau);
        return 1;
    }

    // Si n <= 1000, écrire tout le tableau trié
    if (n <= 1000) {
        for (long int i = 0; i < n; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
    } else {
        // Si n > 1000, écrire les 100 premiers, une séparation, et les 100 derniers
        for (int i = 0; i < 100; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
        fprintf(fichier_sortie, "--------------------\n");
        for (long int i = n - 100; i < n; i++) {
            fprintf(fichier_sortie, "%d\n", tableau[i]);
        }
    }

    // Fermer le fichier de sortie
    fclose(fichier_sortie);

    // Libérer la mémoire allouée
    free(tableau);

    // Afficher le temps d'exécution du tri
    printf("Temps d'exécution du tri : %.6f secondes\n", temps_execution);

    return 0;
}

