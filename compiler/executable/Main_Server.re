open Kore;
open Kore.Compiler;
open Httpaf;

let run = ({paths, port}, compiler) => {
  let server = Routes.route_mapper(paths, compiler) |> Server.create(port);

  server.start();
};
