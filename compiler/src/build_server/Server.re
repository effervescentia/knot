open Kore;
open Reference;

type config_t = {
  name: string,
  root_dir: string,
  source_dir: string,
  target: Target.t,
};

let start = (config: config_t) => {
  let server = JSONRPC.Server.create(stdin, stdout);
  let compiler =
    Compiler.create(
      ~report=_ => ignore,
      {
        name: config.name,
        root_dir: config.root_dir,
        source_dir: config.source_dir,
        fail_fast: false,
        log_imports: false,
      },
    );

  let runtime = Runtime.{server, compiler, target: config.target};

  server.watch(
    Event.deserialize
    % Option.iter(
        Event.(
          fun
          | ModuleFetch(id, params) => {
              Log.debug(
                "received %s",
                ModuleFetch.method_key |> ~@Fmt.bold_str,
              );

              ModuleFetch.handler(runtime, params) |> server.reply(id);
            }

          | ModuleStatus(id, params) => {
              Log.debug(
                "received %s",
                ModuleStatus.method_key |> ~@Fmt.bold_str,
              );

              ModuleStatus.handler(runtime, params) |> server.reply(id);
            }

          | Status(id, params) => {
              Log.debug("received %s", Status.method_key |> ~@Fmt.bold_str);

              Status.handler(runtime, params) |> server.reply(id);
            }

          | Reset(id, params) => {
              Log.debug("received %s", Reset.method_key |> ~@Fmt.bold_str);

              Reset.handler(runtime, params) |> server.reply(id);
            }

          | ModuleAdd(params) => {
              Log.debug(
                "received %s",
                ModuleAdd.method_key |> ~@Fmt.bold_str,
              );

              ModuleAdd.handler(runtime, params);
            }

          | ModuleInvalidate(params) => {
              Log.debug(
                "received %s",
                ModuleInvalidate.method_key |> ~@Fmt.bold_str,
              );

              ModuleInvalidate.handler(runtime, params);
            }
        ),
      ),
  );
};
