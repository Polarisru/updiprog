(******************************************************************************)
(*                                  libUPDI                                   *)
(*                  free pascal wrapper around UPDI library                   *)
(*                                                                            *)
(* Copyright (c) 2023 Ilya Medvedkov                                          *)
(******************************************************************************)
(* Трансляция на основе                                                       *)
(* \file      updilib.h                                                       *)
(* \brief     Модуль интерфейса программирования по интерфейсу UPDI           *)
(* \author    Медведков Илья                                                  *)
(* \version   0.01                                                            *)
(* \date      2023                                                            *)
(******************************************************************************)


unit libUPDI;

{$mode objfpc}{$H+}

{$packrecords c}

interface

uses dynlibs, SysUtils;

const
{$if defined(UNIX) and not defined(darwin)}
  CUPDILDLL = 'libupdilib.so';
{$ELSE}
{$ifdef WINDOWS}
  CUPDILDLL = 'updilib.dll';
{$endif}
{$endif}

  COMPORT_LEN            = 32;
  DEVICES_NAME_LEN       = 16;
  LOG_SRCNAME_LEN        = 8;

  LOG_LEVEL_VERBOSE      = 0;
  LOG_LEVEL_INFO         = 1;
  LOG_LEVEL_WARNING      = 2;
  LOG_LEVEL_ERROR        = 3;
  LOG_LEVEL_LAST         = 4;

  UPDI_SEQ_UNLOCK        = 1;
  UPDI_SEQ_ENTER_PM      = 2;
  UPDI_SEQ_ERASE         = 3;
  UPDI_SEQ_FLASH         = 4;
  UPDI_SEQ_READ          = 5;
  UPDI_SEQ_SET_FUSES     = 6;
  UPDI_SEQ_GET_FUSES     = 7;
  UPDI_SEQ_LOCK          = 8;

type
  UPDI_bool = Byte;

  UPDI_onlog = procedure (ud: Pointer; level: int32; const src, msg: pansichar) cdecl;
  UPDI_onlogfree = procedure (ud: Pointer) cdecl;

  pUPDI_logger = Pointer;

  pUPDI_Params = Pointer;

  UPDI_fuse = record
    fuse : byte;
    value : byte;
  end;

  pUPDI_fuse = ^UPDI_fuse;

  UPDI_seq = record
    seq_type : Byte;
    data : Pointer;
    data_len : Int32;
  end;

  pUPDI_seq = ^UPDI_seq;


function UPDILIB_logger_init(const src : pansichar;
                             loglevel : int32;
                             onlog: UPDI_onlog;
                             onfree: UPDI_onlogfree;
                             ud : Pointer) : pUPDI_logger;
procedure UPDILIB_logger_done(logger : pUPDI_logger);

procedure UPDILIB_set_glb_logger_onlog(onlog: UPDI_onlog; ud : Pointer);
procedure UPDILIB_set_glb_logger_level(level : int32);

function UPDILIB_cfg_init() : pUPDI_Params;
procedure UPDILIB_cfg_done(cfg: pUPDI_Params);

function UPDILIB_cfg_set_logger(cfg : pUPDI_Params;
                             logger : pUPDI_logger) : UPDI_bool;
function UPDILIB_cfg_set_buadrate(cfg : pUPDI_Params;
                             value : uint32) : UPDI_bool;
function UPDILIB_cfg_set_com(cfg : pUPDI_Params;
                             const port : pansichar) : UPDI_bool;
function UPDILIB_cfg_set_device(cfg : pUPDI_Params;
                             const name : pansichar) : UPDI_bool;
function UPDILIB_cfg_get_baudrate(cfg : pUPDI_Params) : uint32;
function UPDILIB_cfg_get_com(cfg : pUPDI_Params) : PAnsiChar;
function UPDILIB_cfg_get_device(cfg : pUPDI_Params) : int8;

function UPDILIB_devices_get_count() : int32;
function UPDILIB_devices_get_name(id : int8;
                             name : pansichar; cnt : pint32) : UPDI_bool;
function UPDILIB_devices_get_fuses_cnt(id : int8) : Byte;

function UPDILIB_launch_seq(cfg : pUPDI_Params; seq : pUPDI_seq;
                             len : Byte) : UPDI_bool;

function UPDILIB_erase(cfg : pUPDI_Params) : UPDI_bool;

