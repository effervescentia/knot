open Kore;

module ImportGraph = Resolve.ImportGraph;

type compiler_context_t = {
  uri: string,
  compiler: Compiler.t,
};

type t = {
  server: JSONRPC.Server.t,
  find_config: string => Config.t,
  compilers: Hashtbl.t(string, compiler_context_t),
};

type request_handler_t('a) = JSONRPC.Protocol.Event.request_handler_t(t, 'a);
