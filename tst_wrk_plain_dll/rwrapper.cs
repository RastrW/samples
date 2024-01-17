using System;
using System.Dynamic;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

public class CRwrapper
{
#if OS_WIN // must be added in .csproj !
    public const string str_path_dll_ = "C:\\projects\\rastr\\RastrWin\\astra\\build\\Debug\\astra_shrd.dll";
#else
    public const string str_path_dll_ = "/home/ustas/projects/git_r/rastr/RastrWin/astra/build/libastra_shrd.so";
#endif
    [DllImport(str_path_dll_)]
    private static extern int test(); 
    [DllImport(str_path_dll_)]
    private static extern System.Int32 RastrCreate();  //!!! must be int32 instead of long!
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)] //!!! must be CharSet.Ansi!
    //private static extern long Load( long idRastr, string pch_fpath, string  pch_tpath );
    private static extern System.Int32 Load( System.Int32 idRastr, string pch_fpath, string  pch_tpath );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 Save( System.Int32 idRastr, string pch_fpath, string pch_tpath );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 Rgm( System.Int32 idRastr, string pch_parameters );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 GetTableNumRows( System.Int32 idRastr, string pch_table, ref System.Int32 n_rows_out );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 SetValInt( System.Int32 idRastr, string pch_table, string pch_col, System.Int32 n_row, System.Int32 n_val );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 GetValInt( System.Int32 idRastr, string pch_table, string pch_col, System.Int32 n_row, ref System.Int32 n_val_out );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 SetValDbl( System.Int32 idRastr, string pch_table, string pch_col, System.Int32 n_row, double d_val );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 GetValDbl( System.Int32 idRastr, string pch_table, string pch_col, System.Int32 n_row, ref double d_val_out );

    //https://stackoverflow.com/questions/32991274/return-string-from-c-dll-export-function-called-from-c-sharp
   // [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern System.Int32 GetJSON( System.Int32 idRastr, string pch_table, string pch_cols, string pch_params, StringBuilder str, int strlen );

    public static void Log(string str)
    {
        Console.WriteLine(str);
    }

    public long call_test()
    {
        System.Int32 nRes = 0;
        string str_path_file_load ="";
        string str_path_file_save ="";
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux)){ 
            str_path_file_load = "/home/ustas/projects/test-rastr/Metro/2023_06_28/d1";
            str_path_file_save = "/home/ustas/projects/test-rastr/Metro/2023_06_28/d1_222";
        }else{ 
            str_path_file_load = @"C:\Users\ustas\Documents\RastrWin3\test-rastr\cx195.rg2";
            str_path_file_save = @"C:\Users\ustas\Documents\RastrWin3\test-rastr\cx195_222.rg2";
        }
        Log($"dll : {str_path_dll_}");
        Log($"Load: {str_path_file_load}");
        Log($"Save: {str_path_file_save}");

        nRes = (int)test();
        Log($"{nRes} = test()");

        System.Int32 idRastr = -1;
        idRastr = RastrCreate();
        Log($"{idRastr} = idRastr = RastrCreate()");

        nRes = Load( idRastr, str_path_file_load, "" );
        Log($"{nRes} = Load( {idRastr}, {str_path_file_load}, \"\" )");

        nRes = Rgm( idRastr, "p1" );
        Log($"{nRes} = Rgm( {idRastr}, \"p1\" )");

        System.Int32 nTable_node_NumRows = 0;
        nRes = GetTableNumRows( idRastr, "node", ref nTable_node_NumRows );
        Log($"{nRes} = GetTableNumRows( {idRastr}, \"node\", ref {nTable_node_NumRows} )");

        nRes = SetValInt( idRastr, "node", "na", 0, 1 );
        Log($"{nRes} = SetValInt( {idRastr}, \"node\", \"na\", 0, 1 )");

        System.Int32 ll = 0;
        nRes = GetValInt( idRastr, "node", "na", 0, ref  ll );
        Log($"{nRes} = GetValInt( {idRastr}, \"node\", \"na\", 0, ref  {ll} )");

        double dd = 0;
        nRes = GetValDbl( idRastr, "node", "pg", 0, ref dd );
        Log($"{nRes} = GetValDbl( {idRastr}, \"node\", \"pg\", 0, ref {dd} )");

        nRes = SetValDbl( idRastr, "node", "pn", 0,  100500.5 );
        Log($"{nRes} = SetValDbl( {idRastr}, \"node\", \"pn\", 0,  100500.5 )");

//        nRes = Save( idRastr, str_path_file_save, "" );
        Log($"{nRes} = Save( {idRastr}, {str_path_file_save}, \"\" )");

        var newArr = new JsonArray();
        for(int i = 0 ; i < 10_000 ;i++)
        {
            var coder = new JsonArray();
            coder.Add(JsonValue.Create(i));
	        coder.Add(JsonValue.Create( $"привед_name_{i}")) ;
            //var jsonNode = JsonNode.Parse("[0,\"\"]");    
            //jsonNode[0] = i;
            //jsonNode[1] = $"_name_{i}";
            //newArr.Add(jsonNode.);
            newArr.Add(coder);
        }

        string str_new_j_array = newArr.ToString();

        const int STRING_MAX_LENGTH = 1_000_000_000;
        StringBuilder str = new StringBuilder(STRING_MAX_LENGTH);
        //nRes = GetJSON( idRastr, "node", "ny,name,vras", "paramas", str, STRING_MAX_LENGTH );
        nRes = GetJSON( idRastr, str_new_j_array, "ny,name,vras", "paramas", str, STRING_MAX_LENGTH );

        Console.OutputEncoding = Encoding.UTF8;
        Log(str.ToString());
     
        byte[] bytes = Encoding.Default.GetBytes(str.ToString());
        //string myString = Encoding.UTF8.GetString(bytes);
        //string myString = Decoding.UTF8.GetString(bytes);

        string str_tmp = newArr.ToString();

        //var converter = new ExpandoObjectConverter();
        //dynamic obj = JsonConvert.DeserializeObject<ExpandoObject>(json, converter);
        JsonArray j_arr_get = JsonNode.Parse(str.ToString()).AsArray();
        string str_get_j_arr = j_arr_get.ToString();
        
        CultureInfo currentCulture = Thread.CurrentThread.CurrentCulture;
        byte[] encodedBytes = Encoding.UTF8.GetBytes(str.ToString());
Encoding.Convert(Encoding.UTF8, Encoding.Unicode, encodedBytes);

        //var data = JsonSerializer.Deserialize<Dictionary<string,string>>(str.ToString());

        return nRes;
    }
}


