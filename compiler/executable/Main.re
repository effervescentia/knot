open Kore;

let () = {
  Setup.run();

  let {paths} as config = Config.get();
  let desc_creator = PathResolver.simple(paths) |> Config.create_descriptor;
  let compiler = Compiler.create(desc_creator);

  compiler.inject(
    Filename.concat(
      Sys.argv[0] |> Filename.dirname |> Filename.dirname,
      "share/knot/definitions/jsx.kd",
    ),
    "@knot/jsx",
  );

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
