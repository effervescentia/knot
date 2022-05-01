open Kore;
open Reference;

let start = (config: Config.t) => {
  let server = JSONRPC.Server.create(stdin, stdout);
  let compiler =
    Compiler.create(
      ~report=_ => ignore,
      {
        name: config.name |?: Filename.basename(config.root_dir),
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: config.fail_fast,
        log_imports: config.log_imports,
      },
    );

  let runtime = Runtime.{server, compiler};

  server.watch(
    Event.deserialize
    % Option.iter(
        Event.(
          fun
          | ModuleFetch(id, params) =>
            ModuleFetch.handler(runtime, params) |> server.reply(id)

          | ModuleStatus(id, params) =>
            ModuleStatus.handler(runtime, params) |> server.reply(id)

          | Status(id, params) =>
            Status.handler(runtime, params) |> server.reply(id)

          | Reset(id, params) =>
            Reset.handler(runtime, params) |> server.reply(id)

          | ModuleAdd(params) => ModuleAdd.handler(runtime, params)

          | ModuleInvalidate(params) =>
            ModuleInvalidate.handler(runtime, params)
        ),
      ),
  );
};