function UPDILIB_write_fuses(cfg : pUPDI_Params;
                             const fuses: pUPDI_fuse; cnt : byte) : UPDI_bool;
function UPDILIB_read_fuses(cfg : pUPDI_Params;
                             fuses : pUPDI_fuse; cnt : byte) : UPDI_bool;

function UPDILIB_write_hex(cfg : pUPDI_Params;
                             const data : pansichar; cnt : int32) : UPDI_bool;
function UPDILIB_read_hex(cfg : pUPDI_Params;
                             data: pansichar; size : pint32) : UPDI_bool;

function IsUPDILibloaded: boolean;
function InitUPDILibInterface(const aPath : String): boolean; overload;
function DestroyUPDILibInterface: boolean;

resourcestring
  SFailedToLoadUPDILib = 'Failed to load UPDI library';

implementation

var
  UPDILloaded : Boolean = false;
  LUPDILib: HModule = NilHandle;

type
   p_UPDILIB_logger_init = function (const src : pansichar;
                             loglevel : int32;
                             onlog: UPDI_onlog;
                             onfree: UPDI_onlogfree;
                             ud : Pointer) : pUPDI_logger cdecl;
   p_UPDILIB_logger_done = procedure (logger : pUPDI_logger) cdecl;
   p_UPDILIB_set_glb_logger_onlog = procedure(onlog: UPDI_onlog;
                             ud : Pointer) cdecl;
   p_UPDILIB_set_glb_logger_level = procedure (level : int32) cdecl;

   p_UPDILIB_cfg_init = function () : pUPDI_Params cdecl;
   p_UPDILIB_cfg_done = procedure (cfg: pUPDI_Params) cdecl;

   p_UPDILIB_cfg_set_logger = function (cfg : pUPDI_Params;
                             logger : pUPDI_logger) : UPDI_bool cdecl;
   p_UPDILIB_cfg_set_buadrate = function (cfg : pUPDI_Params;
                             value : uint32) : UPDI_bool cdecl;
   p_UPDILIB_cfg_set_com = function (cfg : pUPDI_Params;
                             const port : pansichar) : UPDI_bool cdecl;
   p_UPDILIB_cfg_set_device = function (cfg : pUPDI_Params;
                             const name : pansichar) : UPDI_bool cdecl;

   p_UPDILIB_cfg_get_baudrate = function (cfg : pUPDI_Params) : uint32 cdecl;
   p_UPDILIB_cfg_get_com = function (cfg : pUPDI_Params) : pAnsichar cdecl;
   p_UPDILIB_cfg_get_device = function (cfg : pUPDI_Params) : int8 cdecl;

   p_UPDILIB_devices_get_count = function () : int32 cdecl;
   p_UPDILIB_devices_get_name = function (id : int8;
                             name : pansichar; cnt : pint32) : UPDI_bool cdecl;
   p_UPDILIB_devices_get_fuses_cnt = function (id : int8) : byte cdecl;

   p_UPDILIB_launch_seq = function (cfg : pUPDI_Params; seq : pUPDI_seq;
                             len : Byte) : UPDI_bool cdecl;

   p_UPDILIB_erase = function (cfg : pUPDI_Params) : UPDI_bool cdecl;

   p_UPDILIB_write_fuses = function (cfg : pUPDI_Params;
                             const fuses: pUPDI_fuse;
                             cnt : byte) : UPDI_bool cdecl;
   p_UPDILIB_read_fuses = function (cfg : pUPDI_Params;
                             fuses : pUPDI_fuse; cnt : byte) : UPDI_bool cdecl;

   p_UPDILIB_write_hex = function (cfg : pUPDI_Params;
                             const data : pansichar;
                             cnt : int32) : UPDI_bool cdecl;
   p_UPDILIB_read_hex = function (cfg : pUPDI_Params;
                             data: pansichar; size : pint32) : UPDI_bool cdecl;

