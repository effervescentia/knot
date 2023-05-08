open Kore;

let start = (stdlib: string, find_config: string => Config.t) => {
  let server = JSONRPC.Server.create(stdin, stdout);
  let runtime =
    Runtime.{server, compilers: Hashtbl.create(1), find_config, stdlib};

  server.watch(
    Event.deserialize
    % Option.iter(
        Event.(
          fun
          | Initialize(id, params) =>
            Initialize.handler(runtime, params) |> server.reply(id)

          | Hover(id, params) =>
            Hover.handler(runtime, params) |> server.reply(id)

          | GoToDefinition(id, params) =>
            GoToDefinition.handler(runtime, params) |> server.reply(id)

          | CodeCompletion(id, params) =>
            CodeCompletion.handler(runtime, params) |> server.reply(id)

          | LocalSymbols(id, params) =>
            LocalSymbols.handler(runtime, params) |> server.reply(id)

          | WorkspaceSymbols(id, params) =>
            WorkspaceSymbols.handler(runtime, params) |> server.reply(id)

          | Format(id, params) =>
            Format.handler(runtime, params) |> server.reply(id)

          | FileOpen(req) => FileOpen.handler(runtime, req)

          | FileClose(req) => FileClose.handler(runtime, req)

          | FileChange(req) => FileChange.handler(runtime, req)
        ),
      ),
  );
};
