#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 10
#define MAX_BLKS 100
#define MAX_CONTENT 50
//structure des enregistrements
typedef struct {
    int id;
    char contenu[MAX_CONTENT];
} Enregistrement;
//structure du fichier metadonnes
typedef struct {
    char nom[50];
    int tailleEnBlocs;
    int adressePremierBloc;
    char modeOrganisationGlobale[20];
    char modeInsertion[20];
} FichierMetadata;
// structure de bloc

typedef struct {
    int suivant;
    int estOccupe;
    Enregistrement contenu;
    char nomFichier[50];
    int estLogiquementSupprime;  // Ajout de ce champ
} Bloc;


FichierMetadata fichiersMetadata[MAX_FILES];
Bloc blocs[MAX_BLKS];
int nbFichiers = 0;
// procedure pour inisialiser les blocs
void initialiserBlocs() {
    for (int i = 0; i < MAX_BLKS; i++) {
        blocs[i].suivant = -1;
        blocs[i].estOccupe = 0;
        blocs[i].contenu.id = -1;
        strcpy(blocs[i].nomFichier, "");
    }
}
// procedure affichage des blocs
void afficherBlocs() {
    printf("\nEtat des blocs :\n");
    printf("+------+--------+---------+------+----------------+---------------+\n");
    printf("| Bloc | Occupe | Suivant | ID   | Contenu        | Fichier       |\n");
    printf("+------+--------+---------+------+----------------+---------------+\n");
    for (int i = 0; i < MAX_BLKS; i++) {
        printf("| %-4d | %-6d | %-7d | %-4d | %-14s | %-13s |\n",
               i, blocs[i].estOccupe, blocs[i].suivant,
               blocs[i].contenu.id, blocs[i].contenu.contenu, blocs[i].nomFichier);
    }
    printf("+------+--------+---------+------+----------------+---------------+\n");
}
// fonction pour afficher les meta donnes
void afficherMetadonnees() {
    printf("\n--- M%ctadonn%ces des fichiers ---\n",130,130);
    printf("| Nom du fichier | Taille (en blocs) | Premier Bloc | Mode Globale | Mode Interne |\n");
    for (int i = 0; i < nbFichiers; i++) {
        printf("| %-15s | %-18d | %-12d | %-12s | %-14s |\n",
               fichiersMetadata[i].nom, fichiersMetadata[i].tailleEnBlocs,
               fichiersMetadata[i].adressePremierBloc,
               fichiersMetadata[i].modeOrganisationGlobale,
               fichiersMetadata[i].modeInsertion);
    }
}



