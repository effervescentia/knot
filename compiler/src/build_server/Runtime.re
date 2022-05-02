open Kore;

module ImportGraph = Resolve.ImportGraph;

type t = {
  server: JSONRPC.Server.t,
  compiler: Compiler.t,
  target: Target.t,
};

type request_handler_t('a) = JSONRPC.Protocol.Event.request_handler_t(t, 'a);

type notification_handler_t('a) =
  JSONRPC.Protocol.Event.notification_handler_t(t, 'a);
