import os
import tkinter as tk
from tkinter import filedialog, Label
from PIL import Image, ImageTk

root = tk.Tk()
root.title("Crypto Mosaic")
root.configure(bg="#f0f0f0")

modes = ["Moyenne", "Distance de Bhattacharyya","Distance de Chi2"]
sec_modes = ["Vernam", "Swap"]
imagette_sizes = ["4", "8", "16", "32", "64", "128"]
image_path = ""
blocksizes =  ["4", "8", "16", "32", "64", "128"]

def display_image(path, target_label, size):
    try:
        img = Image.open(path)
        img.thumbnail(size)
        img_tk = ImageTk.PhotoImage(img)
        target_label.configure(image=img_tk)
        target_label.image = img_tk
    except Exception as e:
        print(f"Erreur d'affichage de l'image : {e}")

def show_image_in_new_window(image_path, title):
    image_window = tk.Toplevel(root)
    image_window.title(title)   
    image_label = Label(image_window)
    image_label.pack(fill="both", expand=True)
    img = Image.open(image_path)
    width, height = img.size
    aspect_ratio = width / height
    initial_width = 350
    initial_height = int(initial_width / aspect_ratio)
    image_window.geometry(f"{initial_width}x{initial_height}")
    image_window.minsize(350, initial_height)
    image_window.resizable(True, True)

    def update_image(event=None):
        if event is not None:
            new_width, new_height = event.width, event.height
            if new_width / new_height > aspect_ratio:
                new_width = int(new_height * aspect_ratio)
            else:
                new_height = int(new_width / aspect_ratio)
            display_image(image_path, image_label, size=(new_width, new_height))
    
    update_image()
    image_window.bind("<Configure>", update_image)

    def on_window_close():
        image_window.destroy()

    image_window.protocol("WM_DELETE_WINDOW", on_window_close)

def choose_image():
    global image_path
    image_path = filedialog.askopenfilename(
        title="Sélectionner une image",
        filetypes=[("Image files", ["*.ppm", "*.pgm"])],
        initialdir="./in",
    )
    if image_path:
        transform_button.config(state=tk.NORMAL)
        transform_button.config(bg="#2196F3")
        file_label.config(text=f"Image choisie : {image_path}")
    else:
        file_label.config(text="Veuillez choisir une image :")

def transform():
    selected_mode = modes.index(dropdown_var_mode.get())
    mode = str(selected_mode)
    selected_sec_mode = sec_modes.index(dropdown_var_sec_mode.get())
    sec_mode = str(selected_sec_mode)
    imagette_size = str(current_imagette_size.get())
    blocksize = str(current_bloc_size.get())
    libsize = str(LibSize_entry.get())
    img_extension = "ppm" if image_path.endswith(".ppm") else "pgm"
    if sec_var.get():
        os.system(f"./bin/encrypt {image_path} {sec_mode}")
    else:
        os.system(f"./bin/main {image_path} {img_extension} {blocksize} {imagette_size} {libsize} {mode}")
    transformed_image_path = "./out/mosaic."+img_extension
    if os.path.exists(transformed_image_path):
        show_image_in_new_window(image_path, "Image originale")
        show_image_in_new_window(transformed_image_path, "Image transformée")
    else:
        print("Erreur : l'image transformée n'a pas été trouvée.")

# Widgets de l'interface
file_label = Label(root, text="Veuillez choisir une image", wraplength=350,bg="#f0f0f0")
file_label.pack(pady=10)
choose_button = tk.Button(root, text="Choisir une image", command=choose_image, bg="#4CAF50", fg="white")
choose_button.pack()

# Mode de transformation
mode_label = Label(root, text="Veuillez choisir un mode de transformation", wraplength=350, bg="#f0f0f0")
mode_label.pack(padx=20, pady=10)
dropdown_var_mode = tk.StringVar(root)
dropdown_var_mode.set(modes[0])
dropdown_menu_mode = tk.OptionMenu(root, dropdown_var_mode, *modes)
dropdown_menu_mode.config(bg="#f0f0f0", fg="black")
dropdown_menu_mode["menu"].config(bg="#f0f0f0", fg="black")
dropdown_menu_mode.pack()

bloc_label = Label(root, text="Veuillez choisir la taille des blocs", wraplength=350, bg="#f0f0f0")
bloc_label.pack(pady=10)

# Valeurs discrètes autorisées pour le slider
current_bloc_size = tk.IntVar(value=16)

def snap_bloc_value(val):
    val = int(val)
    closest = min(blocksizes, key=lambda x: abs(int(x) - val))
    current_bloc_size.set(closest)
    slider_bloc.set(closest)

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
slider_bloc.pack()

#slider Imagettes
size_label = Label(root, text="Veuillez choisir la taille des blocs dans l'image transformée", wraplength=350, bg="#f0f0f0")
size_label.pack(pady=10)
current_imagette_size = tk.IntVar(value=16)

# Fonction pour arrondir automatiquement à la valeur la plus proche autorisée
def snap_slider_value(val):
    val = int(val)
    closest = min(imagette_sizes, key=lambda x: abs(int(x) - val))
    current_imagette_size.set(closest)
    slider_imagette.set(closest)

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
slider_imagette.pack()

# Choisir la taille de la bibliothèque
Label(root, text="Veuiller renseigner la taille de la bibliothèque", wraplength=350, bg="#f0f0f0").pack(pady=10)
LibSize_entry = tk.Entry(root, bg="#f0f0f0", fg="black")
LibSize_entry.insert(0, '20580')
LibSize_entry.pack()

# Sécurité
Label(root, text="Voulez-vous chiffrer l'image ?", wraplength=350, bg="#f0f0f0").pack(pady=10)
sec_var = tk.BooleanVar()
security_check = tk.Checkbutton(root, variable=sec_var, bg="#f0f0f0", fg="black")
security_check.pack()

Label(root, text="Veuillez choisir un mode de chiffrement", wraplength=350, bg="#f0f0f0").pack(padx=20, pady=10)
dropdown_var_sec_mode = tk.StringVar(root)
dropdown_var_sec_mode.set(sec_modes[0])
dropdown_menu_sec_mode = tk.OptionMenu(root, dropdown_var_sec_mode, *sec_modes)
dropdown_menu_sec_mode.config(bg="#f0f0f0", fg="black")
dropdown_menu_sec_mode["menu"].config(bg="#f0f0f0", fg="black")
dropdown_menu_sec_mode.pack()

transform_button = tk.Button(root, text="Transformer l'image", command=transform, fg="white")
transform_button.config(state=tk.DISABLED)
transform_button.config(bg="#777777")
transform_button.pack(pady=20)

root.mainloop()
