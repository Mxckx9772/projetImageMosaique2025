import os
import tkinter as tk
from tkinter import filedialog, Label

root = tk.Tk()
root.title("Projet Image")

modes = ["Moyenne", "Distance d'histogramme"]
imagette_sizes = ["4", "8", "16", "32", "64", "128"]
image_path = ""

def choose_image():
    global image_path

    image_path = filedialog.askopenfilename(
        title="Sélectionner une image",
        filetypes=[("Image files", ["*.ppm", "*.pgm"])]
    )
    if image_path:
        file_label.config(text=f"Image choisie : {image_path}")
        transform_button.pack(pady=20)
    else:
        file_label.config(text="Veuillez choisir une image à transformer en mosaïque")

def transform():
    # TODO implémenter les arguments du côté C++
    if dropdown_var_mode.get() == modes[0]:
        mode = "0"
    else:
        mode = "1"
    imagette_size = dropdown_var_size.get()
    print(f"../base_code_mosaique {mode} {image_path} {imagette_size}")
    os.system(f"../base_code_mosaique {mode} {image_path} {imagette_size}")

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
