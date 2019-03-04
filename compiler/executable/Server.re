open Lwt.Infix;

type t = {
  start: unit => unit,
  close: unit => Lwt.t(unit),
};

let request_handler = (_, _) => ();

let error_handler =
  Httpaf.(
    (_client_address, ~request=?, error, start_response) => {
      let response_body = start_response(Headers.empty);

      switch (error) {
      | `Exn(exn) =>
        Body.write_string(response_body, Printexc.to_string(exn));
        Body.write_string(response_body, "\n");
      | #Status.standard as error =>
        Body.write_string(response_body, Status.default_reason_phrase(error))
      };

      Body.close_writer(response_body);
    }
  );

let connection_handler =
  Http.Server.create_connection_handler(
    ~config=?None,
    ~request_handler,
    ~error_handler,
  );

let create = port => {
  let listen_address = Unix.(ADDR_INET(inet_addr_loopback, port));
  let server =
    Lwt_io.establish_server_with_client_socket(
      listen_address,
      connection_handler,
    );

  {
    start: () => {
      Lwt.async(() => server >>= (_ => Lwt.return_unit));

      let (forever, _) = Lwt.wait();
      Lwt_main.run(forever);
    },
    close: () => Lwt.bind(server, Lwt_io.shutdown_server),
  };
};
