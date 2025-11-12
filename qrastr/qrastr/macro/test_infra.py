import time
import struct
import socket
import sys

import tkinter as tk
from tkinter import ttk

root = tk.Tk()
root.title("Widgets Demo")

widgets = [
    tk.Label,
    tk.Checkbutton,
    ttk.Combobox,
    tk.Entry,
    tk.Button,
    tk.Radiobutton,
    tk.Scale,
    tk.Spinbox,
]

for widget in widgets:
    try:
        widget = widget(root, text=widget.__name__)
    except tk.TclError:
        widget = widget(root)
    widget.pack(padx=5, pady=5, fill="x")

root.mainloop()

from turtle import Turtle
from random import random

t = Turtle()
for i in range(30):
    steps = int(random() * 100)
    angle = int(random() * 360)
    t.right(angle)
    t.fd(steps)
t.screen.mainloop()


#from tkinter import *
#root = Tk()
#root.geometry('160x140+800+300')
#Label(text='Thats all folks', width=20, height=3).pack()
#root.mainloop()



