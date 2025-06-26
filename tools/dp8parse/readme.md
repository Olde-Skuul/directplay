# Monitoring DirectPlay Network Traffic with Network Monitor

During game development, you might find it useful to monitor Microsoft® DirectPlay® network traffic, especially when trying to understand bugs. The **Network Monitor** is a standard utility for analyzing network traffic. DirectPlay includes a set of parsers, that allow you to use the **Network Monitor** to analyze four components of DirectPlay messaging: the service provider layer, the transport layer, the session layer, and the voice layer.

## How Network Monitor Works With DirectPlay

The DirectPlay protocol stack has three basic layers.

- The voice and session layers share the top level of the stack. Normal messaging passes through the session layer, and voice-related messaging passes through the voice layer.
- The transport layer is the middle of the stack. Both voice and session traffic passes through this layer, which is responsible for such tasks as fragmentation and reassembly of messages and retransmission of lost packets.
- The service provider layer is at the bottom of the stack. All messaging is handled by this layer, which is responsible for communicating with the network. For example, for Transmission Control Protocol/Internet Protocol (TCP/IP) networking, the service provider uses the Windows Sockets (Winsock) application programming interface (API) to communicate with the network stack. The **Network Monitor** can only parse network traffic that is carried on an Internet Protocol (IP) or Internetwork Packet Exchange (IPX) service provider.

By installing the DirectPlay parsers, you can use the **Network Monitor** to analyze the network traffic as it passes through any of these four layers. You can see all DirectPlay traffic by selecting the service provider parser. However, by selecting one of the higher-level parsers, you can filter out traffic that might not be of interest.

With the transport layer parser, you see all voice and session traffic, but not low-level traffic such as connection handshaking. Be aware that the transport layer breaks messages that are longer than the network's Maximum Transmission Unit (MTU) into one or more fragments.

The session and voice layer parsers enable you to analyze session and voice-related traffic separately. Both of these parsers are can detect fragmentation, and notify the user, but cannot parse fragmented packets.

## Configuring Network Monitor for DirectPlay

If you have a Microsoft Windows® 2000 Server system, **Network Monitor** is already installed. For Windows 2000 Professional, you must purchase a copy of Systems Management Server (SMS). For a general discussion of how to use **Network Monitor**, see About Network Monitor 2.0.

To configure the **Network Monitor** to handle DirectPlay traffic:

1. Copy Dp8parse.dll to the appropriate folder. The **Network Monitor** root folder is normally installed in the \Winnt\System32 folder. If you have installed SMS, the root folder will be called NetMonFull. For Windows 2000 Server, the root folder will be called NetMon. Depending on which version of the **Network Monitor** you are using, copy the parser dynamic-link library (DLL) to either the ...\NetMonFull\Parsers, or ...\NetMon\parsers folder.
2. Start the **Network Monitor**.
3. Set the adapter to capture from (Capture, Networks, Local Computer). Be sure to choose the adapter with the "Dial-up Connection" property set to FALSE.

You are now ready to start capturing traffic.

## Capturing DirectPlay Network Traffic

To start the capture process, click **Start Capture** on the **Network Monitor** toolbar to open the capture view. Initially, you will see all the traffic that is passing through your adapter. You can filter that raw traffic stream to focus on only those packets that are of interest. By installing the DirectPlay parsers, you essentially add four DirectPlay-oriented filters to **Network Monitor** that enable you to filter everything but DirectPlay traffic from your capture view.

To select a filter:

1. Click the **Edit Display Filter** button on the **Network Monitor** toolbar.
2. Double-click **Protocol** == **Any**.
3. Click **Disable All**.
4. Under **Disabled Protocols**, double-click **DPLAYSESSION**, **DPLAYSP**, **DPLAYTRANSPORT**, and **DPLAYVOICE**.

Click **OK** twice to return to the capture view, and you are ready to start viewing DirectPlay traffic.

You can also apply a filter to the capture process itself, rather than to the capture view. This allows you, for instance, to capture only IP packets with specified source and destination ports. For details, see the **Network Monitor** documentation.

## Tips for Using Network Monitor with DirectPlay

Here are a few tips to using the **Network Monitor** with DirectPlay.

- By default, the **Network Monitor** captures only 1 MB of the most recent traffic. You will probably want to increase this value to at least 10 to 20 MB.
- The **Network Monitor** doesn't stream to the hard drive, so all you can see is what is in the capture buffer. To stream captured traffic to a hard drive, you need to implement your own capturer. For details, see MSDN®.
- By default, DirectPlay parsing uses the [2302,2400]U{6073} port/socket range to filter IP and IPX packets. To parse ports other than the default DirectPlay ports, create two new **DWORD** values under the **HKEY_CURRENT_USER\Software\Microsoft\DirectPlay\Parsers** key, as shown in the following example.

```text
MinUserPort = x

MaxUserPort = y
```

The x- and y-data values define the range to parse in addition to the default DirectPlay ports. They can be the same value if you only need one custom port.

- DirectPlay parsers support both signed and unsigned traffic. By default, the parsers assume that packets are unsigned. To enable monitoring of signed packets, set the **DWORD** value under the **HKEY_CURRENT_USER\Software\Microsoft\DirectPlay\Parsers** key, as shown in the following example.

```text
AssumeSigned = 1
```

- Because the DirectPlay and Real-time Transport Protocol (RTP) are both layered on top of the User Datagram Protocol (UDP), their parsers might conflict. You should disable the RTP parser when analyzing DirectPlay traffic, and vice versa.

© 2003 Microsoft Corporation. All rights reserved.
