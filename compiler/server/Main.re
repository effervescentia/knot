open Globals;
open KnotCompile.Compiler;

let run = ({paths, port} as config, compiler) => {
  let server = Routes.route_mapper(config, compiler) |> Server.create(port);

  server.start();
};
