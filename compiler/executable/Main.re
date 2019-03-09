open Kore;

let () = {
  Setup.run();

  let {paths} as config = Config.get();
  let desc_creator = PathResolver.simple(paths) |> Config.create_descriptor;
  let compiler = Compiler.create(desc_creator);

  if (config.is_server) {
    Log.info(
      "%s  running server (:%i)",
      Emoji.satellite_antenna,
      config.port,
    );

    Main_Server.run(config, compiler);
  } else {
    Log.info("%s  running stand-alone", Emoji.rocket);

    Main_StandAlone.run(config, compiler);
  };
};
