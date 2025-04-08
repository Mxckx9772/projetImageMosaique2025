import os
import tkinter as tk
from tkinter import filedialog, Label
from PIL import Image, ImageTk

root = tk.Tk()
root.title("Projet Image")

modes = ["Moyenne", "Distance d'histogramme"]
imagette_sizes = ["4", "8", "16", "32", "64", "128"]
image_path = ""

def display_image(path, target_label, max_size=(300, 300)):
    try:
        img = Image.open(path)
        img.thumbnail(max_size)
        img_tk = ImageTk.PhotoImage(img)
        target_label.configure(image=img_tk)
        target_label.image = img_tk
    except Exception as e:
        print(f"Erreur d'affichage de l'image : {e}")

def show_image_in_new_window(image_path, title):
    # Créer une nouvelle fenêtre Toplevel
    image_window = tk.Toplevel(root)
    image_window.title(title)
    
    # Ajouter un Label pour afficher l'image
    image_label = Label(image_window)
    image_label.pack(fill="both", expand=True)

    # Ouvrir l'image pour obtenir ses dimensions
    img = Image.open(image_path)
    width, height = img.size
    aspect_ratio = width / height

    # Adapter la géométrie de la fenêtre à la taille de l'image
    initial_width = min(500, width)  # Taille initiale de la fenêtre
    initial_height = int(initial_width / aspect_ratio)
    image_window.geometry(f"{initial_width}x{initial_height}")  # Adapter à l'image
    image_window.resizable(True, True)  # Permet le redimensionnement

    # Fonction pour ajuster l'image lors du redimensionnement de la fenêtre
    def update_image(event=None):
        if event is not None:  # Vérifier si l'événement est valide
            new_width, new_height = event.width, event.height
            # Maintenir le ratio d'aspect
            if new_width / new_height > aspect_ratio:
                new_width = int(new_height * aspect_ratio)
            else:
                new_height = int(new_width / aspect_ratio)
            # Afficher l'image dans la fenêtre redimensionnée
            display_image(image_path, image_label, max_size=(new_width, new_height))

    # Initialiser l'affichage de l'image
    update_image()

    # Mettre à jour l'image lors du redimensionnement de la fenêtre
    image_window.bind("<Configure>", update_image)

    # Ajouter une gestion de la fermeture de la fenêtre
    def on_window_close():
        print("Fermeture de la fenêtre de l'image.")
        image_window.destroy()  # Fermer la fenêtre

    # Associer la fonction on_window_close au bouton de fermeture (croix)
    image_window.protocol("WM_DELETE_WINDOW", on_window_close)

def choose_image():
    global image_path

    image_path = filedialog.askopenfilename(
        title="Sélectionner une image",
        filetypes=[("Image files", ["*.ppm", "*.pgm"])]
    )
    if image_path:
        file_label.config(text=f"Image choisie : {image_path}")
        transform_button.pack(pady=20)
        show_image_in_new_window(image_path, image_path)
    else:
        file_label.config(text="Veuillez choisir une image à transformer en mosaïque")

def transform():
    if dropdown_var_mode.get() == modes[0]:
        mode = "0"
    else:
        mode = "1"
    imagette_size = dropdown_var_size.get()
    print(f"./base_code_mosaique {mode} {image_path} {imagette_size}")
    os.system(f"./base_code_mosaique {mode} {image_path} {imagette_size}")
    transformed_image_path = image_path.replace(".ppm", "_mosaique.ppm").replace(".pgm", "_mosaique.pgm")
    if os.path.exists(transformed_image_path):
        print(transformed_image_path)
        show_image_in_new_window(transformed_image_path, "Image transformée")
    else:
        print(transformed_image_path)
        print("Erreur : l'image transformée n'a pas été trouvée.")

file_label = Label(root, text="Veuillez choisir une image à transformer en mosaïque", wraplength=350)
file_label.pack(pady=20)
file_label.pack(padx=20)

choose_button = tk.Button(root, text="Choisir une image", command=choose_image)
choose_button.pack()

mode_label = Label(root, text="Veuillez choisir un mode de transformation", wraplength=350)
mode_label.pack(pady=20)
mode_label.pack(padx=20)

dropdown_var_mode = tk.StringVar(root)
dropdown_var_mode.set(modes[0])
dropdown_menu_mode = tk.OptionMenu(root, dropdown_var_mode, *modes)
dropdown_menu_mode.pack()

size_label = Label(root, text="Veuillez choisir la taille des imagettes", wraplength=350)
size_label.pack(pady=20)
size_label.pack(padx=20)

dropdown_var_size = tk.StringVar(root)
dropdown_var_size.set(imagette_sizes[3])
dropdown_menu_size = tk.OptionMenu(root, dropdown_var_size, *imagette_sizes)
dropdown_menu_size.pack(pady=10)

transform_button = tk.Button(root, text="Transformer l'image", command=transform)

root.mainloop()
