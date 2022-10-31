open Kore;

type config_t = {
  name: string,
  root_dir: string,
  source_dir: string,
  target: Target.t,
  log_imports: bool,
  stdlib: string,
  ambient: string,
};

let start = (config: config_t) => {
  let server = JSONRPC.Server.create(stdin, stdout);
  let compiler =
    Compiler.create(
      ~report=_ => Reporter.report(server),
      {
        name: config.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
        log_imports: false,
        stdlib: config.stdlib,
        ambient: config.ambient,
      },
    );

  Compiler.prepare(compiler);

  let runtime =
    Runtime.{
      server,
      compiler,
      target: config.target,
      log_imports: config.log_imports,
      status: Idle,
    };

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

          | ModuleUpdate(params) => ModuleUpdate.handler(runtime, params)

          | ModuleRemove(params) => ModuleRemove.handler(runtime, params)
        ),
      ),
  );
};
