#!/bin/bash

# Étape 1: Compiler le projet avec make
echo "Compilation du projet..."
make

# Vérifier si la compilation a réussi
if [ $? -ne 0 ]; then
    echo "Erreur de compilation !"
    exit 1
fi

echo "Compilation réussie."

# Étape 2: Lancer l'application Python
echo "Lancement de l'application Python..."
python3 ./interface/main.py

# Vérifier si l'application Python a réussi à démarrer
if [ $? -ne 0 ]; then
    echo "Erreur lors du lancement de l'application Python !"
    exit 1
fi

echo "Application Python lancée avec succès."
