open Kore;
open Kore.Compiler;
open Httpaf;

let run = ({paths, port} as config, compiler) => {
  let server = Routes.route_mapper(config, compiler) |> Server.create(port);

  server.start();
};
