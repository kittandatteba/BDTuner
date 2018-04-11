Option Explicit

Dim inFile, outFile
Dim inStream, outStream
Dim buf
Dim txt
Dim i, j

inFile=".\target\bitbang.iic"
outFile="..\..\src\CPLDProgrammer\FX2FarmwareBitBang.inc"

Set outStream=CreateObject("ADODB.Stream")
Set inStream=CreateObject("ADODB.Stream")

outStream.Type=2
outStream.Charset="shift_jis"
outStream.Open

inStream.Type=1
inStream.Open
inStream.LoadFromFile inFile
For i=0 To inStream.Size-1 Step 16
	buf=inStream.Read(16)
	txt=""
	For j=0 To 15
		If i+j<inStream.Size-1 then
			txt=txt & "0x" & mid(hex(&h100+ascb(midb(buf,j+1,1))),2) & ","
		Else
			txt=txt & "0x" & mid(hex(&h100+ascb(midb(buf,j+1,1))),2)
			Exit For
		End If
	Next
	outStream.WriteText txt & VbCrLf, 0
Next
inStream.Close

outStream.SaveToFile outFile, 2
outStream.Close

Set outStream=Nothing
Set inStream=Nothing

MsgBox "output file : " & outFile
