#python script must be encoded 'utf-8' !!!!
import ctypes 
import json
import pprint
import time

def get_max_buf_len(): return 100_000_000

path_to_astra = r'C:\projects\rastr\RastrWin\build-astra-vs-Debug\astra_py.cp311-win_amd64.pyd'
#path_to_rfile = r'C:\TMP\праверка русскаго\схема .rg2'
path_to_rfile = r'D:\Vms\SHARA\huge_schems\65219.rg2'
path_to_rshbl = r''
print(path_to_astra)
dllAstra = ctypes.cdll.LoadLibrary(path_to_astra)
print(dllAstra)
idRastr = dllAstra.RastrCreate()
print(f'idRastr: {idRastr}')
n_res = dllAstra.Load( idRastr, path_to_rfile, path_to_rshbl )
print(f'{n_res} = Load( {idRastr}, {path_to_rfile}, {path_to_rshbl} ) ')
n_res = dllAstra.Rgm( idRastr, "" )
print(f'{n_res} = Rgm( {idRastr}, "" )')
time_start_strbuf = time.time()
str_JSON = ctypes.create_string_buffer(get_max_buf_len())
#dllAstra.GetJSON.restype  = c_long
#dllAstra.GetJSON.argtypes = [c_long, c_char_p, c_char_p, c_char_p, c_char_p, c_char_p, c_long]
time_start_getJSON = time.time()
#n_res = dllAstra.GetJSON( idRastr, 'node'.encode('utf-8'), 'ny,name,vras,delta'.encode('utf-8') , '1'.encode('utf-8'), 'j'.encode('utf-8'), str_JSON, str_JSON._length_ )
#n_res = dllAstra.GetJSON( idRastr, 'vetv'.encode('utf-8'), 'ip,iq,name,dname,r,x,g,b,ktr,kti,sle,slb'.encode('utf-8') , '1'.encode('utf-8'), ''.encode('utf-8'), str_JSON, str_JSON._length_ )
#n_res = dllAstra.GetJSON( idRastr, 'vetv'.encode('utf-8'), 'ip,iq,name,dname,r,x,g,b,ktr,kti'.encode('utf-8') , '1'.encode('utf-8'), ''.encode('utf-8'), str_JSON, str_JSON._length_ )
n_res = dllAstra.GetJSON( idRastr, 'vetv'.encode('utf-8'), 'ip,iq,dname,r,x,g,b,ktr,kti'.encode('utf-8') , '1'.encode('utf-8'), ''.encode('utf-8'), str_JSON, str_JSON._length_ )
#print( "pBuff: %s" % str_JSON.value)
#print(str_JSON.value.decode('utf-8'))
time_start_string_at = time.time()
ww = ctypes.string_at(str_JSON)
time_start_loads = time.time()
json_data = json.loads(ww)
time_start_pprint = time.time()
# print json to screen with human-friendly formatting
print(f'time_start_getJSON   - time_start_strbuf  = {time_start_getJSON   - time_start_strbuf} - alloc buffer' )
print(f'time_start_string_at - time_start_getJSON = {time_start_string_at - time_start_getJSON} - get json from Astra' )
print(f'time_start_loads     - time_start_string_at = {time_start_loads   - time_start_string_at } - get string from char*' )
print(f'time_start_pprint    - time_start_loads = {time_start_pprint  - time_start_loads } - get JSON object from char*' )
pprint.pprint(json_data, compact=False)
print('bay bay! \n')
