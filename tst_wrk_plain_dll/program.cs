namespace tst_wrk_plain_dll;

class Program
{
    static void Main(string[] args)
    {
        CRwrapper.Log($"Hello, World!");
        try
        { 
            CRwrapper.Log($"tst_astra.begin");
            CRwrapper rw = new CRwrapper();
            long nRes = rw.call_test();
            CRwrapper.Log($"tst_astra.end. get[{nRes}]");
        }
        catch (Exception ex) 
        {
            CRwrapper.Log($"catch exception: {ex}");
        }
        CRwrapper.Log($"That's all folks! Press a key to exit.");
        Console.ReadKey();
    }
}
