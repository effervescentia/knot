open Kore;
open Reference;

module Compiler = Compile.Compiler;
module ScopeTree = Compile.ScopeTree;
module ImportGraph = Resolve.ImportGraph;
module InputStream = File.InputStream;

let _subscribe = (channel: in_channel, handler: Event.t => unit): unit => {
  let stream = channel |> Stream.of_channel;
  let deserialize = Tuple.reduce2(Event.deserialize);

  switch (Protocol.read_event(stream) |> deserialize) {
  | Some(Initialize(req) as event) =>
    Log.info("server initialized successfully");

    handler(event);
    Protocol.watch_events(stream, event =>
      event |> deserialize |?> handler |> ignore
    );
  | _ => failwith("did not receive initialize request first")
  };
};

let start = (find_config: string => Config.t) => {
  let runtime = Runtime.{compilers: Hashtbl.create(1), find_config};

  _subscribe(
    stdin,
    Event.(
      fun
      | Initialize(req) => Initialize.handler(runtime, req)
      | Hover(req) => Hover.handler(runtime, req)
      | CodeCompletion(req) => CodeCompletion.handler(runtime, req)
      | FileOpen(req) => FileOpen.handler(runtime, req)
      | FileClose(req) => FileClose.handler(runtime, req)
      | FileChange(req) => FileChange.handler(runtime, req)
      | GoToDefinition(req) => GoToDefinition.handler(runtime, req)
      | Format(req) => Format.handler(runtime, req)
      | LocalSymbols(req) => LocalSymbols.handler(runtime, req)
      | WorkspaceSymbols(req) => WorkspaceSymbols.handler(runtime, req)
    ),
  );

  Lwt.return();
};
