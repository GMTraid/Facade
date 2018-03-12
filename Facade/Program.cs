using System.ServiceModel;
using System.ServiceModel.Web;
using System.Net;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace Facade
{
    [ServiceContract] // preprocess indexer 
    public interface IComputation // initialization of the entry point 
    {
        [OperationContract, WebInvoke(UriTemplate = "/calculate", Method = "POST", BodyStyle = WebMessageBodyStyle.Wrapped, RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        double calculate(double x, double y);
    }

    public class Calculator : IComputation // implementation of the function web service
    {
        public double calculate(double x, double y)
        {
            return x + y;
        }
   
    }


    public class AsyncHttpServer : IDisposable //data transfer protocol
    {
        private readonly HttpListener listener; //object of the transfer
        private Thread listenerThread; 
        private volatile bool isRunning; //asynchronous indexer
        private bool disposed; // release of resources (DRAM -  Dynamic Random Access Memory)

        public AsyncHttpServer()
        {
            listener = new HttpListener();
        }

        public void Start(string prefix) // uri - Universal Resource Identifier 
        {
            lock (listener)
            {
                if (!isRunning)
                {
                    listener.Prefixes.Clear();
                    listener.Prefixes.Add(prefix);
                    listener.Start();

                    listenerThread = new Thread(Listen) // the thread is a background 
                    {
                        IsBackground = true, 
                        Priority = ThreadPriority.Highest
                    };
                    listenerThread.Start();

                    isRunning = true;
                }
            }
        }

        public void Stop()
        {
            lock (listener)
            {
                if (!isRunning)// should always return the true value
                    return;

                listener.Stop();

                listenerThread.Abort();// interruption of the flow
                listenerThread.Join(); //Blocks the calling thread until a thread terminates, represented by an instance

                 isRunning = false;
            }
        }

        public void Dispose()// usual implementation of the IDispose interface
        {
            if (disposed)
                return;

            disposed = true;

            Stop();

            listener.Close();
        }

        private void HandleContext(HttpListenerContext listenerContext) // Terms of POST processing
        {
            listenerContext.Response.StatusCode = (int)HttpStatusCode.OK;
        }

        private void Listen() // HTTP Listener ( processes an incoming request on the server ) 
        {
            while (true)
            {
                try
                {
                    if (listener.IsListening)
                    {
                        var context = listener.GetContext();
                        Task.Run(() => HandleContext(context));// transfer protocol and receives the uri/prefix of the server
                    }
                    else Thread.Sleep(0);//suspends a thread for a specified time
                }
                catch (ThreadAbortException)
                {
                    return;
                }
            }
        }

    }
   
    class Program
    {

            static void Main(string[] args)
        {
            string baseAddres = "http://localhost:8080/Calculator"; // URL server address
            WebServiceHost myHost = new WebServiceHost(typeof(Calculator),new Uri(baseAddres)); // data parsing of http request body
            Console.WriteLine("starting service ... ");
            try
            {
                myHost.Open();//opens the connection port
                Console.WriteLine("press any key to close the service ...");
                Console.ReadLine();
                myHost.Close();
            }
            catch(FormatException e)
            {
                Console.WriteLine("Exception" + e);
            }        
        }
    }
}
