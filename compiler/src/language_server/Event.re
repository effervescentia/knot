open Kore;

type t =
  | Initialize(Deserialize.request_t(Initialize.params_t))
  | Hover(Deserialize.request_t(Hover.params_t))
  | FileOpen(Deserialize.notification_t(FileOpen.params_t))
  | FileClose(Deserialize.notification_t(FileClose.params_t))
  | FileChange(Deserialize.notification_t(FileChange.params_t))
  | GoToDefinition(Deserialize.request_t(GoToDefinition.params_t))
  | CodeCompletion(Deserialize.request_t(CodeCompletion.params_t))
  | Format(Deserialize.request_t(Format.params_t))
  | LocalSymbols(Deserialize.request_t(LocalSymbols.params_t))
  | WorkspaceSymbols(Deserialize.request_t(WorkspaceSymbols.params_t));

let deserialize =
  fun
  | "initialize" => Initialize.request % (x => Some(Initialize(x)))
  | "textDocument/hover" => Hover.request % (x => Some(Hover(x)))
  | "textDocument/didOpen" =>
    FileOpen.notification % (x => Some(FileOpen(x)))
  | "textDocument/didClose" =>
    FileClose.notification % (x => Some(FileClose(x)))
  | "textDocument/didChange" =>
    FileChange.notification % (x => Some(FileChange(x)))
  | "textDocument/definition" =>
    GoToDefinition.request % (x => Some(GoToDefinition(x)))
  | "textDocument/completion" =>
    CodeCompletion.request % (x => Some(CodeCompletion(x)))
  | "textDocument/documentSymbol" =>
    LocalSymbols.request % (x => Some(LocalSymbols(x)))
  | "workspace/symbol" =>
    WorkspaceSymbols.request % (x => Some(WorkspaceSymbols(x)))
  | "textDocument/formatting" => Format.request % (x => Some(Format(x)))
  | _ => (_ => None);
