open Kore;
open Kore.Compiler;
open Httpaf;

let run = ({paths, port}, compiler) => {
  let server = Server.create(port, Routes.route_mapper(compiler));

  server.start();
};
