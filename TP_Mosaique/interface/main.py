import os
import tkinter as tk
from tkinter import filedialog, Label

root = tk.Tk()
root.title("Projet Image")

mode = "0"
image_path = ""
imagette_size = "32"

def choose_image():
    global image_path

    image_path = filedialog.askopenfilename(
        title="Sélectionner une image",
        filetypes=[("Image files", ["*.ppm", "*.pgm"])]
    )
    if image_path:
        file_label.config(text=f"Image choisie : {image_path}")
        choose_button.destroy()
        transform_button.pack(pady=20)
    else:
        file_label.config(text="Veuillez choisir une image à transformer en mosaïque")

def transform():
    transform_button.destroy()
    # TODO implémenter les arguments du côté C++
    os.system(f"../base_code_mosaique {mode} {image_path} {imagette_size}")

file_label = Label(root, text="Veuillez choisir une image à transformer en mosaïque", wraplength=350)
file_label.pack(pady=20)
file_label.pack(padx=20)

choose_button = tk.Button(root, text="Choisir l'image", command=choose_image)
choose_button.pack(pady=20)

transform_button = tk.Button(root, text="Transformer l'image", command=transform)

root.mainloop()
