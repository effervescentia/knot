type t = {close: unit => unit};

let create = () => {
  let server = Httpaf.Server_connection.create(_ => ());

  {close: () => Httpaf.Server_connection.shutdown(server)};
};
