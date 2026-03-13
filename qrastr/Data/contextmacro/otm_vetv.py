#aRow = 13 test on cx195->connected 811,813,831

def MARK_CONNECTED_NODES(index: int) -> None:   
    node = rastr["node"]
    vetv = rastr["vetv"]
    ny = node["ny"]
    sel = node["sel"]
    vet_ip = vetv["ip"]
    vet_iq = vetv["iq"]
    
    n = ny[index]
    vetv.set_selection(f"ip={n}|iq={n}")
    j = vetv.find_next(-1)
    while j != -1:
        nf = vet_ip[j]
        if nf == n:
            nf = vet_iq[j]
        node.set_selection(f"ny={nf}")
        sel.calculate("1")
        j = vetv.find_next(j)
            
    
MARK_CONNECTED_NODES(aRow)