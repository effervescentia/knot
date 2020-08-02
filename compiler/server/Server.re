open Globals;
open Lwt.Infix;

type t = {
  start: unit => unit,
  close: unit => unit,
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
        Log.info(
          "%s  failed [%s %s]",
          Emoji.cross_mark,
          Httpaf.Method.to_string(req.meth),
          Uri.to_string(uri),
        );
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
  HttpServer.create_connection_handler(
    ~config=?None,
    ~request_handler=request_handler(route_mapper),
    ~error_handler,
  );

let create = (port, route_mapper) => {
  let listen_address = Unix.(ADDR_INET(inet_addr_loopback, port));
  let main_task = ref(None);
  let close_server = () =>
    switch (main_task^) {
    | Some(task) => Lwt.cancel(task)
    | None => Log.error("server is not running")
    };
  let server_lwt =
    Lwt_io.establish_server_with_client_socket(
      listen_address,
      connection_handler(route_mapper(close_server)),
    );

  {
    start: () => {
      Lwt.async(() => server_lwt >>= (_ => Lwt.return_unit));

      let (forever, _) = Lwt.task();
      main_task := Some(forever);
      Lwt_main.run(forever);
    },
    close: close_server,
  };
};
