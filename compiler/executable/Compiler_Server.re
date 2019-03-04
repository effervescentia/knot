open Kore;

let run = ({port}) => {
  let server = Server.create(port);

  server.start();
};
