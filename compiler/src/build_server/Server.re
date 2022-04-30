open Kore;
open Reference;

let start = (find_config: string => Config.t) => {
  let server = JSONRPC.Server.create(stdin, stdout);
  let runtime = Runtime.{server, compilers: Hashtbl.create(1), find_config};

  server.watch(
    Event.deserialize
    % Option.iter(
        Event.(
          fun
          | Initialize(id, params) =>
            Initialize.handler(runtime, params) |> server.reply(id)
        ),
      ),
  );
};
