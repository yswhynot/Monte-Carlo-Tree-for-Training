#using <System.dll>

using namespace System;
using namespace System::IO::Ports;
using namespace System::Threading;

public ref class PortChat
{
private:
	static bool _continue;
	static SerialPort^ _serialPort;

public:
	PortChat()
	{
		// Create a new SerialPort object with default settings.
		_serialPort = gcnew SerialPort();

		// Allow the user to set the appropriate properties.
		_serialPort->PortName = SetPortName(_serialPort->PortName);
		_serialPort->BaudRate = 19200;

		// Set read timeout
		_serialPort->ReadTimeout = 2000;

		_serialPort->Open();
	}

	~PortChat() {
		_serialPort->Close();
	}

	void Write(String^ cmd) {
		_serialPort->WriteLine(cmd);
		//Console::WriteLine(String::Format("-> {0}", cmd));
	}

	String^ Read(bool* hasRead)
	{
		try
		{
			String^ message = _serialPort->ReadLine();
			//Console::WriteLine(String::Format("<- {0}", message));
			*hasRead = true;
			return message;
		}
		catch (TimeoutException ^) {
			*hasRead = false;
			return "";
		}
	}

	static String^ SetPortName(String^ defaultPortName)
	{
		String^ portName;

		Console::WriteLine("Available Ports:");
		for each (String^ s in SerialPort::GetPortNames())
		{
			Console::WriteLine("   {0}", s);
		}

		Console::Write("Enter COM port value (Default: {0}): ", defaultPortName);
		portName = Console::ReadLine();

		if (portName == "")
		{
			portName = defaultPortName;
		}
		return portName;
	}
};