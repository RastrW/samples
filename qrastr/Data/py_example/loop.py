import os
#print(os.get_exec_path())
print(os.getcwd())

#if from python!
#from astra_py import Rastr, FieldProperties as FP, PropType
#rastr = Rastr()

# для отладки, test.py лежит рядом с  astra.pyd
#sys.path.append(os.getcwd())
sys.path.append(os.path.join(os.path.dirname(__file__),'.'))

rastr.rgm('p1')
#rastr.new_file('')

from tkinter import * #sudo apt-get install python3-tk


def about():
    a = Toplevel()
    a.geometry('200x150')
    a['bg'] = 'grey'
    a.overrideredirect(True)
    Label(a, text="About this").pack(expand=1)
    a.after(5000, lambda: a.destroy())

root = Tk()
root.title("Главное окно")
Button(text="Button", width=20).pack()
Label(text="Label", width=20, height=3).pack()
Button(text="About", width=20, command=about).pack()

root.mainloop()

"""
for x in range(6):
  if x == 3: break
  print(x)
else:
  print("Finally finished!")
"""