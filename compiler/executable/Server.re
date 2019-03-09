open Kore;
open Lwt.Infix;
open Httpaf;

module Response = Server_Response;

type t = {
  start: unit => unit,
  close: unit => Lwt.t(unit),
};

let request_handler = (route_mapper, addr, req_d) => {
  let req = Reqd.request(req_d);
  let host = Headers.get_exn(req.headers, "host");
  let url = Printf.sprintf("http://%s%s", host, req.target);
  let uri = Uri.of_string(url);
  let path = Uri.path(uri);

  route_mapper((req.meth, String.sub(path, 1, String.length(path) - 1)))
  |> (
    fun
    | Some(f) => f(req_d, uri)
    | None => {
        Log.info("%s  failed (%s)", Emoji.cross_mark, Uri.to_string(uri));
        Reqd.respond_with_string(req_d, Response.not_allowed(), "");
      }
  );
};

let error_handler = (_client_address, ~request=?, error, start_response) => {
  let response_body = start_response(Headers.empty);

  switch (error) {
  | `Exn(exn) =>
    Printexc.to_string(exn)
    |> Printf.sprintf("%s\n")
    |> Body.write_string(response_body)
  | #Status.standard as error =>
    Status.default_reason_phrase(error) |> Body.write_string(response_body)
  };

  Body.close_writer(response_body);
};

let connection_handler = route_mapper =>
  Http.Server.create_connection_handler(
    ~config=?None,
    ~request_handler=request_handler(route_mapper),
    ~error_handler,
  );

let create = (port, route_mapper) => {
  let listen_address = Unix.(ADDR_INET(inet_addr_loopback, port));
  let server =
    Lwt_io.establish_server_with_client_socket(
      listen_address,
      connection_handler(route_mapper),
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
