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
    private static extern long RastrCreate();
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long Load( long idRastr, string pch_fpath, string  pch_tpath );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long Save( long idRastr, string pch_fpath, string pch_tpath );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long Rgm( long idRastr, string pch_parameters );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long GetTableNumRows( long idRastr, string pch_table, ref long n_rows_out );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long SetValInt( long idRastr, string pch_table, string pch_col, long n_row, long n_val );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long GetValInt( long idRastr, string pch_table, string pch_col, long n_row, ref long n_val_out );
    [DllImport(str_path_dll_, CharSet = CharSet.Ansi)]
    private static extern long GetValDbl( long idRastr, string pch_table, string pch_col, long n_row, ref double d_val_out );

    public long call_test()
    {
        long nRes =0 ;

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        { }
        
        nRes = (int)test();

        long idRastr = -1;
        idRastr = RastrCreate();
        nRes = Load( idRastr, "/home/ustas/projects/test-rastr/Metro/2023_06_28/d1", "" );
        nRes = Rgm( idRastr, "p1" );
        long nTable_node_NumRows = 0;
        nRes = GetTableNumRows( idRastr, "node", ref nTable_node_NumRows );
        nRes = SetValInt( idRastr, "node", "na", 0, 1 );
        long ll = 0;
        nRes = GetValInt( idRastr, "node", "na", 0, ref  ll );
        double dd = 0;
        nRes = GetValDbl( idRastr, "node", "pg", 0, ref dd );
        nRes = Save( idRastr, "/home/ustas/projects/test-rastr/Metro/2023_06_28/d1_222", "" );

        return nRes;
    }

}