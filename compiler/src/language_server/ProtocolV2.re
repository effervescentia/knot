open Kore;

module Error = {
  type t =
    | ServerNotInitialized
    | UnknownErrorCode
    | RequestFailed
    | ServerCancelled
    | ContentModified
    | RequestCancelled;

  let to_int =
    fun
    | ServerNotInitialized => (-32002)
    | UnknownErrorCode => (-32001)
    | RequestFailed => (-32803)
    | ServerCancelled => (-32802)
    | ContentModified => (-32801)
    | RequestCancelled => (-32800);
};

module Request = {
  type t =
    | Initialize(Initialize.params_t)
    | Hover(Hover.params_t)
    | GoToDefinition(GoToDefinition.params_t)
    | CodeCompletion(CodeCompletion.params_t)
    | Format(Format.params_t)
    | LocalSymbols(LocalSymbols.params_t)
    | WorkspaceSymbols(WorkspaceSymbols.params_t);

  let of_initialize = x => Initialize(x);
};

module Notification = {
  type t =
    | FileOpen(FileOpen.params_t)
    | FileClose(FileClose.params_t)
    | FileChange(FileChange.params_t);
};

type request_t =
  | Request(int, Request.t)
  | Notification(Notification.t);

let _to_req = (id, req) => Request(id, req);
let _to_notif = notif => Notification(notif);

let deserialize_request = (req: JSONRPC.Protocol.Event.t): request_t =>
  switch (req) {
  | Request(id, "initialize", params) =>
    params |> Initialize.deserialize |> Request.of_initialize |> _to_req(id)
  };