var
  _UPDILIB_logger_init       : p_UPDILIB_logger_init       = nil;
  _UPDILIB_logger_done       : p_UPDILIB_logger_done       = nil;
  _UPDILIB_set_glb_logger_onlog : p_UPDILIB_set_glb_logger_onlog = nil;
  _UPDILIB_set_glb_logger_level : p_UPDILIB_set_glb_logger_level = nil;
  _UPDILIB_cfg_init          : p_UPDILIB_cfg_init          = nil;
  _UPDILIB_cfg_done          : p_UPDILIB_cfg_done          = nil;
  _UPDILIB_cfg_set_logger    : p_UPDILIB_cfg_set_logger    = nil;
  _UPDILIB_cfg_set_buadrate  : p_UPDILIB_cfg_set_buadrate  = nil;
  _UPDILIB_cfg_set_com       : p_UPDILIB_cfg_set_com       = nil;
  _UPDILIB_cfg_set_device    : p_UPDILIB_cfg_set_device    = nil;
  _UPDILIB_cfg_get_baudrate  : p_UPDILIB_cfg_get_baudrate  = nil;
  _UPDILIB_cfg_get_com       : p_UPDILIB_cfg_get_com       = nil;
  _UPDILIB_cfg_get_device    : p_UPDILIB_cfg_get_device    = nil;
  _UPDILIB_devices_get_count : p_UPDILIB_devices_get_count = nil;
  _UPDILIB_devices_get_name  : p_UPDILIB_devices_get_name  = nil;
  _UPDILIB_devices_get_fuses_cnt : p_UPDILIB_devices_get_fuses_cnt = nil;
  _UPDILIB_launch_seq        : p_UPDILIB_launch_seq        = nil;
  _UPDILIB_erase             : p_UPDILIB_erase             = nil;
  _UPDILIB_write_fuses       : p_UPDILIB_write_fuses       = nil;
  _UPDILIB_read_fuses        : p_UPDILIB_read_fuses        = nil;
  _UPDILIB_write_hex         : p_UPDILIB_write_hex         = nil;
  _UPDILIB_read_hex          : p_UPDILIB_read_hex          = nil;

function UPDILIB_logger_init(const src: pansichar; loglevel: int32;
  onlog: UPDI_onlog; onfree: UPDI_onlogfree; ud: Pointer): pUPDI_logger;
begin
  if Assigned(_UPDILIB_logger_init) then
    Result := _UPDILIB_logger_init(src, loglevel, onlog, onfree, ud)
  else
    Result := nil;
end;

procedure UPDILIB_logger_done(logger: pUPDI_logger);
begin
  if Assigned(_UPDILIB_logger_done) then
    _UPDILIB_logger_done(logger);
end;

procedure UPDILIB_set_glb_logger_onlog(onlog: UPDI_onlog; ud: Pointer);
begin
  if Assigned(_UPDILIB_set_glb_logger_onlog) then
    _UPDILIB_set_glb_logger_onlog(onlog, ud);
end;

procedure UPDILIB_set_glb_logger_level(level : int32);
begin
  if Assigned(_UPDILIB_set_glb_logger_level) then
    _UPDILIB_set_glb_logger_level(level);
end;

function UPDILIB_cfg_init: pUPDI_Params;
begin
  if Assigned(_UPDILIB_cfg_init) then
    Result := _UPDILIB_cfg_init()
  else
    Result := nil;
end;

procedure UPDILIB_cfg_done(cfg: pUPDI_Params);
begin
  if Assigned(_UPDILIB_cfg_done) then
    _UPDILIB_cfg_done(cfg);
end;

function UPDILIB_cfg_set_logger(cfg: pUPDI_Params; logger: pUPDI_logger
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_cfg_set_logger) then
    Result := _UPDILIB_cfg_set_logger(cfg, logger)
  else
    Result := 0;
end;

function UPDILIB_cfg_set_buadrate(cfg: pUPDI_Params; value: uint32): UPDI_bool;
begin
  if Assigned(_UPDILIB_cfg_set_buadrate) then
    Result := _UPDILIB_cfg_set_buadrate(cfg, value)
  else
    Result := 0;
end;

function UPDILIB_cfg_set_com(cfg: pUPDI_Params; const port: pansichar
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_cfg_set_com) then
    Result := _UPDILIB_cfg_set_com(cfg, port)
  else
    Result := 0;
end;

function UPDILIB_cfg_set_device(cfg: pUPDI_Params; const name: pansichar
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_cfg_set_device) then
    Result := _UPDILIB_cfg_set_device(cfg, name)
  else
    Result := 0;
end;

function UPDILIB_cfg_get_baudrate(cfg : pUPDI_Params) : uint32;
begin
  if Assigned(_UPDILIB_cfg_get_baudrate) then
    Result := _UPDILIB_cfg_get_baudrate(cfg)
  else
    Result := 0;
