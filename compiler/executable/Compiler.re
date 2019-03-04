open Kore;

let () = {
  Setup.run();

  let config = Config.get();

  if (config.is_server) {
    Log.info(
      "%s  running server (:%i)",
      Emoji.satellite_antenna,
      config.port,
    );

    Compiler_Server.run(config);
  } else {
    Log.info("%s  running stand-alone", Emoji.rocket);

    Compiler_StandAlone.run(config);
  };
};