// creation de fichier en mode chainé ou contigu
void creerFichier(char* nom, int taille, char* mode) {
    if (nbFichiers >= MAX_FILES) {
        printf("Limite de fichiers atteinte.\n");
        return;
    }

    int premierBloc = -1, dernierBloc = -1, blocsTrouves = 0;

    if (strcmp(mode, "Contigu") == 0) {
        for (int i = 0; i <= MAX_BLKS - taille; i++) {
            int contigu = 1;
            for (int j = i; j < i + taille; j++) {
                if (blocs[j].estOccupe) {
                    contigu = 0;
                    break;
                }
            }
            if (contigu) {
                premierBloc = i;
                for (int j = i; j < i + taille; j++) {
                    blocs[j].estOccupe = 1;
                    blocs[j].suivant = (j == i + taille - 1) ? -1 : j + 1;
                    strcpy(blocs[j].nomFichier, nom);
                }
                break;
            }
        }

        if (premierBloc == -1) {
            printf("Pas assez de blocs contigus disponibles.\n");
            return;
        }
    } else if (strcmp(mode, "Chaine") == 0) {
        for (int i = 0; i < MAX_BLKS; i++) {
            if (!blocs[i].estOccupe) {
                if (premierBloc == -1) {
                    premierBloc = i;
                } else {
                    blocs[dernierBloc].suivant = i;
                }
                blocs[i].estOccupe = 1;
                blocs[i].suivant = -1;
                strcpy(blocs[i].nomFichier, nom);
                dernierBloc = i;
                blocsTrouves++;
                if (blocsTrouves == taille) {
                    break;
                }
            }
        }

        if (blocsTrouves < taille) {
            printf("Pas assez de blocs disponibles pour le mode Chaine.\n");
            return;
        }
    } else {
        printf("Mode inconnu : %s. Veuillez choisir 'Contigu' ou 'Chaine'.\n", mode);
        return;
    }

    strcpy(fichiersMetadata[nbFichiers].nom, nom);
    fichiersMetadata[nbFichiers].tailleEnBlocs = taille;
    fichiersMetadata[nbFichiers].adressePremierBloc = premierBloc;
    strcpy(fichiersMetadata[nbFichiers].modeOrganisationGlobale, mode);// copier les donnes dans les metadonnes
    strcpy(fichiersMetadata[nbFichiers].modeInsertion, "Indefini");
    nbFichiers++;

    printf("Fichier '%s' cree en mode %s avec succes.\n", nom, mode);
}
// rennommer un fichier
void renommerFichier(char* ancienNom, char* nouveauNom) {
    for (int i = 0; i < nbFichiers; i++) {
        if (strcmp(fichiersMetadata[i].nom, ancienNom) == 0) {
            strcpy(fichiersMetadata[i].nom, nouveauNom);
            for (int j = 0; j < MAX_BLKS; j++) {
                if (strcmp(blocs[j].nomFichier, ancienNom) == 0) {
                    strcpy(blocs[j].nomFichier, nouveauNom);  // copier le nouveau nom dans metadonnes
                }
            }
            printf("Fichier renomme avec succes de '%s' à '%s'.\n", ancienNom, nouveauNom);
            return;
        }
    }
    printf("Fichier '%s' introuvable.\n", ancienNom);
}
// pour supprimer un fichier

void supprimerFichier(char* nom) {
    for (int i = 0; i < nbFichiers; i++) {
        if (strcmp(fichiersMetadata[i].nom, nom) == 0) {
            int bloc = fichiersMetadata[i].adressePremierBloc;
            while (bloc != -1) {
                blocs[bloc].estOccupe = 0;
                blocs[bloc].contenu.id = -1;
                strcpy(blocs[bloc].contenu.contenu, "");
                strcpy(blocs[bloc].nomFichier, "");  // Libérer le nom du fichier
                blocs[bloc].suivant = -1;
                bloc = blocs[bloc].suivant;
            }
            // Décaler les fichiers pour enlever l'entrée supprimée
            for (int j = i; j < nbFichiers - 1; j++) {
                fichiersMetadata[j] = fichiersMetadata[j + 1];
            }
            nbFichiers--;
            printf("Fichier '%s' supprimé avec succès.\n", nom);
            return;
        }
    }
    printf("Fichier '%s' introuvable.\n", nom);
}


// fonction pour rajouter un enregistrement selon le mode chaine ou contigu

