

rastr.print(f"Hello from contextmacro ! row = {aRow}")

def VKL(i:int):
    node=rastr.table("node")
    vetv=rastr.table("vetv")
    sel = node.column("sel")
    ny = node.column("ny")
    #sta = node.column("sta")
    vet_ip = vetv.column("ip")
    vet_iq = vetv.column("iq")
    vet_sta = vetv.column("sta")
    
    n = ny[i]
    #print(f"node = {n}")
    sel[i] = True
    vetv.set_selection(f"ip= {n} |iq= {n}")
    j = vetv.find_next(-1)
    #print(f"j= {j}")

    while j>-1:
        st = vet_sta[j]
        if st==0:
            nf=vet_ip[j]
           # print(f"nf= {nf}")
            if nf==n: nf=vet_iq[j]
            node.set_selection(f"ny={nf}")
            jn=node.find_next(-1)
            #print(f"jn= {jn}")

            if jn>-1:
                if sel[jn]==False:
                    VKL(jn) 
        #print(f"j= {j}")
        j=vetv.find_next(j)


VKL(aRow)