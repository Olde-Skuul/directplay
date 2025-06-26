# Microsoft DirectX 9.0 SDK Update (Summer 2003) DP8Sim Utility

The ``DP8Sim`` utility is an executable that uses the methods of ``IDP8SimControl`` to allow you to test your application under a variety of network conditions. Alternatively, you can use the DP8Sim service provider and **IDP8SimControl** interface to create your own test environment. For more information, see Testing Network Performance in the documentation.

```text
Note: ``DP8Sim`` is implemented on top of the existing DirectPlay8 TCP/IP Service Provider. The settings are also applied on top of the existing network characteristics. Therefore, it is intended to be used on a high-speed local area network (LAN) where normal latency and packet loss are negligible.
```

## Path

Executable: dp8simui.exe

DLL: dp8sim.dll

## User's Guide

The configuration utility, ``Dp8simui.exe``, is the user interface for controlling ``Dp8sim.dll``. Both files must reside in the same directory. When you start the configuration utility, it will automatically register the ``Dp8sim.dll`` Component Object Model (COM) objects. You can also manually register the dynamic-link library (DLL) by typing the following command at your Microsoft® MS-DOS® prompt.

```bash
regsvr32.exe dp8sim.dll
```

After the ``Dp8sim.dll`` is registered, the DP8Sim service provider will be available. Select the "DirectPlay8 TCP/IP Service Provider (Network Simulator)" service provider from the service providers returned by the ``IDirectPlay8Peer::EnumServiceProviders``, ``IDirectPlay8Client::EnumServiceProviders``, and ``IDirectPlay8Server::EnumServiceProviders`` methods. Note that Microsoft DirectPlay® sessions cannot use the network simulator if the DirectPlay interface was created before running the simulator.

Once the simulator is running, you can control various network options. There are also predefined network settings that may be useful.

The options for sending (receiving) are:

- *Bandwidth* (bytes/second). The total available outbound (inbound) bandwidth for all players in bytes per second. All packets have their latency increased in proportion to their size according to this value. If the application sends (receives) more than this amount, later packets are queued behind earlier ones. Use 0 (zero) for unlimited bandwidth, up to the real underlying network bandwidth.
- *Drop percentage* (0 to 100 percent). The random frequency as a percentage for an individual outbound (inbound) packet to be dropped. Each packet stands the same chance of being dropped, regardless of other packets. Note that this does not necessarily model the behavior of all networks. Packet loss on the Internet, for example, tends to be erratic, with packet loss high at some times and low at others. A value of 1 means that an average of 1 out of every 100 packets is dropped. A value of 100 means every packet is dropped. Use 0 if you do not want to drop any packets other than loss due to the real underlying network.
- *Minimum latency* (milliseconds). The minimum delay in milliseconds for outbound (inbound) packets. The actual delay for an individual packet is chosen randomly between this minimum value and the maximum latency value. Note that the delay is applied on top of any delay imposed by bandwidth limitations. Use 0 if you do not want to have a lower bound for artificial latency beyond the real underlying network.
- *Maximum latency* (milliseconds). The maximum delay in milliseconds for outbound (inbound) packets. The actual delay for an individual packet is chosen randomly between the minimum latency value and this maximum value. If this value is lower than the minimum latency value, it is automatically set to equal the minimum value. Note that the delay is applied on top of any delay imposed by bandwidth limitations. Use 0 if you do not want to have an upper bound for artificial latency beyond the real underlying network.

```text
Note: These options apply for in-game data only. Host enumerations and responses are not subject to the simulation.
```

To make modifications, use the **Apply** and **Revert** buttons. Click **Apply** to change the settings and **Revert** to restore the previous settings. To save you settings, click the **Save As** button. You will be prompted to give a name for your settings.

At the bottom of the configuration utility window, the send and receive statistics for all affected DirectPlay interfaces are displayed. The **Refresh** button updates the statistics and **Clear** resets the statistics to 0.

To unregister Dp8sim.dll, type the following at the MS-DOS command prompt.

```bash
regsvr32.exe /u dp8sim.dll
```

```text
Note: The DP8Sim utility provided with Microsoft DirectX® 8.1 will not work with applications using the DirectX 9.0 DLLs.
```

For more information about network performance, see Optimizing Network Usage.

© 2003 Microsoft Corporation. All rights reserved. 