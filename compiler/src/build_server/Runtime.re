open Kore;

module ImportGraph = Resolve.ImportGraph;

module Status = {
  type t =
    | Idle
    | Running;

  let to_string =
    fun
    | Idle => "idle"
    | Running => "running";
};

type t = {
  server: JSONRPC.Server.t,
  compiler: Compiler.t,
  target: Target.t,
  log_imports: bool,
  mutable status: Status.t,
};

type request_handler_t('a) = JSONRPC.Protocol.Event.request_handler_t(t, 'a);

type notification_handler_t('a) =
  JSONRPC.Protocol.Event.notification_handler_t(t, 'a);
