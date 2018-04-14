using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net.Sockets;

using UnityEngine;

namespace Assets.Class
{
    public class NetworkSocket
    {
        public String host = "127.0.0.1";
        public Int32 port = 2022;

        TcpClient tcpSocket;
        NetworkStream netStream;

        StreamWriter socketWriter;
        StreamReader socketReader;

        Boolean isSetupSocket = false;
        Boolean isCloseSocket = false;

        private static NetworkSocket instance;

        private NetworkSocket()
        {
            var data = new JsonDataClass();
            data.name = "Park";
            data.level = 10;
            data.likes = new List<string>()
            {
                "dog", "cat"
            };

            Debug.Log(JsonUtility.ToJson(data, true));
        }

        public static NetworkSocket Instance
        {
            get
            {
                if( instance == null )
                {
                    instance = new NetworkSocket();
                }

                return instance;
            }
        }

        public void ChangeHost( String host, Int32 port )
        {
            if( this.isSetupSocket == true )
            {
                Debug.Log("ChangeHost must run before SetupSocket function");
                return;
            }

            this.host = host;
            this.port = port;
        }

        public void SetupSocket()
        {
            try
            {
                if( this.isSetupSocket == true )
                {
                    Debug.Log("SetupSocket is already run");
                    return;
                }

                this.isSetupSocket = true;

                this.tcpSocket = new TcpClient(this.host, this.port);

                this.netStream = tcpSocket.GetStream();

                this.socketWriter = new StreamWriter(this.netStream);
                this.socketReader = new StreamReader(this.netStream);
            }
            catch( Exception e )
            {
                // Something went wrong
                Debug.Log("Socket error: " + e);
            }
        }

        public void SendSocket(String line)
        {
            if( this.isCloseSocket == true )
            {
                return;
            }

            line += "\r\n";
            this.socketWriter.Write(line);
            this.socketWriter.Flush();
        }

        public String RecvSocket()
        {
            if( this.isCloseSocket == true )
            {
                return "";
            }

            if( this.netStream.DataAvailable)
            {
                return this.socketReader.ReadLine();
            }

            return "";
        }

        public void CloseSocket()
        {
            if (this.isCloseSocket == true)
            {
                return;
            }

            socketWriter.Close();
            socketReader.Close();
            tcpSocket.Close();
            this.isCloseSocket = true;
        }
    }
}
