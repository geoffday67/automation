using System;
using System.Collections.Generic;
using System.Text;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

namespace X10Test
{
	class X10: IDisposable
	{
		SerialPort Port;
		Object SerialSync;
		bool SerialCaptured;
		ListBox Status;

		public enum X10Function { Unknown = 0x00, On = 0x02, Off = 0x03 };
		byte[] X10Code = new byte[] { 0x06, 0x0E, 0x02, 0x0A, 0x01, 0x09, 0x05, 0x0D, 0x07, 0x0F, 0x03, 0x0B, 0x00, 0x08, 0x04, 0x0C };

		public X10(ListBox status)
		{
			Status = status;

			SerialSync = new Object();
			CaptureSerial(false);

			Port = new SerialPort("COM1");
			Port.BaudRate = 4800;
			Port.Parity = Parity.None;
			Port.DataBits = 8;
			Port.StopBits = StopBits.One;
			Port.ReadTimeout = 3000;
			Port.DataReceived += new SerialDataReceivedEventHandler(Port_DataReceived);
			Port.Open();
		}

		void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
		{
			lock (SerialSync)
			{
				if (SerialCaptured)
					return;

				int n = Port.ReadByte();

				switch (n)
				{
					case 0xA5:
						byte[] datetime = new byte[] { 0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
						Port.Write(datetime, 0, datetime.Length);
						Port.ReadByte();
						break;
				}
			}
		}

		public void SendFunction(string target, X10Function function)
		{
			int house = (int)target[0] - (int)'A';
			if (house < 0 || house > 15)
				throw new Exception(string.Format("Invalid house code {0}", house));

			int device = (int)target[1] - (int)'1';
			if (device < 0 || device > 15)
				throw new Exception(string.Format("Invalid device code {0}", device));

			try
			{
				CaptureSerial(true);

				byte[] buffer = new byte[2];

				buffer[0] = 0x04;
				buffer[1] = (byte)((X10Code[house] << 4) | X10Code[device]);
				Port.Write(buffer, 0, 2);
				int b = Port.ReadByte();
				Status.Items.Add("Byte = " + b.ToString());

				buffer[0] = 0x00;
				Port.Write(buffer, 0, 1);
				Port.ReadByte();

				buffer[0] = 0x06;
				buffer[1] = (byte)((X10Code[house] << 4) | (byte)function);
				Port.Write(buffer, 0, 2);
				Port.ReadByte();

				buffer[0] = 0x00;
				Port.Write(buffer, 0, 1);
				Port.ReadByte();
			}

			catch (Exception x)
			{
				Status.Items.Add(x.Message);
			}

			finally
			{
				CaptureSerial(false);
			}
		}

		void CaptureSerial(bool set)
		{
			lock (SerialSync)
			{
				SerialCaptured = set;
			}
		}

		#region IDisposable Members

		public void Dispose()
		{
			Port.Dispose();
		}

		#endregion
	}
}