void ajouterEnregistrements(char* nomFichier, Enregistrement* enregistrements, int nbEnregistrements, char* modeInsertion) {
    for (int i = 0; i < nbFichiers; i++) {
        if (strcmp(fichiersMetadata[i].nom, nomFichier) == 0) {
            // Mettre à jour le mode d'insertion dans les métadonnées
            strcpy(fichiersMetadata[i].modeInsertion, modeInsertion);

            int premierBloc = fichiersMetadata[i].adressePremierBloc;
            int taille = fichiersMetadata[i].tailleEnBlocs;

            if (strcmp(modeInsertion, "Contigu") == 0) {
                // Mode Contigu : Recherche d'un espace contigu pour chaque enregistrement
                for (int e = 0; e < nbEnregistrements; e++) {
                    int ajoutEffectue = 0;
                    for (int j = 0; j < MAX_BLKS; j++) {
                        if (!blocs[j].estOccupe) {
                            blocs[j].estOccupe = 1;
                            blocs[j].contenu = enregistrements[e];
                            strcpy(blocs[j].nomFichier, nomFichier);
                            ajoutEffectue = 1;
                            printf("Enregistrement ajouté (Contigu) : ID = %d, Contenu = '%s', Bloc = %d.\n",
                                   enregistrements[e].id, enregistrements[e].contenu, j);
                            break;
                        }
                    }
                    if (!ajoutEffectue) {
                        printf("Pas de blocs disponibles pour ajouter l'enregistrement ID = %d en mode Contigu.\n",
                               enregistrements[e].id);
                    }
                }
            } else if (strcmp(modeInsertion, "Chaine") == 0) {
                // Mode Chaîné : Recherche des blocs libres et chaînage
                for (int e = 0; e < nbEnregistrements; e++) {
                    int dernierBloc = -1, ajoutEffectue = 0;
                    for (int j = 0; j < MAX_BLKS; j++) {
                        if (!blocs[j].estOccupe) {
                            if (dernierBloc != -1) {
                                blocs[dernierBloc].suivant = j;
                            }
                            blocs[j].estOccupe = 1;
                            blocs[j].contenu = enregistrements[e];
                            strcpy(blocs[j].nomFichier, nomFichier);
                            dernierBloc = j;
                            ajoutEffectue = 1;
                            printf("Enregistrement ajouté (Chaîné) : ID = %d, Contenu = '%s', Bloc = %d.\n",
                                   enregistrements[e].id, enregistrements[e].contenu, j);
                            break;
                        }
                    }
                    if (!ajoutEffectue) {
                        printf("Pas de blocs libres pour ajouter l'enregistrement ID = %d en mode Chaîné.\n",
                               enregistrements[e].id);
                    }
                }
            } else {
                printf("Mode d'insertion invalide. Veuillez choisir 'Contigu' ou 'Chaine'.\n");
            }
            return;
        }
    }
    printf("Fichier '%s' introuvable.\n", nomFichier);
}




// rechercher un enregistrement selon son ID

void rechercherEnregistrementParID(int id) {
    int blocsTrouves = 0;  // Compteur des enregistrements trouvés
    int blocActuel;

    printf("Recherche des enregistrements avec l'ID %d :\n", id);
    for (int i = 0; i < MAX_BLKS; i++) {
        if (blocs[i].contenu.id == id) {
            blocActuel = i;
            while (blocActuel != -1) {
                // Affichage des enregistrements correspondants à l'ID
                printf("Bloc %d, Fichier : '%s', ID : %d, Contenu : '%s'\n",
                       blocActuel, blocs[blocActuel].nomFichier, blocs[blocActuel].contenu.id, blocs[blocActuel].contenu.contenu);
                blocsTrouves++;
                blocActuel = blocs[blocActuel].suivant;  // Passer au bloc suivant dans la chaîne
            }
        }
    }

    if (blocsTrouves == 0) {
        printf("Aucun enregistrement trouvé pour l'ID %d.\n", id);
    } else {
        printf("Total de %d enregistrement(s) trouvé(s) pour l'ID %d.\n", blocsTrouves, id);
    }
}


// supprimer l'enregitrement par son ID logique ou physique

void supprimerTousLesBlocsParID(int id) {
    int choix;
    printf("Choisissez le type de suppression:\n");
    printf("1. Suppression Logique (Remplacer le contenu par '**')\n");
    printf("2. Suppression Physique (Libérer le bloc)\n");
    printf("Choix : ");
    scanf("%d", &choix);

    int blocsSupprimes = 0;  // Compteur de blocs supprimés
    int blocActuel;

    for (int i = 0; i < MAX_BLKS; i++) {
        if (blocs[i].contenu.id == id) {
            blocActuel = i;
            while (blocActuel != -1) {
                if (choix == 1) {
                    // Suppression Logique : Remplacer le contenu par "**"
                    blocs[blocActuel].estLogiquementSupprime = 1;  // Marquer comme supprimé logiquement
                    blocs[blocActuel].contenu.id = -1;
                    strcpy(blocs[blocActuel].contenu.contenu, "**");  // Remplacer par "**"
                    printf("Bloc %d du fichier '%s' supprimé logiquement.\n", blocActuel, blocs[blocActuel].nomFichier);
                } else if (choix == 2) {
                    // Suppression Physique : Libérer le bloc
                    blocs[blocActuel].estOccupe = 0;  // Libérer le bloc
                    blocs[blocActuel].estLogiquementSupprime = 0;  // Réinitialiser l'état logique
                    blocs[blocActuel].contenu.id = -1;  // Réinitialiser l'ID
                    strcpy(blocs[blocActuel].contenu.contenu, "");  // Réinitialiser le contenu
                    strcpy(blocs[blocActuel].nomFichier, "");  // Réinitialiser le nom du fichier
                    blocs[blocActuel].suivant = -1;  // Réinitialiser le lien vers le bloc suivant
                    printf("Bloc %d du fichier '%s' supprimé physiquement.\n", blocActuel, blocs[blocActuel].nomFichier);
                } else {
                    printf("Choix invalide. La suppression a été annulée.\n");
                    return;
                }

                blocsSupprimes++;
                blocActuel = blocs[blocActuel].suivant;  // Passer au bloc suivant dans la chaîne
            }
        }
    }

    // Vérification s'il y a des blocs supprimés
    if (blocsSupprimes > 0) {
        printf("%d bloc(s) supprimé(s) pour l'ID %d.\n", blocsSupprimes, id);
    } else {
        printf("Aucun bloc trouvé pour l'ID %d.\n", id);
    }
}







