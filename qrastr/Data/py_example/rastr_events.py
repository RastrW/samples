import time
import struct
import socket
import sys

file_test = "../Data/Files/cx195.rg2"
file_test_templ = "../Data/SHABLON/режим.rg2"

#import astra_py as astra
rastr2 = astra.Rastr()

rastr.load(astra.LoadCode.REPL, file_test, file_test_templ)
rastr.print(f"Загружен файл {file_test} по шаблону {file_test_templ}")

try:
  ret = rastr.rgm("p")
  rastr.print('Run rgm(p): retcode = ', str(ret) )
except Exception  as err:
  rastr.print('Handling run-time error:', err)

rastr.print( f"")
for i in range(0,10,1):
    if (i == 3): continue
    if (i == 6): break
    rastr.print( f"{i} : итерация цикла")

node=rastr.table("node")
pn= node.column("pn")
node.set_selection("na=1")
pn.calculate("pn*1.2") # изменяем нагрузку в районе 1
rastr.rgm("zcr") # повторный расчет режима

kod = rastr.rgm("")
if (kod != astra.ASTCode.OK):
    rastr.print("Режим не сбалансирован")

#Запись в файл номеров и названий линий. Разделитель – точка с запятой.
file = "c:/tmp/a.csv"
vetv = rastr["vetv"]
vetv.set_selection("x>100") # по условию
#vetv.set_selection("sta")  # отключенных линий
#!lin not work! vetv.to_csv(astra.CSVCode.REPLACE, file, "ip,iq,name", ";")

#доступ к данным
#rastr.print( f"")
#tnode = rastr["node"]
#ss = tnode.set_selection("=1,2,3")
#data = tnode.data(["ny", "name", "uhom"])
#rastr.print(data["ny"])
#rastr.print(data["name"])
#rastr.print(data["uhom"])
#rastr.print(data["index"])

#Пересчет реактивного сопротивления ЛЭП пропорционально активному.
tvetv = rastr["vetv"]
tvetv.set_selection("tip=0")
tvetv["x"].calculate("4*r")

#Перечисление колонок таблицы
rastr.print( f"")
number = 0
for col in rastr["node"].columns():
    rastr.print(col.name)
    number+=1
    if number == 3 :    # если number = 3, выходим из цикла
        break

#Перечисление таблиц
rastr.print( f"")
tabs = rastr.tables()
for i in range(0, tabs.count-1):
    rastr.print(tabs[i])
    if (i == 6): break

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

#from turtle import Turtle
#from random import random

#t = Turtle()
#for i in range(30):
#    steps = int(random() * 100)
#    angle = int(random() * 360)
#    t.right(angle)
#    t.fd(steps)
#t.screen.mainloop()


#from tkinter import *
#root = Tk()
#root.geometry('160x140+800+300')
#Label(text='Thats all folks', width=20, height=3).pack()
#root.mainloop()