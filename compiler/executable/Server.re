open Kore;
open Lwt.Infix;
open Httpaf;

module Response = Server_Response;

type t = {
  start: unit => unit,
  close: unit => Lwt.t(unit),
};

let request_handler = (addr, req_d) => {
  let req = Reqd.request(req_d);

  switch (req.meth) {
  | `POST =>
    let host = Headers.get_exn(req.headers, "host");
    let url = Printf.sprintf("http://%s%s", host, req.target);
    /* let uri = Uri.of_string(url); */

    Log.info("%s  [POST] %s", Emoji.inbox_tray, url);

    let req_body = Reqd.request_body(req_d);
    let res_body = Response.ok() |> Reqd.respond_with_streaming(req_d);

    let rec respond = () =>
      Body.schedule_read(
        req_body,
        ~on_eof=() => Body.close_writer(res_body),
        ~on_read=
          (data, ~off, ~len) => {
            Body.write_bigstring(res_body, data, ~off, ~len);

            /* continue to write until body is empty */
            respond();
          },
      );

    respond();
  | _ => Reqd.respond_with_string(req_d, Response.not_allowed(), "")
  };
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