end;

function UPDILIB_cfg_get_com(cfg : pUPDI_Params) : PAnsiChar;
begin
  if Assigned(_UPDILIB_cfg_get_com) then
    Result := _UPDILIB_cfg_get_com(cfg)
  else
    Result := nil;
end;

function UPDILIB_cfg_get_device(cfg : pUPDI_Params) : int8;
begin
  if Assigned(_UPDILIB_cfg_get_device) then
    Result := _UPDILIB_cfg_get_device(cfg)
  else
    Result := 0;
end;

function UPDILIB_devices_get_count: int32;
begin
  if Assigned(_UPDILIB_devices_get_count) then
    Result := _UPDILIB_devices_get_count()
  else
    Result := 0;
end;

function UPDILIB_devices_get_name(id: int8; name: pansichar; cnt: pint32
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_devices_get_name) then
    Result := _UPDILIB_devices_get_name(id, name, cnt)
  else
    Result := 0;
end;

function UPDILIB_devices_get_fuses_cnt(id : int8) : Byte;
begin
  if Assigned(_UPDILIB_devices_get_fuses_cnt) then
    Result := _UPDILIB_devices_get_fuses_cnt(id)
  else
    Result := 0;
end;

function UPDILIB_launch_seq(cfg : pUPDI_Params; seq : pUPDI_seq; len : Byte
  ) : UPDI_bool;
begin
  if Assigned(_UPDILIB_launch_seq) then
    Result := _UPDILIB_launch_seq(cfg, seq, len)
  else
    Result := 0;
end;

function UPDILIB_erase(cfg: pUPDI_Params): UPDI_bool;
begin
  if Assigned(_UPDILIB_erase) then
    Result := _UPDILIB_erase(cfg)
  else
    Result := 0;
end;

function UPDILIB_write_fuses(cfg: pUPDI_Params; const fuses: pUPDI_fuse;
  cnt: byte): UPDI_bool;
begin
  if Assigned(_UPDILIB_write_fuses) then
    Result := _UPDILIB_write_fuses(cfg, fuses, cnt)
  else
    Result := 0;
end;

function UPDILIB_read_fuses(cfg: pUPDI_Params; fuses: pUPDI_fuse; cnt: byte
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_read_fuses) then
    Result := _UPDILIB_read_fuses(cfg, fuses, cnt)
  else
    Result := 0;
end;

function UPDILIB_write_hex(cfg: pUPDI_Params; const data: pansichar; cnt: int32
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_write_hex) then
    Result := _UPDILIB_write_hex(cfg, data, cnt)
  else
    Result := 0;
end;

function UPDILIB_read_hex(cfg: pUPDI_Params; data: pansichar; size: pint32
  ): UPDI_bool;
begin
  if Assigned(_UPDILIB_read_hex) then
    Result := _UPDILIB_read_hex(cfg, data, size)
  else
    Result := 0;
end;

function IsUPDILibloaded: boolean;
begin
  Result := UPDILloaded;
end;

procedure UnloadLibraries;
begin
  UPDILloaded := False;
  if LUPDILib <> NilHandle then
  begin
    FreeLibrary(LUPDILib);
    LUPDILib := NilHandle;
  end;
end;

function LoadLibraries(const aPath : String): boolean;
begin
  Result := False;

  if Length(aPath) > 0 then
    LUPDILib := LoadLibrary(aPath)
  else
    LUPDILib := LoadLibrary(CUPDILDLL);

  UPDILloaded := (LUPDILib <> NilHandle);

  Result := UPDILloaded;
end;

function GetProcAddr(module: HModule; const ProcName: string): Pointer;
begin
  Result := GetProcAddress(module, PChar(ProcName));
end;

