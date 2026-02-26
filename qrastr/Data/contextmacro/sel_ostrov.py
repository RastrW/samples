

rastr.print(f"Hello from contextmacro ! row = {aRow}")

def VKL(index: int) -> None:   
    node = rastr["node"]
    vetv = rastr["vetv"]
    sel = node["sel"]
    ny = node["ny"]
    vet_ip = vetv["ip"]
    vet_iq = vetv["iq"]
    vet_sta = vetv["sta"]
    
    n = ny[index]
    sel[index] = True
    vetv.set_selection(f"ip={n}|iq={n}")
    j = vetv.find_next(-1)

    while j != -1:
        st = vet_sta[j]
        if st == 0:
            nf = vet_ip[j]
            if nf == n:
                nf = vet_iq[j]
            node.set_selection(f"ny={nf}")
            jn = node.find_next(-1)

            if jn != -1 and sel[jn] == False:
                VKL(jn)
        j = vetv.find_next(j)


VKL(aRow)
