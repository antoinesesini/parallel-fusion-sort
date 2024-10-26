import random


def generer_fichier_test(n, m, nom_fichier):
    with open(nom_fichier, 'w') as f:
        # Écrire la valeur de n en première ligne
        f.write(f"{n}\n")

        # Générer n entiers aléatoires et les écrire dans le fichier ensuite
        for _ in range(n):
            entier_aleatoire = random.randint(1, m)  # Par exemple, entre 1 et 100
            f.write(f"{entier_aleatoire}\n")


if __name__ == "__main__":
    # Modifie 'n' pour tester avec différents nombres d'entiers
    n = 1000  # Par exemple, 10
    max = 100
    nom_fichier = "fichier_test"

    generer_fichier_test(n, max, nom_fichier)
    print(f"Fichier {nom_fichier} généré avec {n} entiers.")