procedure LoadUPDILibEntryPoints;
begin
  _UPDILIB_logger_init := p_UPDILIB_logger_init(GetProcAddr(LUPDILib, 'UPDILIB_logger_init'));
  _UPDILIB_logger_done := p_UPDILIB_logger_done(GetProcAddr(LUPDILib, 'UPDILIB_logger_done'));
  _UPDILIB_set_glb_logger_onlog := p_UPDILIB_set_glb_logger_onlog(GetProcAddr(LUPDILib, 'UPDILIB_set_glb_logger_onlog'));
  _UPDILIB_set_glb_logger_level := p_UPDILIB_set_glb_logger_level(GetProcAddr(LUPDILib, 'UPDILIB_set_glb_logger_level'));
  _UPDILIB_cfg_init := p_UPDILIB_cfg_init(GetProcAddr(LUPDILib, 'UPDILIB_cfg_init'));
  _UPDILIB_cfg_done := p_UPDILIB_cfg_done(GetProcAddr(LUPDILib, 'UPDILIB_cfg_done'));
  _UPDILIB_cfg_set_logger := p_UPDILIB_cfg_set_logger(GetProcAddr(LUPDILib, 'UPDILIB_cfg_set_logger'));
  _UPDILIB_cfg_set_buadrate := p_UPDILIB_cfg_set_buadrate(GetProcAddr(LUPDILib, 'UPDILIB_cfg_set_buadrate'));
  _UPDILIB_cfg_set_com := p_UPDILIB_cfg_set_com(GetProcAddr(LUPDILib, 'UPDILIB_cfg_set_com'));
  _UPDILIB_cfg_set_device := p_UPDILIB_cfg_set_device(GetProcAddr(LUPDILib, 'UPDILIB_cfg_set_device'));
  _UPDILIB_devices_get_count := p_UPDILIB_devices_get_count(GetProcAddr(LUPDILib, 'UPDILIB_devices_get_count'));
  _UPDILIB_devices_get_name := p_UPDILIB_devices_get_name(GetProcAddr(LUPDILib, 'UPDILIB_devices_get_name'));
  _UPDILIB_devices_get_fuses_cnt := p_UPDILIB_devices_get_fuses_cnt(GetProcAddr(LUPDILib, 'UPDILIB_devices_get_fuses_cnt'));
  _UPDILIB_launch_seq        := p_UPDILIB_launch_seq(GetProcAddr(LUPDILib, 'UPDILIB_launch_seq'));
  _UPDILIB_erase := p_UPDILIB_erase(GetProcAddr(LUPDILib, 'UPDILIB_erase'));
  _UPDILIB_write_fuses := p_UPDILIB_write_fuses(GetProcAddr(LUPDILib, 'UPDILIB_write_fuses'));
  _UPDILIB_read_fuses := p_UPDILIB_read_fuses(GetProcAddr(LUPDILib, 'UPDILIB_read_fuses'));
  _UPDILIB_write_hex := p_UPDILIB_write_hex(GetProcAddr(LUPDILib, 'UPDILIB_write_hex'));
  _UPDILIB_read_hex := p_UPDILIB_read_hex(GetProcAddr(LUPDILib, 'UPDILIB_read_hex'));
end;

procedure ClearUPDILibEntryPoints;
begin
  _UPDILIB_logger_init := nil;
  _UPDILIB_logger_done := nil;
  _UPDILIB_set_glb_logger_onlog := nil;
  _UPDILIB_set_glb_logger_level := nil;
  _UPDILIB_cfg_init := nil;
  _UPDILIB_cfg_done := nil;
  _UPDILIB_cfg_set_logger := nil;
  _UPDILIB_cfg_set_buadrate := nil;
  _UPDILIB_cfg_set_com := nil;
  _UPDILIB_cfg_set_device := nil;
  _UPDILIB_devices_get_count := nil;
  _UPDILIB_devices_get_name := nil;
  _UPDILIB_devices_get_fuses_cnt := nil;
  _UPDILIB_launch_seq := nil;
  _UPDILIB_erase := nil;
  _UPDILIB_write_fuses := nil;
  _UPDILIB_read_fuses := nil;
  _UPDILIB_write_hex := nil;
  _UPDILIB_read_hex := nil;
end;

function InitUPDILibInterface(const aPath: String): boolean;
begin
  Result := IsUPDILibloaded;
  if Result then
    exit;
  Result := LoadLibraries(aPath);
  if not Result then
  begin
    UnloadLibraries;
    Exit;
  end;
  LoadUPDILibEntryPoints;
  Result := UPDILloaded;
end;

function DestroyUPDILibInterface: boolean;
begin
  Result := not IsUPDILibloaded;
  if Result then
    exit;
  ClearUPDILibEntryPoints;
  UnloadLibraries;
  Result := True;
end;

end.

