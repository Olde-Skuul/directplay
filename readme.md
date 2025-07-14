# **DirectPlay**: The Definitive Edition

**DirectPlay** is stored here for historical reasons. It should never be used for future applications. **Burgerlib** has APIs that will detect, load and call **DirectPlay** functions in order to be compatible with older codebases.

Included are the **DirectPlay** headers, libraries, binaries for tools, and sample code. Even ``DirectPlayVoice`` is included.

## Samples

In the ``Samples`` folder, are samples extracted and updated for modern compilers. Every sample has project files for Visual Studio 2003, Visual Studio 2022, CodeWarrior for Windows, and Open Watcom 1.9. If the folder's name ends with an ``8``, then the sample is for DirectX 8 or later. Otherwise, the sample is for **DirectPlay** 3 or 4.

## Don't use **DirectPlay** for any new applications

You've been warned. Also, only a 32 bit Intel implementation exists of **DirectPlay**. Even though a 64 bit library was provided by the source SDK, to date, no 64 bit Intel runtime has been released by Microsoft. ``DirectPlayVoice`` is not available on Windows 8 or higher, so all code that relies on ``DirectPlayVoice`` will not run at all on Windows 8 or higher.

## Source of the SDK

The SDK is obtained directly from the DirectX SDK (August 2007) with only minor modifications to remove compiler warnings

```text
DirectX SDK (August 2007)
http://www.microsoft.com/en-us/download/details.aspx?id=13287 (Link no longer valid)
https://archive.org/details/dxsdk_aug2007 (Current link)

File : dxsdk_aug2007.exe
CRC32: 4EAF2D00
MD5  : E866E58A5CBFC98B3880261B5AE78529
SHA-1: C812C18E2972BDB1D9CBB544BE9CED9370A4656F
```
