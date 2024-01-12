using System.Runtime.InteropServices;

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
    private static extern System.Int32 GetValDbl( System.Int32 idRastr, string pch_table, string pch_col, System.Int32 n_row, ref double d_val_out );

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
        nRes = (int)test();
        System.Int32 idRastr = -1;
        idRastr = RastrCreate();
        nRes = Load( idRastr, str_path_file_load, "" );
        nRes = Rgm( idRastr, "p1" );
        System.Int32 nTable_node_NumRows = 0;
        nRes = GetTableNumRows( idRastr, "node", ref nTable_node_NumRows );
        nRes = SetValInt( idRastr, "node", "na", 0, 1 );
        System.Int32 ll = 0;
        nRes = GetValInt( idRastr, "node", "na", 0, ref  ll );
        double dd = 0;
        nRes = GetValDbl( idRastr, "node", "pg", 0, ref dd );
        nRes = Save( idRastr, str_path_file_save, "" );

        return nRes;
    }

}