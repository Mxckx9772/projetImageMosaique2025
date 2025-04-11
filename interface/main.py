import os
import tkinter as tk
from tkinter import filedialog, Label
from PIL import Image, ImageTk

root = tk.Tk()
root.title("Mosaïque d'images")
root.geometry("500x700")
root.configure(bg="#f0f0f0")

modes = ["Moyenne", "Distance de Pearson", "Distance de Bhattacharyya"]
imagette_sizes = ["4", "8", "16", "32", "64", "128"]
image_path = ""
blocksizes =  ["4", "8", "16", "32", "64", "128"]

#Affichage image
def display_image(path, target_label, max_size=(300, 300)):
    try:
        img = Image.open(path)
        img.thumbnail(max_size)
        img_tk = ImageTk.PhotoImage(img)
        target_label.configure(image=img_tk)
        target_label.image = img_tk
    except Exception as e:
        print(f"Erreur d'affichage de l'image : {e}")

#Fenêtre d'affichage de l'image
def show_image_in_new_window(image_path, title):
    image_window = tk.Toplevel(root)
    image_window.title(title)   
    image_label = Label(image_window)
    image_label.pack(fill="both", expand=True)
    img = Image.open(image_path)
    width, height = img.size
    aspect_ratio = width / height
    initial_width = min(500, width) 
    initial_height = int(initial_width / aspect_ratio)
    image_window.geometry(f"{initial_width}x{initial_height}")
    image_window.resizable(True, True)
    
    def update_image(event=None):
        if event is not None:
            new_width, new_height = event.width, event.height
            if new_width / new_height > aspect_ratio:
                new_width = int(new_height * aspect_ratio)
            else:
                new_height = int(new_width / aspect_ratio)
            display_image(image_path, image_label, max_size=(new_width, new_height))
    
    update_image()
    image_window.bind("<Configure>", update_image)

    def on_window_close():
        print("Fermeture de la fenêtre de l'image.")
        image_window.destroy()

    image_window.protocol("WM_DELETE_WINDOW", on_window_close)

# Choisir l'image
def choose_image():
    global image_path
    image_path = filedialog.askopenfilename(
        title="Sélectionner une image",
        filetypes=[("Image files", ["*.ppm", "*.pgm"])],
        initialdir="./in",
    )
    if image_path:
        file_label.config(text=f"Image choisie : {image_path}")
        show_image_in_new_window(image_path, image_path)
    else:
        file_label.config(text="Veuillez choisir une image à transformer en mosaïque")

# Fonction de transformation de l'image
def transform():
    selected_mode = modes.index(dropdown_var_mode.get())
    mode = str(selected_mode)
    imagette_size = str(current_imagette_size.get())
    blocksize = str(current_bloc_size.get())
    libsize = str(LibSize_entry.get())
    img_extension = "ppm" if image_path.endswith(".ppm") else "pgm"
    print(f"./bin/main {image_path} {img_extension} {blocksize} {imagette_size} {libsize}")
    os.system(f"./bin/main {image_path} {img_extension} {blocksize} {imagette_size} {libsize}")
    transformed_image_path = "./out/mosaic."+img_extension
    if os.path.exists(transformed_image_path):
        print(transformed_image_path)
        show_image_in_new_window(transformed_image_path, "Image transformée")
    else:
        print(transformed_image_path)
        print("Erreur : l'image transformée n'a pas été trouvée.")

#Widgets de l'interface
file_label = Label(root, text="Veuillez choisir une image à transformer en mosaïque", wraplength=350,bg="#f0f0f0")
file_label.pack(pady=20)

choose_button = tk.Button(root, text="Choisir une image", command=choose_image, bg="#4CAF50", fg="white")
choose_button.pack()

#Mode de transformation
mode_label = Label(root, text="Veuillez choisir un mode de transformation", wraplength=350, bg="#f0f0f0")
mode_label.pack(pady=10)
dropdown_var_mode = tk.StringVar(root)
dropdown_var_mode.set(modes[0])
dropdown_menu_mode = tk.OptionMenu(root, dropdown_var_mode, *modes)
dropdown_menu_mode.config(bg="#f0f0f0", fg="black")
dropdown_menu_mode["menu"].config(bg="#f0f0f0", fg="black")
dropdown_menu_mode.pack()

#slider Imagettes
size_label = Label(root, text="Veuillez choisir la taille des imagettes", wraplength=350, bg="#f0f0f0")
size_label.pack(pady=20)
current_imagette_size = tk.IntVar(value=16)

# Fonction pour arrondir automatiquement à la valeur la plus proche autorisée
def snap_slider_value(val):
    val = int(val)
    closest = min(imagette_sizes, key=lambda x: abs(int(x) - val))
    current_imagette_size.set(closest)
    slider_imagette.set(closest)

Label(root, text="Taille des imagettes", wraplength=350,bg="#f0f0f0").pack(pady=(10,0))

slider_imagette = tk.Scale(
    root, 
    from_=4, 
    to=128, 
    resolution=1, 
    orient="horizontal", 
    variable=current_imagette_size, 
    command=snap_slider_value,
    bg="#f0f0f0",
    fg="black",
)
slider_imagette.pack(pady=5)

bloc_label = Label(root, text="Veuillez choisir la taille des blocs", wraplength=350, bg="#f0f0f0")
bloc_label.pack(pady=20)

# Valeurs discrètes autorisées pour le slider
current_bloc_size = tk.IntVar(value=16)

def snap_bloc_value(val):
    val = int(val)
    closest = min(blocksizes, key=lambda x: abs(int(x) - val))
    current_bloc_size.set(closest)
    slider_bloc.set(closest)

Label(root, text="Taille des blocs", wraplength=350,bg="#f0f0f0").pack(pady=(10,0))

slider_bloc = tk.Scale(
    root, 
    from_=4, 
    to=128, 
    resolution=1, 
    orient="horizontal", 
    variable=current_bloc_size, 
    command=snap_bloc_value,
    bg="#f0f0f0",
    fg="black",
)
slider_bloc.pack(pady=10)

# Choisir la taille de la bibliothèque
LibSize_label = Label(root, text="Veuiller renseigner la taille de la bibliothèque", wraplength=350, bg="#f0f0f0").pack(pady=20)
LibSize_entry = tk.Entry(root, bg="#f0f0f0", fg="black")
LibSize_entry.pack(pady=5)

transform_button = tk.Button(root, text="Transformer l'image", command=transform, bg="#2196F3", fg="white").pack(pady=20)

root.mainloop()
