unit updiguimain;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Buttons,
  EditBtn, Grids, ExtCtrls, ComCtrls,
{$IFNDEF MSWINDOWS}
  {$IFNDEF NO_LIBC}
  Libc,
  KernelIoctl,
  {$ELSE}
  termio, baseunix, unix,
  {$ENDIF}
  {$IFNDEF FPC}
  Types,
  {$ENDIF}
{$ELSE}
  Windows, registry,
  {$IFDEF FPC}
  winver,
  {$ENDIF}
{$ENDIF}
  libUPDI,ctypes, Types;

type

  TOnLogEvent = procedure (const aStr : String) of object;

  { TUPDIWorkingThread }

  TUPDIWorkingThread = class(TThread)
  private
    Fcfg : pUPDI_Params;
    FOnLog : TOnLogEvent;
    FOnFinish : TThreadMethod;
    FSuccess: Boolean;
  public
    constructor Create(aCfg : pUPDI_Params; aOnFinish : TThreadMethod);
    procedure Execute; override;

    procedure LogOut(const Str : String);

    property cfg : pUPDI_Params read Fcfg;
    property Success : Boolean read FSuccess write FSuccess;
    property OnLog : TOnLogEvent read FOnLog write FOnLog;
  end;

  { TForm1 }

  TForm1 = class(TForm)
    BaudComboBox: TComboBox;
    DeviceInfoButton: TSpeedButton;
    DevicesComboBox: TComboBox;
    DeviceSNText: TEdit;
    DevIdText: TEdit;
    EraseDeviceButton: TSpeedButton;
    EraseDeviceCheckBox: TCheckBox;
    FlashButton: TButton;
    FusesBox: TGroupBox;
    FusesGrid: TStringGrid;
    ImageList1: TImageList;
    InputHEXFile: TFileNameEdit;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Log: TMemo;
    MemoryBox: TGroupBox;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    PortComboBox: TComboBox;
    ProgressBar1: TProgressBar;
    ReadButton: TButton;
    ReadFuses: TButton;
    RefreshTTYButton: TSpeedButton;
    Timer1: TTimer;
    VerifyButton: TButton;
    VerifyDeviceCheckBox: TCheckBox;
    WriteFuses: TButton;
    procedure Button1Click(Sender: TObject);
    procedure BaudComboBoxSelect(Sender: TObject);
    procedure DeviceInfoButtonClick(Sender: TObject);
    procedure DevicesComboBoxChange(Sender: TObject);
    procedure DevicesComboBoxSelect(Sender: TObject);
    procedure EraseDeviceButtonClick(Sender : TObject);
    procedure FlashButtonClick(Sender : TObject);
    procedure FusesGridDrawCell(Sender : TObject; aCol, aRow : Integer;
      aRect : TRect; aState : TGridDrawState);
    procedure FusesGridPrepareCanvas(Sender : TObject; aCol, aRow : Integer;
      aState : TGridDrawState);
    procedure InputHEXFileChange(Sender : TObject);
    procedure PortComboBoxChange(Sender : TObject);
    procedure PortComboBoxSelect(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure VerifyButtonClick(Sender: TObject);
    procedure VerifyDeviceCheckBoxChange(Sender : TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure ReadButtonClick(Sender : TObject);
    procedure ReadFusesClick(Sender : TObject);
    procedure RefreshTTYButtonClick(Sender : TObject);
    procedure WriteFusesClick(Sender : TObject);
  private
    loclogger : pUPDI_logger;
    locprgs   : pUPDI_progress;
    cfg       : pUPDI_Params;
    wrk       : TUPDIWorkingThread;
    logmutex  : TRTLCriticalSection;
    prgsmutex : TRTLCriticalSection;
    inprogress: Boolean;
    prgtotal, prgpos : integer;
    loglist   : TStringList;

    procedure LockLog;
    procedure UnLockLog;
    procedure LockProgress;
    procedure UnLockProgress;

    procedure OnStopUPDIThread;
    procedure OnReadWriteFuses;
    procedure OnReadDevInfoFuses;
  public
    procedure LaunchUPDIThread(awrk : TUPDIWorkingThread);
    function  FuseState(pos : integer) : Byte;
    procedure ConsumeFuses(fuses : pUPDI_fuse; cnt : integer);
    procedure ConsumeDevInfo(devinfo : pByte);
    procedure RefreshPortsList;
    procedure LogOut(const Str : String);
    procedure ProgressStart(total : uint16);
    procedure ProgressStep(pos, total : uint16);
    procedure ProgressFinish(pos, total : uint16);
  end;

  { TUPDIReadDevInfoThread }

  TUPDIReadDevInfoThread = class(TUPDIWorkingThread)
  public
    devinfo : array [0..DEV_INFO_LEN-1] of Byte;
    procedure Execute; override;
  end;

  { TUPDIReadFusesThread }

  TUPDIReadFusesThread = class(TUPDIWorkingThread)
  public
    fuses : pUPDI_fuse;
    cnt : integer;
    procedure Execute; override;
    destructor Destroy; override;
  end;

  { TUPDIWriteFusesThread }

  TUPDIWriteFusesThread = class(TUPDIReadFusesThread)
  private
    seq : pUPDI_seq;
  public
    constructor Create(aCfg : pUPDI_Params; aOnFinish : TThreadMethod;
                            aseq : pUPDI_seq;
                            afuses : pUPDI_fuse;
                            acnt : integer);
    procedure Execute; override;
    destructor Destroy; override;
  end;

  { TUPDIReadFlashThread }

  TUPDIReadFlashThread = class(TUPDIWorkingThread)
  private
    FN : String;
  public
    constructor Create(aCfg : pUPDI_Params; aOnFinish : TThreadMethod;
                              const aFN : String);
    procedure Execute; override;
  end;

  { TUPDIWriteFlashThread }

  TUPDIWriteFlashThread = class(TUPDIReadFlashThread)
  private
    Erase, Verify : Boolean;
  public
    constructor Create(aCfg: pUPDI_Params; aOnFinish: TThreadMethod;
                             const aFN: String; aerase, averify: boolean);
    procedure Execute; override;
  end;

  { TUPDIVerifyFlashThread }

  TUPDIVerifyFlashThread = class(TUPDIReadFlashThread)
  public
    procedure Execute; override;
  end;

var
  Form1: TForm1;

resourcestring
  sOKString = 'OK';
  sYouShouldSpecifyCorr = 'You should specify correct file name';
  sWritingHexDataToFile = 'Writing hex data to %s';
  sReadingHexDataFromFile = 'Reading hex data from %s';
  sFileSIsNotExists = 'File %s is not exists';
  sFileSExistsRewrite = 'File %s exists. Rewrite?';
  sHEXFileIsEmpty = 'HEX file is empty';
  sNotEnoughtMemoryCant = 'Not enought memory. Cant allocate buffer';
  sVerifyOK = 'Verify OK';
  sVerifyFail = 'Verify Fail';

implementation

const
  cDEV_FLASH_SIZE_MAX = $100000;

{$R *.lfm}


{ this part of code
  taken from     TLazSerial/lazsynaser.pas
  https://github.com/JurassicPork/TLazSerial
}
{$IFDEF MSWINDOWS}
function GetSerialPortNames: string;
var
  reg: TRegistry;
  l, v: TStringList;
  n: integer;
begin
  l := TStringList.Create;
  v := TStringList.Create;
  reg := TRegistry.Create;

  try
{$IFNDEF VER100}
{$IFNDEF VER120}
    reg.Access := KEY_READ;
{$ENDIF}
{$ENDIF}
    reg.RootKey := HKEY_LOCAL_MACHINE;
    reg.OpenKey('\HARDWARE\DEVICEMAP\SERIALCOMM\', false);
    reg.GetValueNames(l);
    for n := 0 to l.Count - 1 do
// Modif J.P  03/2013
      v.Add(Pchar(reg.ReadString(l[n])));
    Result := v.Text ;
  finally
    reg.Free;
    l.Free;
    v.Free;
  end;
end;
{$ENDIF}
{$IFNDEF MSWINDOWS}

{$ifndef DARWIN}
type
   TSerialStruct = packed record
          typ: cint;
          line: cint;
          port: cuint;
          irq:  cint;
          flags: cint;
          xmit_fifo_size: cint;
          custom_divisor: cint;
          baud_base: cint;
          close_delay: cushort;
          io_type: cchar;
          reserved_char:  pcchar;
          hub6: cint;
          closing_wait: cushort; // time to wait before closing
          closing_wait2: cushort; // no longer used...
          iomem_base: pcchar;
          iomem_reg_shift: cushort;
          port_high: clong; // cookie passed into ioremap
   end;
{$endif}

// Modif J.P   03/2013 - O1/2017 - 11/2022
function GetSerialPortNames: string;
var
  Index: Integer;
  Data: string;
  TmpPorts: String;
  flags : Longint;
  sr : TSearchRec;
// J.P  01/2017  new boolean parameter : special
  procedure ScanForPorts( const ThisRootStr : string; special :  boolean); // added by PDF
  var theDevice : String;
  var FD : Cint;
{$IFnDEF DARWIN}        // RPH - Added 14May2016
   var Ser : TSerialStruct;
{$ENDIF}
  begin
    if FindFirst( ThisRootStr, flags, sr) = 0 then
    begin
      repeat
        if (sr.Attr and flags) = Sr.Attr then
        begin
          data := sr.Name;
          index := length(data);
          theDevice := '/dev/' + data;
// try to open the device
       FD := fpopen(thedevice,O_RdWr or O_NonBlock or O_NoCtty);
       if FD > 0 then
          begin
// try to get serial info from the device
          {$IFDEF DARWIN}       // RPH - Added 14May2016 for OS-X
           if fpioctl( FD,TIOCEXCL, nil) <> -1  then
             begin
              TmpPorts := TmpPorts + sLineBreak + theDevice;
              fpclose(FD);
             end;
          {$ELSE}
           if fpioctl( FD,TIOCGSERIAL, @Ser) <> -1 then
             begin
// device is serial if type is not unknown (if not special device)
              // new parameter special
              if ((Ser.typ <> 0) OR (special) ) then
               TmpPorts := TmpPorts + sLineBreak + theDevice;
              fpclose(FD);
             end;
          {$ENDIF}
          end;
        end;
      until FindNext(sr) <> 0;
    FindClose(sr);
    end;
  end;

begin
  try
    TmpPorts := '';
    flags := faAnyFile AND (NOT faDirectory);
    ScanForPorts( '/dev/rfcomm*',true);
    ScanForPorts( '/dev/ttyUSB*',true);
    ScanForPorts( '/dev/ttyS*',false);
    ScanForPorts( '/dev/ttyACM*',true);
   {$IFDEF DARWIN}
    ScanForPorts( '/dev/tty.usbserial*',false); // RPH 14May2016, for FTDI driver
    ScanForPorts( '/dev/tty.UC-232*',false);    // RPH 15May2016, for Prolific driver
   {$ELSE}
     ScanForPorts( '/dev/ttyAM*',false); // for ARM board
   {$ENDIF}
  finally
    Result:=TmpPorts;
  end;
end;
{$ENDIF}

function CompareIntelHex(d1 : pointer; d1sz : int32;
                         d2 : pointer; d2sz : int32) : Boolean;

procedure SkipToStartCode(var c : PByte; var i : integer; sz : integer);
begin
  while (i < sz) do
  begin
    if chr(c^) = ':' then
      break;
    Inc(c);
    inc(i);
  end;
end;

function HexToByte(c : PByte) : Byte;
begin
  case char(c^) of
  '0'..'9': Result := c^ - ord('0');
  'A'..'F': Result := c^ - ord('A') + 10;
  end;
  Result := (Result shl 4) and $f0;
  inc(c);
  case char(c^) of
  '0'..'9': Result := Result or (c^ - ord('0'));
  'A'..'F': Result := Result or (c^ - ord('A') + 10);
  end;
end;

function HexToWord(c : PByte) : Word;
var
  b1, b2 : Byte;
begin
  b1 := HexToByte(c);
  inc(c, 2);
  b2 := HexToByte(c);
  Result := (b1 shl 8) and $ff00 or b2
end;

function ConvertToRaw(src, dst : PByte; sz : Int32) : int32;
var
  b1   : Word;
  bc   : Word;
  addr : Word;
  rt   : Byte;
  i, j : int32;
  st   : Byte;

  dstloc : PByte;

  ok, finish : boolean;
begin
  Result := 0;
  i := 0;
  ok := true;
  finish := false;
  st := 0;
  while (i < sz) and ok and (not finish) do
  begin
    case st of
    0: begin
        SkipToStartCode(src, i, sz);
        inc(st);
      end;
    1: begin
        Inc(src);
        inc(i);
        Inc(st);
      end;
    2: begin
        ok := ((sz - i) >= 2);
        if ok then
        begin
          b1 := HexToByte(src);
          bc := b1 shl 1;
          Inc(src, 2);
          inc(i, 2);
          inc(st);
        end;
      end;
    3: begin
        ok := ((sz - i) >= 4);
        if ok then
        begin
          addr := HexToWord(src);
          Inc(src, 4);
          inc(i, 4);
          inc(st);
        end;
      end;
    4: begin
        ok := ((sz - i) >= 2);
        if ok then
        begin
          rt := HexToByte(src);
          Inc(src, 2);
          inc(i, 2);
          inc(st);
        end;
      end;
    5: begin
        ok := (sz - i) >= (bc + 2);
        if ok then
        begin
          if ok and (bc = 0) and (addr = 0) and (rt = 1) then
            finish := true
          else
          if rt = 0 then
          begin
            Result := addr + (bc shr 1);
            dstloc := @(dst[addr]);
            // copy data
            j := 0;
            while j < bc do
            begin
              dstloc^ := HexToByte(src);
              Inc(src, 2);
              Inc(j, 2);
              Inc(dstloc)
            end;
          end;

          inc(src, 2); // pass crc
          inc(i, bc + 2);
          st := 0;
        end;
      end;
    end;
  end;
end;

var
  c1 : PByte;
  c2 : PByte;

  sz1, sz2, s : Integer;
  buf1, buf2 : PByte;
begin
  c1 := PByte(d1);
  c2 := PByte(d2);

  buf1 := GetMem(cDEV_FLASH_SIZE_MAX);
  buf2 := GetMem(cDEV_FLASH_SIZE_MAX);
  try
    FillByte(buf1^, cDEV_FLASH_SIZE_MAX, $ff);
    FillByte(buf2^, cDEV_FLASH_SIZE_MAX, $ff);

    sz1 := ConvertToRaw(c1, buf1, d1sz);
    sz2 := ConvertToRaw(c2, buf2, d2sz);

    if sz1 > sz2 then
      s := sz2
    else
      s := sz1;

    Result := CompareByte(buf1^, buf2^, s) = 0;
  finally
    FreeMem(buf1);
    FreeMem(buf2);
  end;
end;

procedure log_onlog(ud: Pointer; level: int32; const src, msg: pansichar) cdecl;
begin
  TForm1(ud).LogOut(StrPas(src) + ': ' +StrPas(msg));
end;

procedure prgs_onstep(ud: pointer; pos, total:  uint16) cdecl;
begin
  TForm1(ud).ProgressStep(pos, total);
end;

procedure prgs_onstart(ud: pointer; total:  uint16) cdecl;
begin
  TForm1(ud).ProgressStart(total);
end;

procedure prgs_onstop(ud: pointer; pos, total: uint16) cdecl;
begin
  TForm1(ud).ProgressFinish(pos, total);
end;

{ TUPDIVerifyFlashThread }

procedure TUPDIVerifyFlashThread.Execute;
 var
   F : TFileStream;
   sz : Int32;
   buffer, rbuffer : Pointer;
   seq : Array [0..1] of UPDI_seq;
 begin
   try
     try
       F := TFileStream.Create(FN, fmOpenRead);
       try
         LogOut(Format(sReadingHexDataFromFile, [FN]));
         buffer := GetMem(cDEV_FLASH_SIZE_MAX);
         if assigned(buffer) then
         try
           sz := cDEV_FLASH_SIZE_MAX;
           sz := F.Read(buffer^, sz);

           if sz > 0 then
           begin
             LogOut(sOKString);

             seq[0].seq_type := UPDI_SEQ_ENTER_PM;

             rbuffer := GetMem(cDEV_FLASH_SIZE_MAX);
             with seq[1] do
             begin
               seq_type := UPDI_SEQ_READ;
               data := rbuffer;
               data_len := cDEV_FLASH_SIZE_MAX;
             end;

             try
               if UPDILIB_launch_seq(cfg, @(seq), 2) > 0 then
               begin
                 if CompareIntelHex(buffer,
                                    sz,
                                    seq[1].data,
                                    seq[1].data_len) then
                   LogOut(sVerifyOK)
                 else
                   LogOut(sVerifyFail);
               end else
                 Exit;
             finally
               if assigned(rbuffer) then
                 Freemem(rbuffer);
             end;
           end else
           begin
             LogOut(sHEXFileIsEmpty);
             Exit;
           end;
         finally
           Freemem(buffer);
         end else
           LogOut(sNotEnoughtMemoryCant);
       finally
         F.Free;
       end;
     except
       on E : Exception do
       begin
         LogOut(e.Message);
       end;
     end;
   finally
     if Assigned(FOnFinish) then
       Synchronize(FOnFinish);
   end;
end;

{ TUPDIReadDevInfoThread }

procedure TUPDIReadDevInfoThread.Execute;
begin
  try
    Success := UPDILIB_read_dev_info(cfg, @(devinfo[0])) > 0;
  finally
    inherited Execute;
  end;
end;

{ TUPDIWriteFlashThread }

constructor TUPDIWriteFlashThread.Create(aCfg: pUPDI_Params;
  aOnFinish: TThreadMethod; const aFN: String; aerase, averify : boolean);
begin
  inherited Create(aCfg, aOnFinish, aFN);
  Erase := aerase;
  Verify := averify;
end;

procedure TUPDIWriteFlashThread.Execute;
var
  F : TFileStream;
  sz : Int32;
  buffer, rbuffer : Pointer;
  seq : Array [0..3] of UPDI_seq;
  seq_len, rloc, wloc : integer;
begin
  try
    try
      F := TFileStream.Create(FN, fmOpenRead);
      try
        LogOut(Format(sReadingHexDataFromFile, [FN]));
        buffer := GetMem(cDEV_FLASH_SIZE_MAX);
        if assigned(buffer) then
        try
          sz := cDEV_FLASH_SIZE_MAX;
          sz := F.Read(buffer^, sz);

          if sz > 0 then
          begin
            LogOut(sOKString);

            seq_len := 1;
            seq[0].seq_type := UPDI_SEQ_ENTER_PM;

            if Erase then
            begin
              seq[1].seq_type := UPDI_SEQ_ERASE;
              inc(seq_len);
            end;

            with seq[seq_len] do
            begin
              seq_type := UPDI_SEQ_FLASH;
              data := buffer;
              data_len := sz;
            end;
            wloc := seq_len;
            inc(seq_len);

            if Verify then
            begin
              rbuffer := GetMem(cDEV_FLASH_SIZE_MAX);
              with seq[seq_len] do
              begin
                seq_type := UPDI_SEQ_READ;
                data := rbuffer;
                data_len := cDEV_FLASH_SIZE_MAX;
              end;
              rloc := seq_len;
              inc(seq_len);
            end else begin
              rloc := -1;
              rbuffer := nil;
            end;

            try
              if UPDILIB_launch_seq(cfg, @(seq), seq_len) > 0 then
              begin
                if Verify and (rloc > 0) then
                begin
                  if CompareIntelHex(seq[wloc].data,
                                     seq[wloc].data_len,
                                     seq[rloc].data,
                                     seq[rloc].data_len) then
                    LogOut(sVerifyOK)
                  else
                    LogOut(sVerifyFail);
                end;
              end else
                Exit;
            finally
              if assigned(rbuffer) then
                Freemem(rbuffer);
            end;
          end else
          begin
            LogOut(sHEXFileIsEmpty);
            Exit;
          end;
        finally
          Freemem(buffer);
        end else
          LogOut(sNotEnoughtMemoryCant);
      finally
        F.Free;
      end;
    except
      on E : Exception do
      begin
        LogOut(e.Message);
      end;
    end;
  finally
    if Assigned(FOnFinish) then
      Synchronize(FOnFinish);
  end;
end;

{ TUPDIWriteFusesThread }

constructor TUPDIWriteFusesThread.Create(aCfg: pUPDI_Params;
  aOnFinish: TThreadMethod; aseq: pUPDI_seq; afuses: pUPDI_fuse;
  acnt: integer);
begin
  inherited Create(aCfg, aOnFinish);
  Seq := aseq;
  fuses := afuses;
  cnt := acnt;
end;

procedure TUPDIWriteFusesThread.Execute;
begin
  try
    Success := UPDILIB_launch_seq(cfg, seq, 3) > 0;
  finally
    if Assigned(FOnFinish) then
      Synchronize(FOnFinish);
  end;
end;

destructor TUPDIWriteFusesThread.Destroy;
begin
  if assigned(seq) then
    FreeMem(seq);
  inherited Destroy;
end;

{ TUPDIReadFlashThread }

constructor TUPDIReadFlashThread.Create(aCfg: pUPDI_Params;
  aOnFinish: TThreadMethod; const aFN: String);
begin
  inherited Create(aCfg, aOnFinish);
  FN := aFN;
end;

procedure TUPDIReadFlashThread.Execute;
var
  F : TFileStream;
  sz : Int32;
  buffer : Pointer;
begin
  try
     buffer := GetMem(cDEV_FLASH_SIZE_MAX);
    try
      sz := cDEV_FLASH_SIZE_MAX;
      if UPDILIB_read_hex(cfg, PAnsiChar(buffer), @sz) = 0 then
        Exit;
      try
        LogOut(Format(sWritingHexDataToFile, [FN]));
        F := TFileStream.Create(FN, fmOpenWrite or fmCreate);
        try
          F.Write(buffer^, sz);
          Success := true;
        finally
          F.Free;
        end;
      except
        on E : Exception do
        begin
          LogOut(e.Message);
        end;
      end;
    finally
      Freemem(buffer);
    end;
  finally
    inherited Execute;
  end;
end;

{ TUPDIReadFusesThread }

procedure TUPDIReadFusesThread.Execute;
var
  i : integer;
begin
  fuses := nil;
  cnt := UPDILIB_devices_get_fuses_cnt(UPDILIB_cfg_get_device(cfg));
  try
    if cnt > 0 then
    begin
      fuses := pUPDI_fuse(Getmem(sizeof(UPDI_fuse) * cnt));
      if Assigned(fuses) then
      begin
        for i := 0 to cnt-1 do fuses[i].fuse := i;
        Success := UPDILIB_read_fuses(cfg, fuses, cnt) > 0;
      end;
    end;
  finally
    inherited Execute;
  end;
end;

destructor TUPDIReadFusesThread.Destroy;
begin
  if Assigned(fuses) then
    FreeMem(fuses);
  inherited Destroy;
end;

{ TUPDIWorkingThread }

constructor TUPDIWorkingThread.Create(aCfg: pUPDI_Params;
  aOnFinish: TThreadMethod);
begin
  inherited Create(true);
  FreeOnTerminate := false;
  Fcfg := aCfg;
  FOnFinish:= aOnFinish;
  FSuccess:= false;
end;

procedure TUPDIWorkingThread.Execute;
begin
  if Assigned(FOnFinish) then
    Synchronize(FOnFinish);
end;

procedure TUPDIWorkingThread.LogOut(const Str: String);
begin
  if Assigned(FOnLog) then
    FOnLog(Str);
end;

{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
var
  i, c : Integer;
  l : Integer;
  dst : AnsiString;
begin
  InitCriticalSection(logmutex);
  InitCriticalSection(prgsmutex);
  loglist := TStringList.Create;
  loclogger := nil;
  cfg := nil;

  if InitUPDILibInterface('') then
  begin
    UPDILIB_set_glb_logger_onlog(@log_onlog, Self);
    UPDILIB_set_glb_logger_level(LOG_LEVEL_INFO);

    loclogger := UPDILIB_logger_init(pansichar('gui'), LOG_LEVEL_INFO,
                                                       @log_onlog,
                                                       nil,
                                                       Self);

    locprgs := UPDILIB_progress_init(@prgs_onstart, @prgs_onstep,
                                                    @prgs_onstop, Self);

    SetLength(dst, COMPORT_LEN);
    cfg := UPDILIB_cfg_init();

    if Assigned(cfg) then
    begin
      UPDILIB_cfg_set_logger(cfg, loclogger);
      UPDILIB_cfg_set_progress(cfg, locprgs);
      i := UPDILIB_cfg_get_baudrate(cfg);
      BaudComboBox.Text := Inttostr(i);
    end;

    SetLength(dst, DEVICES_NAME_LEN);
    c := UPDILIB_devices_get_count();
    DevicesComboBox.Items.BeginUpdate;
    try
      for i := 0 to c-1 do
      begin
        l := DEVICES_NAME_LEN;
        UPDILIB_devices_get_name(i, @(dst[1]), @l);
        DevicesComboBox.Items.Add(Copy(dst, 1, l));
      end;
      if c > 0 then
        DevicesComboBox.ItemIndex := 0;
    finally
      DevicesComboBox.Items.EndUpdate;
    end;

    FusesGrid.ColWidths[0] := 32;
    FusesGrid.ColWidths[1] := 48;
    FusesGrid.ColWidths[2] := 48;

    FusesGrid.Cells[0,0] := '#';
    FusesGrid.Cells[1,0] := 'Cur';
    FusesGrid.Cells[2,0] := 'New';

    RefreshPortsList;
  end else
    LogOut(SFailedToLoadUPDILib);

  Timer1.Enabled := true;
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
  Timer1.Enabled := false;

  if assigned(wrk) then
  begin
    wrk.WaitFor;
    FreeAndNil(wrk);
  end;

  if assigned(loclogger) then
  begin
    UPDILIB_logger_done(loclogger);
    loclogger := nil;
  end;
  if Assigned(locprgs) then
  begin
    UPDILIB_progress_done(locprgs);
    locprgs := nil;
  end;
  if Assigned(cfg) then
  begin
    UPDILIB_cfg_done(cfg);
    cfg := nil;
  end;
  loglist.Free;

  DoneCriticalSection(logmutex);
  DoneCriticalSection(prgsmutex);

  if IsUPDILibloaded then
    DestroyUPDILibInterface;
end;

procedure TForm1.ReadButtonClick(Sender : TObject);
var
  c : Boolean;
begin
  if not Assigned(cfg) then Exit;


  if Length(InputHEXFile.FileName) = 0 then
  begin
    MessageDlg(sYouShouldSpecifyCorr, mtError, [mbOK], -1);
    exit;
  end;

  if FileExists(InputHEXFile.FileName) then
  begin
    c := MessageDlg(Format(sFileSExistsRewrite, [InputHEXFile.FileName]),
                  mtWarning, mbYesNo, -1) = mrYes;

  end else
    c := true;

  if c then
  begin
    LaunchUPDIThread(TUPDIReadFlashThread.Create(cfg,
                                  @OnStopUPDIThread, InputHEXFile.FileName));
  end;
end;

procedure TForm1.ReadFusesClick(Sender : TObject);
begin
  if Assigned(cfg) then
  begin
    FusesGrid.RowCount := 1;

    LaunchUPDIThread(TUPDIReadFusesThread.Create(cfg, @OnReadWriteFuses));
  end;
end;

procedure TForm1.RefreshTTYButtonClick(Sender : TObject);
begin
  RefreshPortsList;
end;

procedure TForm1.WriteFusesClick(Sender : TObject);
var
  seq : pUPDI_seq;
  fuses, wfuses : pUPDI_fuse;
  cnt, i, wcnt : integer;
begin
  if Assigned(cfg) then
  begin
    cnt := UPDILIB_devices_get_fuses_cnt(UPDILIB_cfg_get_device(cfg));
    if cnt > 0 then
    begin
      fuses := pUPDI_fuse(Getmem(sizeof(UPDI_fuse) * 2 * cnt));
      wfuses := @(fuses[cnt]);
      if Assigned(fuses) then
      begin
        for i := 0 to cnt-1 do
          fuses[i].fuse := i;
        wcnt := 0;
        for i := 0 to FusesGrid.RowCount-2 do
        begin
          if FuseState(i) = 1 then begin
            wfuses[wcnt].fuse  := strtoint(FusesGrid.Cells[0, i+1]);
            wfuses[wcnt].value := strtoint('$' + FusesGrid.Cells[2, i+1]);
            inc(wcnt);
          end;
        end;
        if wcnt > 0 then
        begin
          seq := pUPDI_seq(GetMem(Sizeof(UPDI_seq) * 3));

          seq[0].seq_type := UPDI_SEQ_ENTER_PM;
          seq[1].seq_type := UPDI_SEQ_SET_FUSES;
          seq[1].data := Pointer(wfuses);
          seq[1].data_len := wcnt;
          seq[2].seq_type := UPDI_SEQ_GET_FUSES;
          seq[2].data := Pointer(fuses);
          seq[2].data_len := cnt;

          LaunchUPDIThread(TUPDIWriteFusesThread.Create(cfg, @OnReadWriteFuses,
                                                             seq, fuses, cnt));
        end else
          Freemem(fuses);
      end;
    end;
  end;
end;

procedure TForm1.LockLog;
begin
  EnterCriticalSection(logmutex);
end;

procedure TForm1.UnLockLog;
begin
  LeaveCriticalSection(logmutex);
end;

procedure TForm1.LockProgress;
begin
  EnterCriticalSection(prgsmutex);
end;

procedure TForm1.UnLockProgress;
begin
  LeaveCriticalSection(prgsmutex);
end;

procedure TForm1.OnStopUPDIThread;
begin
  Panel1.Enabled := true;
  ProgressBar1.Visible := false;
  if wrk.Success then
    LogOut(sOKString);
end;

procedure TForm1.OnReadWriteFuses;
begin
  with TUPDIReadFusesThread(wrk) do
  if Success then
    ConsumeFuses(fuses, cnt);
  OnStopUPDIThread;
end;

procedure TForm1.OnReadDevInfoFuses;
begin
  with TUPDIReadDevInfoThread(wrk) do
  if Success then
    ConsumeDevInfo(@(devinfo[0]));
  OnStopUPDIThread;
end;

procedure TForm1.LaunchUPDIThread(awrk: TUPDIWorkingThread);
begin
  if Assigned(wrk) then
  begin
    wrk.WaitFor;
    FreeAndNil(wrk);
  end;
  if Assigned(awrk) then
  begin
    Panel1.Enabled := false;
    wrk := awrk;
    wrk.OnLog := @LogOut;
    wrk.Start;
  end;
end;

function TForm1.FuseState(pos : integer) : Byte;
var
  v, v0 : integer;
begin
  if pos >= (FusesGrid.RowCount-1) then Exit(2);

  If TryStrToInt('$' + FusesGrid.Cells[2, pos+1], v) then
  begin
    v0 := StrToInt('$'+ FusesGrid.Cells[1, pos+1]);
    if v0 = v then
      Result := 0
    else
      Result := 1
  end else
  begin
    Result := 2;
  end;
end;

procedure TForm1.ConsumeFuses(fuses : pUPDI_fuse; cnt : integer);
var
  i : integer;
begin
  FusesGrid.BeginUpdate;
  try
    FusesGrid.RowCount := cnt + 1;
    for i := 0 to cnt-1 do begin
      FusesGrid.Cells[0, i + 1] := Inttostr(fuses[i].fuse);
      FusesGrid.Cells[1, i + 1] := IntToHex(fuses[i].value, 2);
      FusesGrid.Cells[2, i + 1] := IntToHex(fuses[i].value, 2);
    end;
  finally
    FusesGrid.EndUpdate;
  end;
end;

procedure TForm1.ConsumeDevInfo(devinfo: pByte);
var
  i : Int32;
  s : String;
begin
  s := '0x';
  for i := 0 to 2 do
    s := s + IntToHex(devinfo[i], 2);
  DevIdText.Text := s;

  s := '0x';
  for i := 3 to 13 do
    s := s + IntToHex(devinfo[i], 2);
  DeviceSNText.Text := s;
end;

procedure TForm1.RefreshPortsList;
var
  ports : string;
begin
  ports := GetSerialPortNames;
  PortComboBox.Items.Text := ports;
end;

procedure TForm1.Button1Click(Sender: TObject);
begin

end;

procedure TForm1.BaudComboBoxSelect(Sender: TObject);
begin
  if Assigned(cfg) then
  begin
    UPDILIB_cfg_set_buadrate(cfg, strtoint(BaudComboBox.Text));
  end;
end;

procedure TForm1.DeviceInfoButtonClick(Sender: TObject);
begin
  if Assigned(cfg) then
  begin
    LaunchUPDIThread(TUPDIReadDevInfoThread.Create(cfg, @OnReadDevInfoFuses));
  end;
end;

procedure TForm1.DevicesComboBoxChange(Sender: TObject);
var
  dev : AnsiString;
begin
  if IsUPDILibloaded then
  begin
    if Assigned(cfg) and (DevicesComboBox.ItemIndex >= 0) then
    begin
      dev := DevicesComboBox.Items[DevicesComboBox.ItemIndex];
      UPDILIB_cfg_set_device(cfg, PChar(dev));
      FusesGrid.RowCount := 1;
    end;
  end;
end;

procedure TForm1.DevicesComboBoxSelect(Sender: TObject);
begin
  DevicesComboBoxChange(Sender);
end;

procedure TForm1.EraseDeviceButtonClick(Sender : TObject);
begin
  if IsUPDILibloaded then
  begin
    if Assigned(cfg) then
      UPDILIB_erase(cfg);
  end;
end;

procedure TForm1.FlashButtonClick(Sender : TObject);
begin
  if not Assigned(cfg) then Exit;

  if Length(InputHEXFile.FileName) = 0 then
  begin
    MessageDlg(sYouShouldSpecifyCorr, mtError, [mbOK], - 1);
    exit;
  end;

  if not FileExists(InputHEXFile.FileName) then
  begin
    MessageDlg(Format(sFileSIsNotExists, [InputHEXFile.FileName]),
                  mtError, [mbOK], -1);

  end else
  begin
    LaunchUPDIThread(TUPDIWriteFlashThread.Create(cfg, @OnStopUPDIThread,
                                                       InputHEXFile.FileName,
                                                       EraseDeviceCheckBox.Checked,
                                                       VerifyDeviceCheckBox.Checked));
  end;
end;

procedure TForm1.FusesGridDrawCell(Sender : TObject; aCol, aRow : Integer;
  aRect : TRect; aState : TGridDrawState);
begin

end;

procedure TForm1.FusesGridPrepareCanvas(Sender : TObject; aCol, aRow : Integer;
  aState : TGridDrawState);
var
  fst : Byte;
begin
  if aRow > 0 then
  begin
    if aCol = 2 then
    begin
      fst := FuseState(aRow - 1);
      case fst of
      0: FusesGrid.Canvas.Font.Color := clGreen;
      1: FusesGrid.Canvas.Font.Color := clBlue;
      2: FusesGrid.Canvas.Font.Color := clRed;
      end;
    end;
  end;
end;

procedure TForm1.InputHEXFileChange(Sender : TObject);
begin

end;

procedure TForm1.PortComboBoxChange(Sender : TObject);
var
  p : AnsiString;
begin
  if IsUPDILibloaded then
  begin
    if Assigned(cfg) and (PortComboBox.ItemIndex >= 0) then
    begin
      p := PortComboBox.Items[PortComboBox.ItemIndex];
      UPDILIB_cfg_set_com(cfg, PChar(p));
      FusesGrid.RowCount := 1;
    end;
  end;
end;

procedure TForm1.PortComboBoxSelect(Sender: TObject);
begin
  PortComboBoxChange(Sender);
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
  LockLog;
  try
    if loglist.Count > 0 then
    begin
      Log.Lines.AddStrings(loglist);
      Log.SelStart:=Length(Log.lines.Text);
      Log.VertScrollBar.Position:=1000000;
      loglist.Clear;
    end;
  finally
    UnLockLog;
  end;
  LockProgress;
  try
    ProgressBar1.Max := prgtotal;
    ProgressBar1.Position := prgpos;
    ProgressBar1.Visible:= inprogress;
  finally
    UnLockProgress;
  end;
end;

procedure TForm1.VerifyButtonClick(Sender: TObject);
begin
  if not Assigned(cfg) then Exit;

  if Length(InputHEXFile.FileName) = 0 then
  begin
    MessageDlg(sYouShouldSpecifyCorr, mtError, [mbOK], - 1);
    exit;
  end;

  if not FileExists(InputHEXFile.FileName) then
  begin
    MessageDlg(Format(sFileSIsNotExists, [InputHEXFile.FileName]),
                  mtError, [mbOK], -1);

  end else
  begin
    LaunchUPDIThread(TUPDIVerifyFlashThread.Create(cfg, @OnStopUPDIThread,
                                                       InputHEXFile.FileName));
  end;
end;

procedure TForm1.VerifyDeviceCheckBoxChange(Sender : TObject);
begin

end;

procedure TForm1.LogOut(const Str: String);
begin
  LockLog;
  try
    loglist.Add(Str);
  finally
    UnLockLog;
  end;
end;

procedure TForm1.ProgressStart(total: uint16);
begin
  LockProgress;
  try
    prgpos := 0;
    prgtotal := total;
    inprogress := true;
  finally
    UnLockProgress;
  end;
end;

procedure TForm1.ProgressStep(pos, total: uint16);
begin
  LockProgress;
  try
    prgpos := pos;
    prgtotal := total;
    inprogress := true;
  finally
    UnLockProgress;
  end;
end;

procedure TForm1.ProgressFinish(pos, total: uint16);
begin
  LockProgress;
  try
    prgpos := pos;
    prgtotal := total;
    inprogress := false;
  finally
    UnLockProgress;
  end;
end;

end.

