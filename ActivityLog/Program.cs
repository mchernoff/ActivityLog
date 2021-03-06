﻿using Prometheus;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace MattAppCS
{
	class Program
	{
		public static string data = null;
		private static readonly Counter KeyDownCount = Metrics
			.CreateCounter("total_key_downs", "Number of key down presses");
		private static readonly Counter MouseClickCount = Metrics
			.CreateCounter("total_mouse_clicks", "Number of mouse key down presses (left, right, middle)");

		const string keyCode = "kdown";
		const string mouseCode = "mclick";

		static void Main(string[] args)
		{
			var metricServer = new MetricServer(port: 1234);
			metricServer.Start();

			byte[] bytes = new Byte[1024];
			IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
			IPAddress ipAddress = ipHostInfo.AddressList[0];
			IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 11000);
			Socket listener = new Socket(ipAddress.AddressFamily,
				SocketType.Stream, ProtocolType.Tcp);


			try
			{
				listener.Bind(localEndPoint);
				listener.Listen(10);

				// Start listening for connections.  
				while (true)
				{
					Console.WriteLine("Waiting for a connection...");

					var currentFolder = Directory.GetCurrentDirectory();
					string daemonFile = string.Format($"{currentFolder}\\..\\..\\..\\Release\\CWinHook.exe");
					Console.WriteLine(daemonFile);
					try
					{
						Process.Start(daemonFile);
					}
					catch (FileNotFoundException)
					{
						daemonFile = string.Format($"{currentFolder}\\..\\..\\..\\Debug\\CWinHook.exe");
						Process.Start(daemonFile);
					}

					// Program is suspended while waiting for an incoming connection.  
					Socket handler = listener.Accept();
					data = null;
					Console.WriteLine("Connected");

					// An incoming connection needs to be processed.  
					while (true)
					{
						int bytesRec = handler.Receive(bytes);
						data = Encoding.ASCII.GetString(bytes, 0, bytesRec);
						if (data.Equals(keyCode))
						{
							KeyDownCount.Inc();
						}
						else if (data.Equals(mouseCode))
						{
							MouseClickCount.Inc();
						}
					}

					// Show the data on the console.  
					Console.WriteLine("Text received : {0}", data);

					// Echo the data back to the client.  
					byte[] msg = Encoding.ASCII.GetBytes(data);

					handler.Send(msg);
					handler.Shutdown(SocketShutdown.Both);
					handler.Close();
				}

			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
			}

			Console.WriteLine("\nPress ENTER to continue...");
			Console.Read();
		}
	}
}