// Fonction principale
int main() {
    initialiserBlocs();
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Cr%cer un fichier\n",130);
        printf("2. Ajouter un enregistrement\n");
        printf("3. Renommer un fichier\n");
        printf("4. Supprimer un fichier\n");
        printf("5. Rechercher un enregistrement par ID\n");
        printf("6. Supprimer un enregistrement\n");
        printf("7. Afficher les blocs\n");
        printf("8. Afficher les m%ctadonn%ces\n",130,130);
        printf("9. Quitter\n");
        printf("Choix : ");

        int choix;
        scanf("%d", &choix);

        if (choix == 1) {
            char nom[50], mode[20];
            int taille;
            printf("Nom du fichier : ");
            scanf("%s", nom);
            printf("Taille (en blocs) : ");
            scanf("%d", &taille);
            printf("Mode (Contigu/Chaine) : ");
            scanf("%s", mode);
            creerFichier(nom, taille, mode);
        }
        else if (choix == 2) {
    char nomFichier[50], mode[20];
    int nbEnregistrements;
    printf("Nom du fichier : ");
    scanf("%s", nomFichier);
    printf("Nombre d'enregistrements à ajouter : ");
    scanf("%d", &nbEnregistrements);

    Enregistrement enregistrements[nbEnregistrements];
    for (int i = 0; i < nbEnregistrements; i++) {
        printf("ID de l'enregistrement %d : ", i + 1);
        scanf("%d", &enregistrements[i].id);
        printf("Contenu de l'enregistrement %d : ", i + 1);
        scanf("%s", enregistrements[i].contenu);
    }

    printf("Mode d'insertion (Contigu/Chaine) : ");
    scanf("%s", mode);
    ajouterEnregistrements(nomFichier, enregistrements, nbEnregistrements, mode);
}
 else if (choix == 3) {
            char ancienNom[50], nouveauNom[50];
            printf("Ancien nom du fichier : ");
            scanf("%s", ancienNom);
            printf("Nouveau nom du fichier : ");
            scanf("%s", nouveauNom);
            renommerFichier(ancienNom, nouveauNom);
        } else if (choix == 4) {
            char nom[50];
            printf("Nom du fichier %c supprimer : ",133);
            scanf("%s", nom);
            supprimerFichier(nom);
        } else if (choix == 5) {
            int id;
            printf("ID de l'enregistrement %c rechercher : ",133);
            scanf("%d", &id);
            rechercherEnregistrementParID(id);
        } else if (choix == 6) {
            int id;
            printf("ID de l'enregistrement %c supprimer : ",133);
            scanf("%d", &id);
            supprimerTousLesBlocsParID(id);
        } else if (choix == 7) {
            afficherBlocs();
        } else if (choix == 8) {
            afficherMetadonnees();
        } else if (choix == 9) {
            printf("Au revoir !\n");
            break;
        } else {
            printf("Choix invalide.\n");
        }
    }

    return 0;
}
