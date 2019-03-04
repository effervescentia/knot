open Kore;

let () = {
  Setup.run();

  let config = Config.get();

  if (config.is_server) {
    Compiler_Server.run(config);
  } else {
    Compiler_StandAlone.run(config);
  };
};
