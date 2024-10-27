#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 48  // nombre maximal de threads

// Prototype de la fonction de tri
void tri_tableau(int *tableau, long int n);

// Structure pour les données à passer au thread
typedef struct {
    int *tableau;
    long int n;
} ThreadData;

// Compteur de threads actifs
int active_threads = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* tri_tableau_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
	
    // Tri de la partie donnée
    tri_tableau(data->tableau, data->n);
    
    return NULL;
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
    for (int i = 0; i < n; i++) {
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

    // Fermer le fichier de sortie
    fclose(fichier_sortie);

    // Libérer la mémoire allouée
    free(tableau);

    // Afficher le temps d'exécution du tri
    printf("Temps d'exécution du tri : %.6f secondes\n", temps_execution);

    return 0;
}

// Fonction de tri par fusion
void tri_tableau(int *tableau, long int n) {
    if (n <= 1) {
        return;
    }

    // Diviser le tableau en deux parties : les tableaux gauche et droite
    long int milieu = n / 2;
    int *gauche = (int *)malloc(milieu * sizeof(int));
    int *droite = (int *)malloc((n - milieu) * sizeof(int));
    for (int i = 0; i < milieu; i++) {
        gauche[i] = tableau[i];
    }
    for (int i = milieu; i < n; i++) {
        droite[i - milieu] = tableau[i];
    }

    // Créer un thread pour trier la partie gauche si le nombre de threads actifs le permet
    pthread_t thread_gauche;
    ThreadData data_gauche = {gauche, milieu};
    
    int thread_cree = 0;
    pthread_mutex_lock(&mutex);
    if (active_threads < MAX_THREADS) {
        active_threads++;
        if (pthread_create(&thread_gauche, NULL, tri_tableau_thread, (void *)&data_gauche) != 0) {
            perror("Erreur lors de la création du thread");
            active_threads--;
            pthread_mutex_unlock(&mutex);
            tri_tableau(gauche, milieu);  // Exécuter le tri de gauche de manière séquentielle si le thread échoue
        } else {
            pthread_mutex_unlock(&mutex);
            thread_cree = 1;       
        }
    } else {
        pthread_mutex_unlock(&mutex);
        tri_tableau(gauche, milieu);  // Exécuter le tri de gauche de manière séquentielle si le nombre de threads actifs est maximum
    }

    // Trier la partie droite de manière séquentielle
    tri_tableau(droite, n - milieu);
    
    if (thread_cree){
        pthread_join(thread_gauche, NULL);  // Attendre la fin du thread
        pthread_mutex_lock(&mutex);
        active_threads--;         // Fin du thread, décrémenter le compteur
        pthread_mutex_unlock(&mutex);
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
    // On termine de remplir le tableau avec les éléments restants de gauche
    while (i < milieu) {
        tableau[k] = gauche[i];
        i++;
        k++;
    }
    // On termine de remplir le tableau avec les éléments restants de droite
    while (j < n - milieu) {
        tableau[k] = droite[j];
        j++;
        k++;
    }

    // Libérer la mémoire allouée pour les deux parties
    free(gauche);
    free(droite);
}

