namespace tst_wrk_plain_dll;

class Program
{
    static void Log(string str)
    {
        Console.WriteLine(str);
    }

    static void Main(string[] args)
    {
        Console.WriteLine("Hello, World!");
        try
        { 
            //c++ /home/ustas/projects/c2
            Log($"tst_astra_beg\n");
            CRwrapper rw = new CRwrapper();
            long nRes = rw.call_test();
            Log($"tst_astra_end. get[{nRes}]\n");
        }
        catch (Exception ex) 
        {
            Log($"catch exception: {ex}");
        }
    }
}
