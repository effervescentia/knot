open Kore;

let () = {
  Setup.run();

  let {paths} as config = Config.get();
  let desc_creator = PathResolver.simple(paths) |> Config.create_descriptor;
  let compiler = Compiler.create(desc_creator);
  let inject_definition = file =>
    compiler.inject(
      Filename.concat(
        Sys.argv[0] |> Filename.dirname |> Filename.dirname,
        "share/knot/definitions/" ++ file,
      ),
    );

  inject_definition("jsx.kd", "@knot/jsx");
  inject_definition("style.kd", "@knot/style");

  if (config.is_server) {
    Log.info(
      "%s  running server (:%i)",
      Emoji.satellite_antenna,
      config.port,
    );

    try (Main_Server.run(config, compiler)) {
    | Lwt.Canceled => Log.info("%s  shutting down", Emoji.robot_face)
    };
  } else {
    Log.info("%s  running stand-alone", Emoji.rocket);

    Main_StandAlone.run(config, compiler);
  };
};
