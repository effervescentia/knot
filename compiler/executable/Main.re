open Globals;

module Server = KnotServe.Main;
module Transpiler = KnotTranspile.Transpiler;

let inject_definition =
  Compiler.(
    (compiler, file) =>
      compiler.inject(
        Filename.concat(
          Sys.argv[0] |> Filename.dirname |> Filename.dirname,
          "share/knot/definitions/" ++ file,
        ),
      )
  );

let run = () => {
  let {paths} as config = Setup.run();

  let desc_creator =
    PathResolver.simple(paths) |> Config.create_descriptor(config);
  let compiler = Compiler.create(desc_creator);

  inject_definition(compiler, "jsx.kd", "@knot/jsx");
  inject_definition(compiler, "style.kd", "@knot/style");

  if (config.is_server) {
    Log.info(
      "%s  running server (:%i)",
      Emoji.satellite_antenna,
      config.port,
    );

    try(Server.run(config, compiler)) {
    | Lwt.Canceled => Log.info("%s  shutting down", Emoji.robot_face)
    };
  } else {
    Log.info("%s  running stand-alone", Emoji.rocket);

    Transpiler.run(config, compiler);
  };
};

let () =
  try(run()) {
  | err =>
    switch (err) {
    /* already handled, only to force a non-zero exit */
    | InternalCompilationError => ()
    | ExecutionError(e) =>
      Log.error("%s  knot encountered an error", Emoji.red_paper_lantern);
      Error.print_execution_error(e);
    | Invariant(inv) =>
      Log.error(
        "%s  invariant violated, this should never happen",
        Emoji.upside_down_face,
      );
      print_invariant(inv);
    | _ =>
      Log.error(
        "%s  knot encountered an unexpected error '%s'",
        Emoji.thinking_face,
        Printexc.to_string(err),
      );
      Printexc.print_backtrace(stderr);
    };

    exit(-1);
  };
