open Kore;

module ImportGraph = Resolve.ImportGraph;

type compiler_context_t = {
  uri: string,
  compiler: Compiler.t,
};

type t = {
  server: JSONRPC.Server.t,
  compiler: Compiler.t,
};

type request_handler_t('a) = JSONRPC.Protocol.Event.request_handler_t(t, 'a);

type notification_handler_t('a) =
  JSONRPC.Protocol.Event.notification_handler_t(t, 'a);
